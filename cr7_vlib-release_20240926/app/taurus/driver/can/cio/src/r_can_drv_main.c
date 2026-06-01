/*************************************************************************************************************
* taurus_can_drv_main
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"

#include "r_taurus_bridge.h"

#include "r_can_drv_api.h"
#include "r_taurus_can_protocol.h"

#include "r_cio_api.h"
#include "r_cio_can.h"

#include "r_print_api.h"

#define LOC_NUM_CAN_IF          2


/*******************************************************************************
   Section: Local Defines
 */

#define LOC_EVENTTHREAD_PRIO       0x24
#define LOC_EVENTTHREAD_STACKSIZE  0x2000

/* Create Resource ID */
#define LOC_TAURUS_CAN_RESOURCE_ID 0x8000U                              /* RESOURCE_ID_BSP               */
#define LOC_TAURUS_CAN0_THREAD_ID  (LOC_TAURUS_CAN_RESOURCE_ID + 9U)    /* OSAL_THREAD_TAURUS_CAN_EVENT  */
#define LOC_TAURUS_CAN1_THREAD_ID  (LOC_TAURUS_CAN_RESOURCE_ID + 90U)   /* OSAL_THREAD_TAURUS_CAN_EVENT1 */
#define LOC_TAURUS_CAN0_MQ_ID      (LOC_TAURUS_CAN_RESOURCE_ID + 19U)   /* OSAL_MQ_TAURUS_PARACAN        */
#define LOC_TAURUS_CAN1_MQ_ID      (LOC_TAURUS_CAN_RESOURCE_ID + 67U)   /* OSAL_MQ_PARA_CAN              */


/* timeout (ms) */
#define LOC_CAN_TIMEOUT_MS 1000 /* 1000 milisecond */

#define NOT_USED NULL

/* Mq definitions */
#define LOC_TAURUS_CAN_MQ_NUM_OF_MSG    1
#define LOC_TAURUS_CAN_MQ_MSG_SIZE      sizeof(uint32_t)

/*******************************************************************************
   Section: Local Types
 */

struct CanInstance {
    char CioName[64];
    int Id;
    int Handle;
    int Initialized;
    osal_thread_handle_t CanThreadHandle;
    uint64_t EventThreadStack[LOC_EVENTTHREAD_STACKSIZE >> 3];
    int EventThreadShouldStop;
    osal_mq_handle_t CanMqHandle;
};

/*******************************************************************************
   Section: Local Variables
 */

static int (*locSendSignal)(int Guest, R_TAURUS_SignalId_t Signal, void* Option) = 0;


static struct CanInstance loc_CanInstances[LOC_NUM_CAN_IF] = {
    {
#if defined (R_USE_CANFD)
        .CioName = "CanFD:0",
#else
        .CioName = "Can:0",
#endif
        .Id = 0,
        .Handle = -1,
        .Initialized = 0,
    },
    {
#if defined (R_USE_CANFD)
        .CioName = "CanFD:1",
#else
        .CioName = "Can:1",
#endif
        .Id = 1,
        .Handle = -1,
        .Initialized = 0,
    },
};

static void *my_memcpy(void *dst, const void *src, unsigned long n)
{
    uint8_t *_dst, *_src;

    _dst = (uint8_t *)dst;
    _src = (uint8_t *)src;

    while (n--)
        *(_dst++) = *(_src++);

    return dst;
}

/* stub */
static int loc_CAN_DRV_ioctl_op1(struct taurus_can_ioc_op1_in *in,
                                 struct taurus_can_ioc_op1_out *out)
{
    out->cookie = in->cookie;
    out->res = 1;

    return 0;
}

/* stub */
static int loc_CAN_DRV_ioctl_op2(struct taurus_can_ioc_op2_in *in,
                                 struct taurus_can_ioc_op2_out *out)
{
    out->cookie = in->cookie;
    out->res = 1;

    return 0;
}


