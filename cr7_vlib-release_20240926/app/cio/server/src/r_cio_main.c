/*************************************************************************************************************
* cio_main_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Title: Main Module

   This module contains the main and the main loop functions.
 */

/*******************************************************************************
   Section: Includes
 */
#include <string.h>
#include <stdbool.h>

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_bridge.h"
#include "r_cio_bsp.h"
#include "r_cio_server.h"
#include "r_cio_main.h"
#include "r_print_api.h"


/*******************************************************************************
   Section: Local Defines
 */

#define TIMEOUT_MS 1000 /* 1000 milisecond */

#define CIO_RESOURCE_ID_BSP      0x8000U
#define MQ_CIO_CMDMSG_MAIN      (CIO_RESOURCE_ID_BSP   +  0U)
#define MQ_CIO_RESULTMSG_MAIN   (CIO_RESOURCE_ID_BSP   +  1U)
#define THREAD_CIO_MAIN_TASK    (CIO_RESOURCE_ID_BSP   + 12U)

#define CIO_MSG_MQ_NUM_OF_MSG    10
#define CIO_MSG_MQ_MSG_SIZE      sizeof(r_cio_CmdMsg_t) // 20
#define CIO_RET_MQ_NUM_OF_MSG    10
#define CIO_RET_MQ_MSG_SIZE      sizeof(r_cio_ResultMsg_t) // 12

#define MAX_MQ_NUM 18

#define QUEUE_NAME_LEN 32


/*******************************************************************************
   Section: CIO Global Types
 */

typedef struct {
    char Name[QUEUE_NAME_LEN];
    osal_mq_handle_t mq_handle;
} Cio_MqHandle_t;

/*******************************************************************************
   Section: global  Variables
 */

Cio_MqHandle_t gCioMqHandle[MAX_MQ_NUM];


/*******************************************************************************
   Section: Local Variables
 */


/*******************************************************************************
   Variable: locQuit

   Signal flag to leave the application.
 */

static volatile unsigned char locQuit;


/*******************************************************************************
   Section: Local Variables
 */

char loc_compile_time[] = __TIME__;
char loc_compile_date[] = __DATE__;


/*******************************************************************************
   Section: Local Functions
 */


/*******************************************************************************
   Function: GetFreeMqHandle
 */
Cio_MqHandle_t *GetFreeMqHandle(const char *Name);


/*******************************************************************************
   Section: global Functions
 */
Cio_MqHandle_t *GetFreeMqHandle(const char *Name)
{
    Cio_MqHandle_t *mq;
    int i;
    int x;

    mq = NULL;
    for (i = 0; i < MAX_MQ_NUM; i++ ) {
        if (gCioMqHandle[i].Name[0] == '\0') {
            for (x = 0; x < QUEUE_NAME_LEN; x++) {
                gCioMqHandle[i].Name[x] = Name[x];
                if (Name[x] == '\0') {
                    break;
                }
            }
            mq = &gCioMqHandle[i];
            break;
        }
    }
    return mq;
}

Cio_MqHandle_t *GetMqHandl(const char *Name)
{
    Cio_MqHandle_t *mq;
    int i, cnt = 0;

    mq = NULL;
    while(NULL == mq && cnt < 10) {
        for (i = 0; i < MAX_MQ_NUM; i++ ) {
            if (strcmp(Name, gCioMqHandle[i].Name) == 0) {
                mq = &gCioMqHandle[i];
                break;
            }
        }
        R_OSAL_ThreadSleepForTimePeriod(10);
        cnt++;
    }
    return mq;
}

void InitMqHandl(osal_mq_handle_t handle)
{
    int i;

    for (i = 0; i < MAX_MQ_NUM; i++ ) {
        if (gCioMqHandle[i].mq_handle == handle) {
            gCioMqHandle[i].Name[0] = '\0';
            break;
        }
    }
}


/*******************************************************************************
   Function: loc_CioMainLoop

   The application main loop.

   This thread waits for commands sent by other threads
   and executes them.

   Parameters:
   Arg        - Thread parameter (not used).

   Returns:
   int64_t
 */

