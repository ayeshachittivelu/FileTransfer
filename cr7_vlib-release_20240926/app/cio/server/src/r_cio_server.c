/*************************************************************************************************************
* cio_server_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/



/*******************************************************************************
   Title: Server Module

   This module contains the server thread for the I/O channels
 */

/*******************************************************************************
   Section: Includes
 */

#include <string.h>
#include <stdbool.h>

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_bridge.h"
#include "r_cio_bsp.h"
#include "r_cio_main.h"
#include "r_cio_server.h"
#include "r_print_api.h"


/*******************************************************************************
   Section: Local Defines
 */


#define LOC_MAX_CHANNEL_NUM        8
#define LOC_MAX_MSG_NUM           10



typedef struct {
    char MsgBuffer[LOC_MAX_MSG_NUM * sizeof(r_cio_CmdMsg_t)];
    char ResultBuffer[LOC_MAX_MSG_NUM * sizeof(r_cio_ResultMsg_t)];
    r_cio_Driver_t *Driver;
    size_t Addr;
    int InUseBy;
    osal_mutex_id_t mutex_id;
    osal_mutex_handle_t mutex_handle;
    osal_thread_id_t dummy_thread_id;
    osal_thread_id_t canfd_thread_id;
    osal_thread_id_t vin_thread_id;
    osal_thread_id_t wm_thread_id;
    osal_thread_id_t eth_thread_id;
    osal_thread_id_t video_thread_id;
    osal_thread_handle_t thread_handle;
    osal_mq_id_t mq_cmdmsg_id;
    osal_mq_id_t mq_result_id;

} Cio_ChannelData_t;

#define TIMEOUT_MS 1000 // 1000 milisecond

#define CIO_RESOURCE_ID_BSP      0x8000U

#define MUTEX_CIO_00        (CIO_RESOURCE_ID_BSP   + 12U)
#define THREAD_CIO_DUMMY_00 (CIO_RESOURCE_ID_BSP   + 37U)
#define THREAD_CIO_CANFD_00 (CIO_RESOURCE_ID_BSP   + 13U)
#define THREAD_CIO_VIN_00   (CIO_RESOURCE_ID_BSP   + 29U)
#define THREAD_CIO_WM_00    (CIO_RESOURCE_ID_BSP   + 21U)
#define THREAD_CIO_ETHER_00 (CIO_RESOURCE_ID_BSP   + 50U)
#define THREAD_CIO_VIDEO_00 (CIO_RESOURCE_ID_BSP   + 65U)
#define MQ_CIO_RESULT_00    (CIO_RESOURCE_ID_BSP   + 10U)
#define MQ_CIO_CMD_00       (CIO_RESOURCE_ID_BSP   +  2U)

#define CIO_MSG_MQ_NUM_OF_MSG    10
#define CIO_MSG_MQ_MSG_SIZE      sizeof(r_cio_CmdMsg_t) // 20
#define CIO_RET_MQ_NUM_OF_MSG    10
#define CIO_RET_MQ_MSG_SIZE      sizeof(r_cio_ResultMsg_t) // 12

#define QUEUE_NAME_LEN 32

/*******************************************************************************
   Section: CIO Global Types
 */

typedef struct {
    char Name[QUEUE_NAME_LEN];
    osal_mq_handle_t mq_handle;
} Cio_MqHandle_t;

/*******************************************************************************
   Function: GetFreeMqHandle
 */
Cio_MqHandle_t *GetFreeMqHandle(const char *Name);

/*******************************************************************************
   Function: InitMqHandl
 */
void InitMqHandl(osal_mq_handle_t handle);

/*******************************************************************************
   Section: Local Variables
 */

static int locChannelId;
static Cio_ChannelData_t locChannelData[LOC_MAX_CHANNEL_NUM];



/*******************************************************************************
   Section: Local Functions
 */

/*******************************************************************************
   Function: ServerInit
 */