static int64_t loc_EventThreadFunc(void *data)
{
    ssize_t ret;
    R_TAURUS_SignalId_t signal = 0;
    struct CanInstance *canInstance = (struct CanInstance *)data;
    e_osal_return_t     osal_ret = OSAL_RETURN_FAIL;
    uint32_t returndata = 0;
    st_osal_mq_config_t mq_config;
    osal_mq_id_t mq_id[] = {
        LOC_TAURUS_CAN0_MQ_ID,
        LOC_TAURUS_CAN1_MQ_ID
    };

    switch (canInstance->Id) {
    case 0:
        signal = CAN_PROTOCOL_EVENT_PKT_AVAIL_CH0;
        break;
    case 1:
        signal = CAN_PROTOCOL_EVENT_PKT_AVAIL_CH1;
        break;
    default:
        R_PRINT_Log("[TaurusCanMain] loc_EventThreadFunc canInstance->Id(%d) fail\r\n", canInstance->Id);
        goto exit;
    }

    /* The Taurus CAN server sends a signal to the client whenever
     * some data is available. This way the client doesn't have to
     * poll and will request a read operation only when data is
     * ready. */
    osal_ret = R_OSAL_MqInitializeMqConfigSt(&mq_config);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[TaurusCanMain] loc_EventThreadFunc R_OSAL_MqInitializeMqConfigSt fail(%d)\r\n", osal_ret);
    }
    mq_config.max_num_msg = LOC_TAURUS_CAN_MQ_NUM_OF_MSG;
    mq_config.msg_size    = LOC_TAURUS_CAN_MQ_MSG_SIZE;
    osal_ret = R_OSAL_MqCreate(&mq_config, mq_id[canInstance->Id], &canInstance->CanMqHandle);
    if (OSAL_RETURN_OK == osal_ret) {
        ret = R_CIO_IoCtl(canInstance->Handle,
                          (void *)R_CIO_CAN_REGISTER_RX_EVENT,
                          (void *)canInstance->CanMqHandle,
                          NOT_USED);
        if (ret == -1) {
            canInstance->EventThreadShouldStop = 1;
            R_PRINT_Log("[TaurusCanMain] loc_EventThreadFunc R_CIO_IoCtl(R_CIO_CAN_REGISTER_RX_EVENT) fail(%d)\r\n", ret);
        }
        else
        {
            while (0 == canInstance->EventThreadShouldStop) {
                osal_ret = R_OSAL_MqReceiveForTimePeriod(canInstance->CanMqHandle,
                                                        (osal_milli_sec_t)LOC_CAN_TIMEOUT_MS,
                                                        (void*)&returndata,
                                                        LOC_TAURUS_CAN_MQ_MSG_SIZE);

                if ((OSAL_RETURN_OK != osal_ret) && (OSAL_RETURN_TIME != osal_ret)) {
                    R_PRINT_Log("[TaurusCanMain]: loc_EventThreadFunc R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
                }

                if (OSAL_RETURN_OK == osal_ret) {
                    locSendSignal(1, signal, NULL);
                }
            }
        }
        osal_ret = R_OSAL_MqDelete(canInstance->CanMqHandle);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[TaurusCanMain]: loc_EventThreadFunc R_OSAL_MqDelete failed(%d)\r\n", osal_ret);
        }
    } else {
        R_PRINT_Log("[TaurusCanMain]: loc_EventThreadFunc R_OSAL_MqCreate failed(%d), max_num_msg = %lld, msg_size = %lld, mq_id = 0x%04X\r\n",
                    osal_ret, mq_config.max_num_msg, mq_config.msg_size, mq_id[canInstance->Id]);
    }

exit:
    return 0;
}

static int32_t loc_CreateEventThread(struct CanInstance *canInstance)
{
    int32_t         ret      = 0;
    e_osal_return_t osal_ret = OSAL_RETURN_FAIL;
    st_osal_thread_config_t config;

    osal_thread_id_t th_id[] = {
        LOC_TAURUS_CAN0_THREAD_ID,
        LOC_TAURUS_CAN1_THREAD_ID
    };

    config.func        = (p_osal_thread_func_t)loc_EventThreadFunc;
    config.userarg     = canInstance;
    osal_ret = R_OSAL_ThreadCreate(&config, th_id[canInstance->Id], &canInstance->CanThreadHandle);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[TaurusCanMain]: loc_EventThreadFunc R_OSAL_ThreadCreate failed(%d)\r\n", osal_ret);
        ret = -1;
    }

    return ret;
}