static int64_t loc_CioMainLoop(void * Arg)
{
    int x = 0;
    r_cio_CmdMsg_t cmd;
    r_cio_ResultMsg_t result;

    e_osal_return_t osal_ret;
    Cio_MqHandle_t *mq_msg;
    Cio_MqHandle_t *mq_result;
    bool receive = false;
    st_osal_mq_config_t cio_msg_mq_config;
    st_osal_mq_config_t cio_ret_mq_config;

    R_PRINT_Log("CIO loc_CioMainLoop START\r\n");
    (void)ServerInit();

    mq_msg = GetFreeMqHandle(R_CIO_QUEUE_MSG_NAME);
    if (mq_msg == NULL) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop GetFreeMqHandle(R_CIO_QUEUE_MSG_NAME) failed.\r\n");
        Error(ERR_MQ_OPEN);
        goto end;
    }

    /* create the message queue */
    osal_ret = R_OSAL_MqInitializeMqConfigSt(&cio_msg_mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop R_OSAL_MqInitializeMqConfigSt(MSG) failed(%d)\r\n", osal_ret);
        Error(ERR_MQ_OPEN);
        goto clean4;
    }
    cio_msg_mq_config.max_num_msg = CIO_MSG_MQ_NUM_OF_MSG;
    cio_msg_mq_config.msg_size    = CIO_MSG_MQ_MSG_SIZE;
    osal_ret = R_OSAL_MqCreate(&cio_msg_mq_config, MQ_CIO_CMDMSG_MAIN, &mq_msg->mq_handle);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop MQ_CIO_CMDMSG_MAIN R_OSAL_MqCreate failed(%d), max_num_msg(%lld), msg_size(%lld), mq_id(0x%04X)\r\n",
                    osal_ret, cio_msg_mq_config.max_num_msg, cio_msg_mq_config.msg_size, MQ_CIO_CMDMSG_MAIN);
        Error(ERR_MQ_OPEN);
        goto clean4;
    }

    mq_result = GetFreeMqHandle(R_CIO_QUEUE_RET_NAME);
    if (mq_result == NULL) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop GetFreeMqHandle(R_CIO_QUEUE_RET_NAME) failed.\r\n");
        Error(ERR_MQ_OPEN);
        goto clean3;
    }

    osal_ret = R_OSAL_MqInitializeMqConfigSt(&cio_ret_mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop R_OSAL_MqInitializeMqConfigSt(RET) fail(%d)\r\n", osal_ret);
        Error(ERR_MQ_OPEN);
        goto clean2;
    }
    cio_ret_mq_config.max_num_msg = CIO_RET_MQ_NUM_OF_MSG;
    cio_ret_mq_config.msg_size    = CIO_RET_MQ_MSG_SIZE;
    osal_ret = R_OSAL_MqCreate(&cio_ret_mq_config, MQ_CIO_RESULTMSG_MAIN, &mq_result->mq_handle);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop MQ_CIO_RESULTMSG_MAIN R_OSAL_MqCreate failed(%d), max_num_msg(%lld), msg_size_(%lld), mq_id(0x%04X)\r\n",
                    osal_ret, cio_ret_mq_config.max_num_msg, cio_ret_mq_config.msg_size, MQ_CIO_RESULTMSG_MAIN);
        Error(ERR_MQ_OPEN);
        goto clean2;
    }

    x = R_CIO_BSP_Init();
    if (x != 0) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop R_CIO_BSP_Init failed(%d)\r\n", x);
        Error(ERR_BSP_INIT);
        goto clean1;
    }

    do {
        /* receive the message */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(mq_msg->mq_handle, (osal_milli_sec_t)TIMEOUT_MS, (void*)&cmd, CIO_MSG_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);

        if ((OSAL_RETURN_OK != osal_ret) || (sizeof(r_cio_CmdMsg_t) < 0)) {
            R_PRINT_Log("[CioMain]: loc_CioMainLoop _OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            Error(ERR_MQ_RCV);
        } else {
            receive = true;
        }

        if( receive ) {
            result.Id = cmd.Id;
            result.Aux = 0;

            /* ACK or NACK the command */
            switch (cmd.Cmd) {
            case R_CIO_CMD_NOP:
            case R_CIO_CMD_OPEN:
            case R_CIO_CMD_EXIT:
                result.Result = R_CIO_RES_ACK;
                break;
            default:
                result.Result = R_CIO_RES_NACK;
                break;
            }
            osal_ret = R_OSAL_MqSendForTimePeriod(mq_result->mq_handle, (osal_milli_sec_t)TIMEOUT_MS, &result, CIO_RET_MQ_MSG_SIZE);
            if (OSAL_RETURN_OK != osal_ret) {
                result.Result = R_CIO_RES_ERROR;
                R_PRINT_Log("[CioMain]: loc_CioMainLoop R_OSAL_MqSendForTimePeriod failed(%d), Result(%d)\r\n", osal_ret, result.Result);
                Error(ERR_MQ_SEND);
            } else {
                /* Execute the command */
                switch (cmd.Cmd) {
                case R_CIO_CMD_NOP:
                    result.Result = R_CIO_RES_COMPLETE;
                    break;
                case R_CIO_CMD_OPEN:
                    x = ServerOpen(&cmd);
                    if (x >= 0) {
                        result.Result = R_CIO_RES_COMPLETE;
                        result.Aux = (void *)(ssize_t)x;
                    } else {
                        result.Result = R_CIO_RES_ERROR;
                    }
                    break;
                case R_CIO_CMD_EXIT:
                    result.Result = R_CIO_RES_COMPLETE;
                    locQuit = 1;
                    break;
                default:
                    result.Result = R_CIO_RES_ERROR;
                    break;
                }
            }
            osal_ret = R_OSAL_MqSendForTimePeriod(mq_result->mq_handle, (osal_milli_sec_t)TIMEOUT_MS, &result, CIO_RET_MQ_MSG_SIZE);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioMain]: loc_CioMainLoop R_OSAL_MqSendForTimePeriod failed(%d), Result(%d)\r\n", osal_ret, result.Result);
                Error(ERR_MQ_SEND);
            }
        }
    } while (!locQuit);

    x = R_CIO_BSP_DeInit();
    if (x != 0) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop R_CIO_BSP_DeInit failed(%d).\r\n", x);
        Error(ERR_BSP_DEINIT);
    }