static Cio_ChannelData_t *locAllocFreeChannel(int Id)
{
    Cio_ChannelData_t *ch;
    int i;

    i = 0;
    ch = 0;
    while (i < LOC_MAX_CHANNEL_NUM) {
        if (locChannelData[i].InUseBy == 0) {
            locChannelData[i].InUseBy = Id;
            locChannelData[i].mutex_id          = MUTEX_CIO_00          +i;
            locChannelData[i].dummy_thread_id   = THREAD_CIO_DUMMY_00   +i;
            locChannelData[i].canfd_thread_id   = THREAD_CIO_CANFD_00   +i;
            locChannelData[i].vin_thread_id     = THREAD_CIO_VIN_00     +i;
            locChannelData[i].wm_thread_id      = THREAD_CIO_WM_00      +i;
            locChannelData[i].eth_thread_id     = THREAD_CIO_ETHER_00   +i;
            locChannelData[i].video_thread_id   = THREAD_CIO_VIDEO_00   +i;
            locChannelData[i].mq_cmdmsg_id      = MQ_CIO_CMD_00         +i;
            locChannelData[i].mq_result_id      = MQ_CIO_RESULT_00      +i;
            ch = &locChannelData[i];
            break;
        }
        i++;
    }
    return ch;
}


/*******************************************************************************
   Function: loc_Server

   The application main loop.

   This thread waits for commands sent by other threads
   and executes them.

   Parameters:
   Arg        - Thread parameter (not used).

   Returns:
   int64_t
 */