int R_CAN_DRV_Open(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_can_cmd_msg     *cmd;
    struct taurus_can_res_msg     *res;
    struct taurus_can_open_in     *params_in;
    struct taurus_can_open_out    *params_out;

    struct CanInstance *canInstance;
    int handle;
    int ret = -1;

    cmd = (struct taurus_can_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_can_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_can_res_msg);

    params_in = &cmd->params.open;
    params_out = &res->params.open;

    params_out->cookie = params_in->cookie;

    if (params_in->can_ch >= LOC_NUM_CAN_IF) {
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Open can_ch failed(%d)\r\n", params_in->can_ch);
        params_out->res = 1;
        goto exit;
    }

    canInstance = &loc_CanInstances[params_in->can_ch];

    if (1 == canInstance->Initialized)
    {
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Open already initialized\r\n");
        params_out->res = 2;
    }
    else
    {
        handle = R_CIO_Open(canInstance->CioName, "rwb");
        if (handle <= 0) {
            R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Open cio open failed\r\n");
            params_out->res = 2;
        } else {
            canInstance->Handle = handle;
            canInstance->Initialized = 1;
            canInstance->EventThreadShouldStop = 0;
            ret = (int)loc_CreateEventThread(canInstance);
            if (0 == ret) {
                params_out->res = 0;
            } else {
                R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Open create thread failed\r\n");
            }
        }
    }

exit:
    return ret;
}

int R_CAN_DRV_Close(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_can_cmd_msg     *cmd;
    struct taurus_can_res_msg     *res;
    struct taurus_can_close_in    *params_in;
    struct taurus_can_close_out   *params_out;

    struct CanInstance *canInstance;

    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int ret = -1;

    cmd = (struct taurus_can_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_can_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_can_res_msg);

    params_in = &cmd->params.close;
    params_out = &res->params.close;

    params_out->cookie = params_in->cookie;

    if (params_in->can_ch >= LOC_NUM_CAN_IF) {
        R_PRINT_Log("[TaurusCanMain]: R_CAN_DRV_Close can_ch failed(%d)\r\n", params_in->can_ch);
        params_out->res = 1;
        goto exit;
    }

    canInstance = &loc_CanInstances[params_in->can_ch];

    if (0 == canInstance->Initialized) {
        R_PRINT_Log("[TaurusCanMain]: R_CAN_DRV_Close already closed.\r\n");
        params_out->res = 2;
        goto exit;
    }

    ret = R_CIO_Close(canInstance->Handle);
    if (0 != ret) {
        R_PRINT_Log("[TaurusCanMain]: R_CAN_DRV_Close cio close failed.\r\n");
        params_out->res = 3;
    } else {
        canInstance->Initialized = 0;
        canInstance->Handle = -1;
        canInstance->EventThreadShouldStop = 1;
        params_out->res = 0;

        osal_ret = R_OSAL_ThreadJoin(canInstance->CanThreadHandle, 0);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[TaurusCanMain]: Thread join failed(%d)\n", osal_ret);
        }
    }

exit:
    return ret;
}


ssize_t R_CAN_DRV_Read(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_can_cmd_msg     *cmd;
    struct taurus_can_res_msg     *res;
    struct taurus_can_read_in     *params_in;
    struct taurus_can_read_out    *params_out;

    struct CanInstance *canInstance;
    r_cio_can_Msg_t can_msg;
    ssize_t ret = -1;

    cmd = (struct taurus_can_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_can_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_can_res_msg);

    params_in = &cmd->params.read;
    params_out = &res->params.read;

    params_out->cookie = params_in->cookie;
    params_out->can_ch = params_in->can_ch;

    if (params_in->can_ch >= LOC_NUM_CAN_IF) {
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Read can_ch failed(%d)\r\n", params_in->can_ch);
        params_out->res = 1;
        goto exit;
    }

    canInstance = &loc_CanInstances[params_in->can_ch];

    if (0 == canInstance->Initialized) {
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Read Not Initialized.\r\n");
        params_out->res = 2;
        goto exit;
    }

    ret = R_CIO_Read(canInstance->Handle, (void *)&can_msg, sizeof(r_cio_can_Msg_t));
    if (ret != sizeof(r_cio_can_Msg_t)) {
        params_out->data_len = 0;
        params_out->res = 3;
        ret = -1;
    } else {
        params_out->node_id = can_msg.Id;
        params_out->data_len = can_msg.DataLen;
        (void)my_memcpy(&params_out->data[0], &can_msg.Data[0], can_msg.DataLen);
        params_out->res = 0;
        ret = 0;
    }

exit:
    return ret;
}


