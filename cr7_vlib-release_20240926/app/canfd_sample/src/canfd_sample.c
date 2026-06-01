/*************************************************************************************************************
* canfd_sample_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : canfd_sample.c
 * Version      : 1.0
 * Description  : Demostrate how to use a CIO vin device.
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "r_cio_api.h"
#include "r_cio_can.h"

#include "canfd_sample.h"

#include "r_print_api.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#define CANFD_RESOURCE_ID   0x8000U
#define LOC_MQ_CANFD_ID     (CANFD_RESOURCE_ID  + 45U) /* OSAL_MQ_CAN_APP */
#define CANFD_MQ_NUM_OF_MSG    1
#define CANFD_MQ_MSG_SIZE      sizeof(uint32_t)

#define NOT_USED            NULL

#define TIMEOUT_MS          1000 /* 1000 milisecond */

/**********************************************************************************************************************
 Exported/Imported global variables
 *********************************************************************************************************************/


/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/
static uint8_t candata[16] =
{
    0x0000,
    0x0001,
    0x0002,
    0x0003,
    0x0004,
    0x0005,
    0x0006,
    0x0007,
    0x0008,
    0x0009,
    0x000A,
    0x000B,
    0x000C,
    0x000D,
    0x000E,
    0x000F
};


/**********************************************************************************************************************
 * Function Name: canfd_sample_main
 * Description  : This function is a test program to check the transfer size.
 * Arguments    : ConfigPtr - Pointer to app configuration.
 * Return Value : void
 *********************************************************************************************************************/

void canfd_sample_main(void)
{
    int      canfd_cio_channel;
    uint8_t  i;
    uint8_t  k = 0u;
    st_osal_mq_config_t mq_config;
    osal_mq_handle_t canfd_mq_handle;
    r_cio_can_Msg_t send_can_msg;
    r_cio_can_Msg_t recv_can_msg;
    uint32_t canfd_mqbuffer = 0;
    ssize_t  ret;
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

#if defined(USE_CANFD_CH00)
    canfd_cio_channel = R_CIO_Open("CanFD:0", "rwb");
#else
    canfd_cio_channel = R_CIO_Open("CanFD:1", "rwb");
#endif
    if (0 == canfd_cio_channel) {
        R_PRINT_Log("CANFD: RCar-CanFD CIO open failed. return value = %d \r\n", canfd_cio_channel);
        while (1) {
            R_OSAL_ThreadSleepForTimePeriod(10);
        }
    }

    osal_ret = R_OSAL_MqInitializeMqConfigSt(&mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[CANFD Sample App] R_OSAL_MqInitializeMqConfigSt fail(%d)\r\n", osal_ret);
    }
    mq_config.max_num_msg = CANFD_MQ_NUM_OF_MSG;
    mq_config.msg_size    = CANFD_MQ_MSG_SIZE;
    osal_ret = R_OSAL_MqCreate(&mq_config, (osal_mq_id_t)LOC_MQ_CANFD_ID, &canfd_mq_handle);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CANFD Sample App] R_OSAL_MqCreate fail(%d), max_num_msg = %lld, msg_size = %lld, mq_id = 0x%04X\r\n",
                    osal_ret, mq_config.max_num_msg, mq_config.msg_size, LOC_MQ_CANFD_ID);
    }

    ret = R_CIO_IoCtl(canfd_cio_channel, (void *)R_CIO_CAN_REGISTER_RX_EVENT, canfd_mq_handle, NOT_USED);
    if (0 != ret) {
        R_PRINT_Log("[CANFD Sample App] R_CIO_IoCtl(R_CIO_CAN_REGISTER_RX_EVENT) error. return value = %d\r\n", ret);
    }

    do {
        osal_ret = R_OSAL_MqReceiveForTimePeriod(canfd_mq_handle, (osal_milli_sec_t)TIMEOUT_MS,
                                                 (void *)&canfd_mqbuffer, CANFD_MQ_MSG_SIZE);
    } while (OSAL_RETURN_TIME == osal_ret);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CANFD Sample App] R_OSAL_MqReceiveForTimePeriod error. return value = %d\r\n", osal_ret);
    }

    ret = R_CIO_Read(canfd_cio_channel, (void *)&recv_can_msg, sizeof(r_cio_can_Msg_t));
    if (sizeof(r_cio_can_Msg_t) != ret) {
        R_PRINT_Log("[CANFD Sample App] R_CIO_Read error. return value = %d\r\n", ret);
    }

    R_PRINT_Log("[CANFD Sample App] Receive buffer Flags(%ld), Id(%d), DataLen(%d),\n",
            recv_can_msg.Flags, recv_can_msg.Id, recv_can_msg.DataLen);
    for (i = 0; i < 16u; i++) {
        R_PRINT_Log("[CANFD Sample App] 0x%08x, 0x%08x, 0x%08x, 0x%08x, \n",
               recv_can_msg.Data[k], recv_can_msg.Data[k+1], recv_can_msg.Data[k+2], recv_can_msg.Data[k+3]);
        k = k + 4u;
    }

    send_can_msg.Flags   = 0x04;
    send_can_msg.Id      = 1;
    send_can_msg.DataLen = 64u;
    for (i = 0; i < 16; i++)
    {
        send_can_msg.Data[i] = candata[i];
    }

    ret = R_CIO_Write(canfd_cio_channel, (void *)&send_can_msg, sizeof(r_cio_can_Msg_t));
    if (sizeof(r_cio_can_Msg_t) != ret) {
        R_PRINT_Log("[CANFD Sample App] R_CIO_Write error. return value = %d\r\n", ret);
    }

    /* Waiting to finish writing the data. */
    R_OSAL_ThreadSleepForTimePeriod(100);

    ret = R_CIO_Status(canfd_cio_channel, NOT_USED, NOT_USED, NOT_USED);
    if (0 != ret) {
        R_PRINT_Log("[CANFD Sample App] R_CIO_Status error. return value = %d\r\n", ret);
    }

    /* When this API is called, the processing will be interrupted if there is a processing request immediately before. */
    ret = R_CIO_Close(canfd_cio_channel);
    if (0 != ret) {
        R_PRINT_Log("[CANFD Sample App] R_CIO_Close error. return value = %d\r\n", ret);
    }

    osal_ret = R_OSAL_MqDelete(canfd_mq_handle);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CANFD Sample App] R_OSAL_MqDelete error. return value = %d\r\n", osal_ret);
    }

    return;
}

/***********************************************************************************************************************
 End of function canfd_sample_main
 **********************************************************************************************************************/
