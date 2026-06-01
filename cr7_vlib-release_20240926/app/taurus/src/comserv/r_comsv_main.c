/*************************************************************************************************************
* taurus_comsv_main_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/* Manage communication between TAURUS and the client.
   Code has o be agnostic of the concrete HW, communication method and the OS.
   For OS functionality, the OSAL has to be used.
   COMAL is the HW and communicaton method abstraction.

 */



/*******************************************************************************
   Section: Includes
 */

#include "r_taurus_bridge.h"

#include "rcar-xos/osal/r_osal.h"
#include "r_rte_comal_api.h"
#include "r_taurus_main.h"
#include "r_perserv_api.h"
#include "r_taurus_platform.h"
#include "r_taurus_platform_peri_ids.h"

#include "r_comsv_main.h"

#include "r_print_api.h"

/*******************************************************************************
   Section: Local Defines
 */

#define LOC_MAX_GUEST_NUM         1

/* Create thread ID */
#define LOC_COMSRV_RESOURCE_ID    0x8000U                        /* RESOURCE_ID_BSP               */
#define LOC_COMSRV_THREAD_CREATE  (LOC_COMSRV_RESOURCE_ID + 10U) /* OSAL_THREAD_TAURUS_COM_SERVER */

/* Prio is lower than typical CIO drivers of (0x81),
   but higher than normal application (0x6a)
 */

#define LOC_COMSERVERAPP_PRIO     0x74

#define LOC_SERVERAPP_STACKSIZE   0x2000


/*******************************************************************************
   Section: Local Types
 */

typedef struct {
    int Guest;
    osal_thread_handle_t            comsrv_thread_handle;
    R_TAURUS_PeripheralServer_t   * *Peripheral;
    char Quit;
} R_COMSV_Data_t;


/*******************************************************************************
   Section: Local Variables
 */

static R_COMSV_Data_t locData[LOC_MAX_GUEST_NUM];


/*******************************************************************************
   Section: Local Functions
 */

static R_COMSV_Data_t *locFindComData(int Guest)
{
    int i;
    R_COMSV_Data_t *data;

    data = 0;
    for (i = 0; i < LOC_MAX_GUEST_NUM; i++ ) {
        if (locData[i].Guest == Guest) {
            data = &locData[i];
            break;
        }
    }
    return data;
}



static R_COMSV_Data_t *locAllocComData(int Guest)
{
    R_COMSV_Data_t *data;

    data = locFindComData(0);
    if (data) {
        data->Guest = Guest;
    } else {
        R_PRINT_Log("[ComsvMain]: locAllocComData data NULL\r\n");
    }
    return data;
}



static int locFreeComData(int Guest)
{
    R_COMSV_Data_t *data;

    data = locFindComData(Guest);
    if (data) {
        data->Guest = 0;
        return 0;
    } else {
        R_PRINT_Log("[ComsvMain]: locFreeComData data NULL\r\n");
    }
    return -1;
}


/*******************************************************************************
   Function: locComServerApp

   The communication server main loop.

   This thread waits for commands sent by a guest and distributes them
   to the peripheral servers. For each guest, there is one thread executing
   this function.

   If the parallel execution of commands for  multiple peripheral drivers is
   desired, you have to run multiple instances of the server. In such case,
   the COMAL has to deal with concurrent access to the TAURUS communication
   interface  based on e.g. guest id.

   Parameters:
   Arg        - Guest identifier.

   Returns:
   void*
 */

