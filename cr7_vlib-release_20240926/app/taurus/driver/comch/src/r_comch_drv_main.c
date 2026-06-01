/*************************************************************************************************************
* taurus_ether_drv_main
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"

#include "r_taurus_bridge.h"

#include "r_comch_drv_api.h"
#include "r_taurus_comch_protocol.h"

#include "r_print_api.h"
#include <string.h>
//#include "r_osal_resource_definition.h"
#define RESOURCE_ID_BSP      0x8000U
#define OSAL_THREAD_TAURUS_CAN_EVENT1             (RESOURCE_ID_BSP   +  90U)
#define LOC_TAURUS_COMCH_THREAD_ID OSAL_THREAD_TAURUS_CAN_EVENT1

#define LOC_NUM_COMCH_IF          1

struct comch_read_buffer_t {
    char data[64];
    int size;
};

struct ComchInstance {
    char AppName[64];
    int Handle;
    int Id;
    int Initialized;
    int EventThreadShouldStop;
    osal_thread_handle_t ComchThreadHandle;
};

static struct comch_read_buffer_t read_buffer=
{    
    .size=0,
};

static struct ComchInstance loc_ComchInstances[LOC_NUM_COMCH_IF] = {
    {
        .AppName = "MyTestApp",
        .Id = 0,
        .Handle = -1,
        .Initialized = 0,
	.ComchThreadHandle = NULL,
	.EventThreadShouldStop = 0,	
    },
};

static struct ComchInstance *comchInstance;

static int (*locSendSignal)(int Guest, R_TAURUS_SignalId_t Signal, void* Option) = 0;

static int loc_ioctl_myfunction1(struct taurus_comch_myfunction1_in *in,
                              struct taurus_comch_myfunction1_out *out)
{
    int ret = 0;
    (void)in;
    (void)out;
    R_PRINT_Log("[TaurusComchMain] R_CIO_COMCH_IOC_MYFUNCTION1 \r\n", ret);
    return ret;
}

static int loc_ioctl_myfunction2(struct taurus_comch_myfunction2_in *in,
                              struct taurus_comch_myfunction2_out *out)
{
    int ret = 0;
    (void)in;
    (void)out;
    R_PRINT_Log("[TaurusComchMain] R_CIO_COMCH_IOC_MYFUNCTION2 \r\n", ret);
    return ret;
}

static int64_t loc_EventThreadFunc(void *data)
{
    R_TAURUS_SignalId_t signal = 0;
    struct ComchInstance *comchInstance = (struct ComchInstance *)data;
    static int count=0;

    signal = COMCH_PROTOCOL_EVENT_PKT_AVAIL;

    /* Init some message queue in the open function and pass handle here */
    /* Establish communication with other application here */

    while (0 == comchInstance->EventThreadShouldStop) {
	/* wait for receive some data here */
	R_OSAL_ThreadSleepForTimePeriod(3000); /* Send an event every 3 seconds here */
	count++;
	sprintf(read_buffer.data,"I am sending a nice signal %d.\n",count);
	read_buffer.size=strlen(read_buffer.data);
	locSendSignal(1, signal, NULL);	
    }

    /* Delete message queue or terminate connection */
    /* Could also be done in close */
    
    return 0;
}

static int32_t loc_CreateEventThread(struct ComchInstance *loc_comchInstance)
{
    int32_t         ret      = 0;
    e_osal_return_t osal_ret = OSAL_RETURN_FAIL;
    st_osal_thread_config_t config;

    config.func        = (p_osal_thread_func_t)loc_EventThreadFunc;
    config.userarg     = loc_comchInstance;
    osal_ret = R_OSAL_ThreadCreate(&config, LOC_TAURUS_COMCH_THREAD_ID, &loc_comchInstance->ComchThreadHandle);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[TaurusComchMain]: CreateEventThread Thread create failed(%d)\r\n", osal_ret);
        ret = -1;
    }
    return ret;
}