static int64_t locServerThread(void * Arg)
{
    int ch_id;
    int quit;
    int x;
    char mq_name[0x100];
    char result_name[0x100];
    Cio_ChannelData_t *ch;
    r_cio_CmdMsg_t cmd;
    r_cio_ResultMsg_t result;
    e_osal_return_t osal_ret;
    Cio_MqHandle_t *mq_msg;
    Cio_MqHandle_t *mq_result;
    bool receive = false;
    st_osal_mq_config_t cio_msg_mq_config;
    st_osal_mq_config_t cio_ret_mq_config;
    osal_thread_handle_t cio_thread_handle;

    quit = 0;


    ch = (Cio_ChannelData_t *)Arg;
    ch_id = ch->InUseBy;
    R_PRINT_Log("CIO Channel %d START\r\n", ch_id);

    (void)sprintf(mq_name, "%s%d", R_CIO_QUEUE_MSG_NAME, ch_id);
    R_PRINT_Log("%s\r\n", mq_name);
    (void)sprintf(result_name, "%s%d", R_CIO_QUEUE_RET_NAME, ch_id);
    R_PRINT_Log("%s\r\n", result_name);

    /* create the message queue */
    mq_msg = GetFreeMqHandle(mq_name);
    if (mq_msg == NULL) {
        R_PRINT_Log("[CioSrv]: locServerThread GetFreeMqHandle failed. mq_name is NULL.\r\n");
        Error(ERR_MQ_OPEN);
        goto end;
    }

    osal_ret = R_OSAL_MqInitializeMqConfigSt(&cio_msg_mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[CioSrv]: locServerThread R_OSAL_MqInitializeMqConfigSt(MSG) failed(%d)\r\n", osal_ret);
        Error(ERR_MQ_OPEN);
        goto end;
    }
    cio_msg_mq_config.max_num_msg = CIO_MSG_MQ_NUM_OF_MSG;
    cio_msg_mq_config.msg_size    = CIO_MSG_MQ_MSG_SIZE;
    osal_ret = R_OSAL_MqCreate(&cio_msg_mq_config, ch->mq_cmdmsg_id, &mq_msg->mq_handle);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioSrv]: locServerThread CmdMsg R_OSAL_MqCreate failed(%d), max_num_msg(%lld), msg_size(%lld), mq_id(0x%04X)\r\n",
                    osal_ret, cio_msg_mq_config.max_num_msg, cio_msg_mq_config.msg_size, ch->mq_cmdmsg_id);
        Error(ERR_MQ_OPEN);
        goto end;
    }

    mq_result = GetFreeMqHandle(result_name);
    if (mq_result == NULL) {
        R_PRINT_Log("[CioSrv]: locServerThread GetFreeMqHandle failed. mq_result is NULL.\r\n");
        Error(ERR_MQ_OPEN);
        goto end;
    }

    osal_ret = R_OSAL_MqInitializeMqConfigSt(&cio_ret_mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[CioSrv]: locServerThread R_OSAL_MqInitializeMqConfigSt(RET) failed(%d)\r\n", osal_ret);
        Error(ERR_MQ_OPEN);
        goto end;
    }
    cio_ret_mq_config.max_num_msg = CIO_RET_MQ_NUM_OF_MSG;
    cio_ret_mq_config.msg_size    = CIO_RET_MQ_MSG_SIZE;
    osal_ret = R_OSAL_MqCreate(&cio_ret_mq_config, ch->mq_result_id, &mq_result->mq_handle);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioSrv]: locServerThread Result R_OSAL_MqCreate failed(%d), max_num_msg(%lld), msg_size(%lld), mq_id(0x%04X)\r\n",
                    osal_ret, cio_ret_mq_config.max_num_msg, cio_ret_mq_config.msg_size, ch->mq_result_id);
        Error(ERR_MQ_OPEN);
        goto end;
    }

    x = ch->Driver->Open(ch->Addr, 0, 0, 0);
    if (x != 0) {
        R_PRINT_Log("[CioSrv]: locServerThread Driver open failed(%d).\r\n", x);
        Error(-1);
        goto end;
    }

    do {
        /* receive the message */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(mq_msg->mq_handle, (osal_milli_sec_t)TIMEOUT_MS, (void*)&cmd, CIO_MSG_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);

        if ((OSAL_RETURN_OK != osal_ret) || (sizeof(r_cio_CmdMsg_t) < 0)) {
            R_PRINT_Log("[CioSrv]: locServerThread R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
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
            case R_CIO_CMD_CLOSE:
            case R_CIO_CMD_READ:
            case R_CIO_CMD_WRITE:
            case R_CIO_CMD_IOCTL:
            case R_CIO_CMD_STATUS:
                result.Result = R_CIO_RES_ACK;
                break;
            default:
                result.Result = R_CIO_RES_NACK;
                break;
            }
            osal_ret = R_OSAL_MqSendForTimePeriod(mq_result->mq_handle, (osal_milli_sec_t)TIMEOUT_MS, &result, CIO_RET_MQ_MSG_SIZE);
            if (OSAL_RETURN_OK != osal_ret) {
                result.Result = R_CIO_RES_ERROR;
                R_PRINT_Log("[CioSrv]: locServerThread R_OSAL_MqSendForTimePeriod failed(%d), Result(%d)\r\n",
                            osal_ret, result.Result);
                Error(ERR_MQ_SEND);
            } else {

                /* Execute the command */
                switch (cmd.Cmd) {
                case R_CIO_CMD_NOP:
                    result.Result = R_CIO_RES_COMPLETE;
                    break;
                case R_CIO_CMD_CLOSE:
                    osal_ret = R_OSAL_ThreadSelf(&cio_thread_handle);
                    if (OSAL_RETURN_OK == osal_ret) {
                        x = ch->Driver->Close(ch->Addr, cmd.Par1, cmd.Par2, cmd.Par3);
                        if (x >= 0) {
                            result.Result = R_CIO_RES_COMPLETE;
                            if (0 == x) {
                                /* exit thread*/
                                quit = 1;
                                result.Aux = (void*)cio_thread_handle;
                            }
                        } else {
                            result.Result = R_CIO_RES_ERROR;
                        }
                    } else {
                        result.Result = R_CIO_RES_ERROR;
                    }
                    break;
                case R_CIO_CMD_READ:
                    x = ch->Driver->Read(ch->Addr, cmd.Par1, cmd.Par2, cmd.Par3);
                    result.Aux = (void *)(size_t)x;
                    if (x >= 0) {
                        result.Result = R_CIO_RES_COMPLETE;
                    } else {
                        result.Result = R_CIO_RES_ERROR;
                    }
                    break;
                case R_CIO_CMD_WRITE:
                    x = ch->Driver->Write(ch->Addr, cmd.Par1, cmd.Par2, cmd.Par3);
                    result.Aux = (void *)(size_t)x;
                    if (x >= 0) {
                        result.Result = R_CIO_RES_COMPLETE;
                    } else {
                        result.Result = R_CIO_RES_ERROR;
                    }
                    break;
                case R_CIO_CMD_IOCTL:
                    x = ch->Driver->IoCtl(ch->Addr, cmd.Par1, cmd.Par2, cmd.Par3);
                    result.Aux = (void *)(size_t)x;
                    if (x == 0) {
                        result.Result = R_CIO_RES_COMPLETE;
                    } else {
                        result.Result = R_CIO_RES_ERROR;
                    }
                    break;
                case R_CIO_CMD_STATUS:
                    x = ch->Driver->Status(ch->Addr, cmd.Par1, cmd.Par2, cmd.Par3);
                    result.Aux = (void *)(size_t)x;
                    if (x == 0) {
                        result.Result = R_CIO_RES_COMPLETE;
                    } else {
                        result.Result = R_CIO_RES_ERROR;
                    }
                    break;
                default:
                    result.Result = R_CIO_RES_ERROR;
                    break;
                }
                if (R_CIO_RES_COMPLETE != result.Result) {
                    R_PRINT_Log("[CioSrv]: locServerThread failed. cmd.Cmd(%d), result.Result(%d)\r\n",
                                cmd.Cmd, result.Result);
                }
            }
            osal_ret = R_OSAL_MqSendForTimePeriod(mq_result->mq_handle, (osal_milli_sec_t)TIMEOUT_MS, &result, CIO_RET_MQ_MSG_SIZE);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioSrv]: locServerThread R_OSAL_MqSendForTimePeriod failed(%d), Result(%d)\r\n",
                            osal_ret, result.Result);
                Error(ERR_MQ_SEND);
            }
        }
    } while (!quit);

    ch->InUseBy = 0;