clean1:
    osal_ret = R_OSAL_MqDelete(mq_result->mq_handle);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop R_OSAL_MqDelete(mq_result->mq_handle) failed(%d)\r\n", osal_ret);
        Error(ERR_MQ_CLOSE);
    }
    /* CIO sever will unlink its queues, which will succeed only, if no other
       application has an open connection to it.
       It is not POSIX, but it might be better, if the OS kernel would postpone
       the unlink operation until last connection is closed or even force a
       close instead of returning with an error and leavon the MQ alive.
     */
    /* give other apps at least some time to close their connections */
    (void)R_OSAL_ThreadSleepForTimePeriod(10);

clean2:
    InitMqHandl(mq_result->mq_handle);

clean3:
    osal_ret = R_OSAL_MqDelete(mq_msg->mq_handle);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioMain]: loc_CioMainLoop R_OSAL_MqDelete(mq_msg->mq_handle) failed(%d)\r\n", osal_ret);
        Error(ERR_MQ_CLOSE);
    }
    /* CIO sever will unlink its queues, which will succeed only, if no other
       application has an open connection to it.
       It is not POSIX, but it might be better, if the OS kernel would postpone
       the unlink operation until last connection is closed or even force a
       close instead of returning with an error and leavon the MQ alive.
     */
    /* give other apps at least some time to close their connections */
    (void)R_OSAL_ThreadSleepForTimePeriod(10);

clean4:
    InitMqHandl(mq_msg->mq_handle);

end:
    R_PRINT_Log("CIO loc_CioMainLoop END\r\n");
    return (int64_t)x;
}




/*******************************************************************************
   Function: locAppInit

   Setup the basic threads of the application.

   The function creates the background and the main thread.

   Parameters:
   void

   Returns:
   void
 */

static void locAppInit(void)
{
    osal_thread_handle_t    thrd_hndl = OSAL_THREAD_HANDLE_INVALID;
    st_osal_thread_config_t thrd_cfg;
    locQuit = 0;
    e_osal_return_t osal_ret;

    thrd_cfg.task_name  = "CIO-server mainloop";
    thrd_cfg.func       = (p_osal_thread_func_t)loc_CioMainLoop;
    thrd_cfg.userarg    = NULL;
    thrd_cfg.priority   = OSAL_THREAD_PRIORITY_HIGHEST;
    osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, THREAD_CIO_MAIN_TASK, &thrd_hndl);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioMain]: locAppInit R_OSAL_ThreadCreate failed(%d)\r\n", osal_ret);
    }
}


/*******************************************************************************
   Function: Error
 */

void Error(int Err)
{
    R_PRINT_Log("CIO MAIN NG(%d)\r\n", Err);
}

/*******************************************************************************
   Function: main
 */

int cio_main(void)
{
    R_PRINT_Log("ROS CIO Server START  %s %s\n\r", loc_compile_date, loc_compile_time);
    locAppInit();
    R_PRINT_Log("ROS CIO Server END\n\r");
    return 0;
}