ssize_t R_CAN_DRV_Write(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_can_cmd_msg     *cmd;
    struct taurus_can_res_msg     *res;
    struct taurus_can_write_in    *params_in;
    struct taurus_can_write_out   *params_out;

    struct CanInstance *canInstance;
    r_cio_can_Msg_t can_msg;
    int ret = -1;

    cmd = (struct taurus_can_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_can_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_can_res_msg);

    params_in = &cmd->params.write;
    params_out = &res->params.write;

    params_out->cookie = params_in->cookie;

    if (params_in->can_ch >= LOC_NUM_CAN_IF) {
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Write can_ch failed(%d)\r\n", params_in->can_ch);
        params_out->res = 1;
        goto exit;
    }

    canInstance = &loc_CanInstances[params_in->can_ch];

    if (0 == canInstance->Initialized) {
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_Write Not Initialized.\r\n");
        params_out->res = 2;
        goto exit;
    }

#if defined (R_USE_CANFD)
    can_msg.Flags = 0x4;
#else
    can_msg.Flags = 0x0;
#endif
    can_msg.Id = params_in->node_id;
    can_msg.DataLen = (params_in->data_len > R_CAN_MSG_MAX_BYTES) ? R_CAN_MSG_MAX_BYTES : params_in->data_len;
    (void)my_memcpy(&can_msg.Data[0], &params_in->data[0], can_msg.DataLen);

    ret = R_CIO_Write(canInstance->Handle, (void *)&can_msg, sizeof(r_cio_can_Msg_t));
    if (ret != sizeof(r_cio_can_Msg_t)) {
        R_PRINT_Log("[TaurusCanMain]: R_CIO_Write failed(%d).\r\n", ret);
        params_out->res = 3;
        ret = -1;
    } else {
        params_out->res = 0;
        ret = 0;
    }

exit:
    return ret;
}


ssize_t R_CAN_DRV_IoCtl(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    ssize_t x;
    uint32_t ioctl_id;
    struct taurus_can_cmd_msg     *cmd;
    struct taurus_can_res_msg     *res;

    ioctl_id = Cmd->Par1;
    cmd = (struct taurus_can_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_can_res_msg *)(size_t)Cmd->Par3;

    switch (ioctl_id) {
    case CAN_PROTOCOL_IOC_OP1:
        x = loc_CAN_DRV_ioctl_op1(&cmd->params.ioc_op1, &res->params.ioc_op1);
        if (x == 0)
            res->hdr.Aux = sizeof(struct taurus_can_res_msg);
        break;
    case CAN_PROTOCOL_IOC_OP2:
        x = loc_CAN_DRV_ioctl_op2(&cmd->params.ioc_op2, &res->params.ioc_op2);
        if (x == 0)
            res->hdr.Aux = sizeof(struct taurus_can_res_msg);
        break;
    default:
        R_PRINT_Log("[TaurusCanMain]: CAN_DRV_IoCtl ioctl_id failed(%d).\r\n", ioctl_id);
        x = 1;
        break;
    }

    return x;
}


ssize_t R_CAN_DRV_Status(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    return 0;
}

void R_CAN_DRV_SetSendSignalImpl(int (*SendSignalImpl)(int Guest, R_TAURUS_SignalId_t Signal, void* Option))
{
    locSendSignal = SendSignalImpl;
}