end:
    R_PRINT_Log("CIO Channel %d END\r\n", ch_id);
    return 0;
}


/*******************************************************************************
   Section: Global Functions
 */


/*******************************************************************************
   Function: ServerOpen
 */

int ServerOpen(r_cio_CmdMsg_t  *Cmd)
{
    Cio_ChannelData_t       *ch;
    int x;
    st_osal_thread_config_t thrd_cfg;
    e_osal_return_t         osal_ret = OSAL_RETURN_OK;

    locChannelId++;

    ch = locAllocFreeChannel(locChannelId);
    if (ch == 0) {
        R_PRINT_Log("[CioSrv]: ServerOpen CIO failed to allocate channel\r\n");
        Error(-1);
        return -1;
    }
    x = R_CIO_BSP_GetDriver(Cmd->Par1, &ch->Driver, &ch->Addr);
    if (x != 0) {
        R_PRINT_Log("[CioSrv]: ServerOpen CIO failed to find driver\r\n");
        Error(-1);
        ch->InUseBy = 0;
        return -1;
    }

    thrd_cfg.priority   = OSAL_THREAD_PRIORITY_HIGHEST;
    thrd_cfg.func       = (p_osal_thread_func_t)locServerThread;
    thrd_cfg.userarg    = (void *)ch;
    thrd_cfg.task_name = "CIO-driver";

    if (strcmp(ch->Driver->Name, "Dummy") == 0) {
        osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, ch->dummy_thread_id, &ch->thread_handle);
    }else if(strcmp(ch->Driver->Name, "R-CAR_cio_canfd") == 0){
        osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, ch->canfd_thread_id, &ch->thread_handle);
    }else if(strcmp(ch->Driver->Name, "R-CAR_VIN") == 0){
        osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, ch->vin_thread_id, &ch->thread_handle);
    }else if(strcmp(ch->Driver->Name, "R-Car_WM") == 0){
        osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, ch->wm_thread_id, &ch->thread_handle);
    }else if(strcmp(ch->Driver->Name, "R-CAR_cio_ether") == 0){
        osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, ch->eth_thread_id, &ch->thread_handle);
    }else if(strcmp(ch->Driver->Name, "R-CAR_cio_video") == 0){
        osal_ret = R_OSAL_ThreadCreate(&thrd_cfg, ch->video_thread_id, &ch->thread_handle);
    }else{
        R_PRINT_Log("[CioSrv]: ServerOpen CIO failed to driver name\r\n");
        Error(-1);
        return -1;
    }

    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioSrv]: ServerOpen R_OSAL_ThreadCreate failed(%d). DriverName(%s)\r\n",
                    osal_ret, ch->Driver->Name);
    }

    return locChannelId;
}


/*******************************************************************************
   Function: ServerInit
 */

int ServerInit(void)
{
    int i;

    locChannelId = 0;
    for (i = 0; i < LOC_MAX_CHANNEL_NUM; i++) {
        locChannelData[i].InUseBy = 0;
    }
    return 0;
}