int R_COMCH_DRV_Open(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_comch_cmd_msg     *cmd;
    struct taurus_comch_res_msg     *res;
    struct taurus_comch_init_in     *params_in;
    struct taurus_comch_init_out    *params_out;
    int handle;
    int ret = -1;
    (void)Guest;
    cmd = (struct taurus_comch_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_comch_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_comch_res_msg);
    
    params_in  = &cmd->params.open;
    params_out = &res->params.open;

    params_out->cookie = params_in->cookie;

    comchInstance = &loc_ComchInstances[0];
    
    if (1 == comchInstance->Initialized) {
        params_out->res = 2;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Open failed(%lld)\r\n", params_out->res);
    } else {
	/* Connect destination application here */
	R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Open connecting %s\r\n", comchInstance->AppName);
        /* Create some connections here */
	handle=1;
        if (handle <= 0) {
            params_out->res = 2;
            R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Open failed(%lld)\r\n", params_out->res);
        } else {
            comchInstance->Handle = handle;
            comchInstance->Initialized = 1;
	    comchInstance->EventThreadShouldStop = 0;
            ret = (int)loc_CreateEventThread(comchInstance);
	    R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Open create success\r\n");
            if (0 == ret) {
                params_out->res = 0;
            } else {
                R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Open create thread failed\r\n");
            }
        }
    }
    return ret;
}

int R_COMCH_DRV_Close(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_comch_cmd_msg     *cmd;
    struct taurus_comch_res_msg     *res;
    struct taurus_comch_close_in    *params_in;
    struct taurus_comch_close_out   *params_out;

    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int ret = -1;
    (void)Guest;

    cmd = (struct taurus_comch_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_comch_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_comch_res_msg);

    params_in = &cmd->params.close;
    params_out = &res->params.close;

    params_out->cookie = params_in->cookie;

    /* find instance for your connection here */
    comchInstance = &loc_ComchInstances[0];

    if (0 == comchInstance->Initialized) {
        params_out->res = 2;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Close failed(%lld)\r\n", params_out->res);
    } else {
	/* Close connection to application here */	
        /* ret = R_CIO_Close(comchInstance->Handle); */
	ret=0;
        if (0 != ret) {
            params_out->res = 3;
            R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Close failed(%lld)\r\n", params_out->res);
        } else {
            comchInstance->Initialized = 0;
            comchInstance->Handle = -1;
	    /* Comch has an Event thread here, TODO: check if this also needed for comch */
            comchInstance->EventThreadShouldStop = 1;
            params_out->res = 0;

            osal_ret = R_OSAL_ThreadJoin(comchInstance->ComchThreadHandle, 0);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[TaurusComchMain]: Thread join failed(%d)\n", osal_ret);
            }
	    
        }
	R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Close success\r\n");
    }
    return ret;
}


ssize_t R_COMCH_DRV_Read(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_comch_cmd_msg     *cmd;
    struct taurus_comch_res_msg     *res;
    struct taurus_comch_receive_in     *params_in;
    struct taurus_comch_receive_out    *params_out;
    char read_buffer[]="This is Cr7 special message.\n";
    int LenByte;
    static int count = 0;
    
    ssize_t ret = -1;
    (void)Guest;

    cmd = (struct taurus_comch_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_comch_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_comch_res_msg);

    params_in  = &cmd->params.receive;
    params_out = &res->params.receive;

    params_out->cookie = params_in->cookie;

    comchInstance = &loc_ComchInstances[0];

    if (0 == comchInstance->Initialized) {
        params_out->res = 2;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Read failed(%lld)\r\n", params_out->res);
        goto exit;
    }

    /* Do some read for the application here, or send some previously prepared message here */
    /* ret = R_CIO_Read(comchInstance->Handle, (void *)&comch_receive, sizeof(r_cio_comch_receive));*/
    /* TODO: check how the buffers are transmitted */

    LenByte = strlen(read_buffer)+1;
    if(params_in->type==0) {
        params_out->LenByte = LenByte;
        memcpy(params_out->MessageBuffer, read_buffer, LenByte);
        if (count % 3000 == 0) {
            R_PRINT_Log("[TaurusComchMain]:Sending read message (n=%d) by Message buffer\n", count);
        }
        ret = 0;
        count++;
    } else if (params_in->type==1) {
        R_PRINT_Log("[TaurusComchMain]:Sending read message via pointer (Zero Copy)\n");
        params_out->BufferAddress = 0x13371337;
        params_out->LenByte = 42;
        ret = 0;
    } else {
        R_PRINT_Log("Received unkown type\n");
        ret = -1;
    }    
    
    ret = 0;
    if (ret != 0 ) {
        params_out->res = ret;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Read failed(%lld)\r\n", params_out->res);
        ret = -1;
    } else {
	    // R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Read success\n");
        /* params_out->RxStatus = comch_receive.RxStatus; */
        params_out->res = 0;
        ret = 0;
    }
exit:
    return ret;
}