static int64_t locComServerApp(void * Arg)
{
    R_COMSV_Data_t        *server;
    R_TAURUS_CmdMsg_t cmd;
    R_TAURUS_ResultMsg_t result;
    int guest;
    int x;

    server = (R_COMSV_Data_t *)Arg;
    guest = server->Guest;

    x = R_RTE_COMAL_Init(guest);
    if (x != 0) {
        R_PRINT_Log("[ComsvMain]: locComServerApp R_RTE_COMAL_Init guest(%d) error(%d)\r\n", guest, x);
        TAURUS_Error(-1);
        goto exit;
    }

    do {
        /* Get command from guest */
        x = R_RTE_COMAL_ReadCmd(guest, &cmd);
        if (x != 0) {
            R_PRINT_Log("[ComsvMain]: locComServerApp R_RTE_COMAL_ReadCmd guest(%d) error(%d)\r\n", guest, x);
            TAURUS_Error(-1);
            break;
        }
        result.Id = cmd.Id;
        result.Channel = cmd.Channel;

        /* ACK or NACK the command */
        switch (cmd.Cmd) {
        case R_TAURUS_CMD_NOP:
        case R_TAURUS_CMD_OPEN:
        case R_TAURUS_CMD_CLOSE:
        case R_TAURUS_CMD_READ:
        case R_TAURUS_CMD_WRITE:
        case R_TAURUS_CMD_IOCTL:
        case R_TAURUS_CMD_STATUS:
            result.Result = R_TAURUS_RES_ACK;
            break;
        default:
            result.Result = R_TAURUS_RES_NACK;
            break;
        }
        x = R_RTE_COMAL_SendAck(guest, &result);
        if (x != 0) {
            R_PRINT_Log("[ComsvMain]: locComServerApp R_RTE_COMAL_SendResult guest(%d) error(%d)\r\n", guest, x);
            TAURUS_Error(-1);
            break;
        }

        if (result.Result == R_TAURUS_RES_NACK) {
            /* the command was not recognized, so no need to forward
             * it to the peripheral */
            continue;
        }

        /* Execute command */
        server->Peripheral[cmd.Per]->Execute(guest, &cmd, &result);

        /* Send command execution result */
        x = R_RTE_COMAL_SendResult(guest, &result);
        if (x != 0) {
            R_PRINT_Log("[ComsvMain]: locComServerApp R_RTE_COMAL_SendResult Execute command guest(%d) error(%d)\r\n", guest, x);
            TAURUS_Error(-1);
            break;
        }


    } while (server->Quit == 0);

exit:
    return 0;
}


/*******************************************************************************
   Section: Global Functions
 */


int R_COMSV_Init(int Guest)
{
    R_COMSV_Data_t            * server;
    int i;
    e_osal_return_t             osal_ret = OSAL_RETURN_FAIL;
    st_osal_thread_config_t     thread_config;

    if (Guest == 0) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_Init Guest(%d) error\r\n", Guest);
        return -1;
    }
    server = locAllocComData(Guest);
    if (!server) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_Init locAllocComData server NULL\r\n");
        return -1;
    }
    server->Guest = Guest;
    server->Quit = 0;
    server->Peripheral = R_PF_GetDeviceTable(Guest);

    for (i = 1; i < R_TAURUS_LAST_PERI_ID; i++) {
        server->Peripheral[i]->SetSendSignalImpl(&R_RTE_COMAL_SendSignal);
    }

    thread_config.priority   = LOC_COMSERVERAPP_PRIO;
//    thread_config.priority   = 10;
    thread_config.task_name  = "ComServer";
    thread_config.func        = (p_osal_thread_func_t)locComServerApp;
    thread_config.userarg     = server;
    osal_ret = R_OSAL_ThreadCreate(&thread_config, LOC_COMSRV_THREAD_CREATE, &server->comsrv_thread_handle);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_Init R_OSAL_ThreadCreate error(%d)\r\n", osal_ret);
        return -1;
    }

    return 0;
}


int R_COMSV_DeInit(int Guest)
{
    R_COMSV_Data_t *server;
    int x;
    e_osal_return_t osal_ret = OSAL_RETURN_FAIL;

    if (Guest == 0) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_DeInit Guest(%d) error\r\n", Guest);
        return -1;
    }

    /* stop thread */
    server = locFindComData(Guest);
    if (!server) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_DeInit locFindComData server NULL\r\n");
        return -1;
    }
    server->Quit = 1;
    osal_ret = R_OSAL_ThreadJoin(server->comsrv_thread_handle, 0);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_DeInit R_OSAL_ThreadJoin error(%d)\r\n", osal_ret);
        return -1;
    }

    /* free resource */
    x = locFreeComData(Guest);
    if (-1 == x) {
        R_PRINT_Log("[ComsvMain]: R_COMSV_DeInit locFreeComData error(%d)\r\n", x);
    }
    return x;
}