ssize_t R_COMCH_DRV_Write(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    struct taurus_comch_cmd_msg     *cmd;
    struct taurus_comch_res_msg     *res;
    struct taurus_comch_transmit_in    *params_in;
    struct taurus_comch_transmit_out   *params_out;
    char write_buffer[64];
    
    int ret = -1;
    (void)Guest;

    static int count = 0;

    /* r_cio_comch_transmit comch_tx; */


    cmd = (struct taurus_comch_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_comch_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_comch_res_msg);

    params_in  = &cmd->params.transmit;
    params_out = &res->params.transmit;

    params_out->cookie = params_in->cookie;

    comchInstance = &loc_ComchInstances[0];

    if (0 == comchInstance->Initialized) {
        params_out->res = 2;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Write failed(%lld)\r\n", params_out->res);
        goto exit;
    }

    if(params_in->type==0) {
        memcpy(write_buffer,params_in->MessageBuffer,params_in->LenByte);
        if (count % 3000 == 0) {
            R_PRINT_Log("[TaurusComchMain]:  (n=%d), the message is:%s\n", count, write_buffer);
        }
        ret = 0;
        count++;
    } else if (params_in->type==1) {
        R_PRINT_Log("[TaurusComchMain]:Received address, the address is:%08x\n",params_in->BufferAddress);
        ret = 0;
    } else {
        R_PRINT_Log("Received unkown type\n");
        ret = -1;
    }

    /* ret = R_CIO_Write(comchInstance->Handle, (void *)&comch_tx, sizeof(r_cio_comch_transmit)); */
    if (ret != 0) {
    /* if (ret != sizeof(r_cio_comch_transmit)) { */
        params_out->res = 3;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Write failed(%lld)\r\n", params_out->res);
        ret = -1;
    } else {
        params_out->res = 0;
        ret = 0;
    }
exit:
    return ret;
}

ssize_t R_COMCH_DRV_IoCtl(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    ssize_t x;
    uint32_t ioctl_id;
    struct taurus_comch_cmd_msg     *cmd;
    struct taurus_comch_res_msg     *res;
    (void)Guest;

    ioctl_id = Cmd->Par1;
    cmd = (struct taurus_comch_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_comch_res_msg *)(size_t)Cmd->Par3;

    res->hdr.Aux = sizeof(struct taurus_comch_res_msg);

    switch (ioctl_id) {
    case COMCH_PROTOCOL_IOC_MYFUNCTION1:
        x = loc_ioctl_myfunction1(&cmd->params.myfunction1, &res->params.myfunction1);
        break;
    case COMCH_PROTOCOL_IOC_MYFUNCTION2:
        x = loc_ioctl_myfunction2(&cmd->params.myfunction2, &res->params.myfunction2);
        break;
    default:
        x = 1; /* fail */
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_IoCtl ioctl_id(%d) failed(%d)\r\n", ioctl_id, x);
        break;
    }
    return x;
}


ssize_t R_COMCH_DRV_Status(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{

    struct taurus_comch_cmd_msg     *cmd;
    struct taurus_comch_res_msg     *res;
    ssize_t ret;

    cmd = (struct taurus_comch_cmd_msg *)(size_t)Cmd->Par2;
    res = (struct taurus_comch_res_msg *)(size_t)Cmd->Par3;
    (void)Guest;
    (void)cmd;

    comchInstance = &loc_ComchInstances[0];
    /* r_cio_comch_get_comch_stats comch_get_comch_stats; */

    /* comch_get_comch_stats.CtrlIdx = cmd->params.get_comch_stats.CtrlIdx; */
    /* ret = R_CIO_Status(comchInstance->Handle, &comch_get_comch_stats, 0, 0); */
    ret = 0;
    if (0 != ret) {
        res->params.get_comch_stats.res = 1;
        R_PRINT_Log("[TaurusComchMain]: R_COMCH_DRV_Status failed(%lld)\r\n", res->params.get_comch_stats.res);
    } else {
        res->params.get_comch_stats.res = 0;
	/* res->params.get_comch_stats.comchStats = comch_get_comch_stats.comchStats; */
	res->params.get_comch_stats.comchStats = 0;
        res->hdr.Aux = sizeof(struct taurus_comch_res_msg);
    }

    return 0;
}

void R_COMCH_DRV_SetSendSignalImpl(int (*SendSignalImpl)(int Guest, R_TAURUS_SignalId_t Signal, void* Option))
{
    locSendSignal = SendSignalImpl;
}
