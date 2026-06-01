/*************************************************************************************************************
* cio_um_main_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/


#ifndef R_CIO_UM_MAIN_H
#define R_CIO_UM_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int            ChannelId;
    int            MsgId;
    int            QId;
    osal_mutex_handle_t ChannelMutex;
    osal_mq_handle_t   MsgQueue;
    osal_mq_handle_t   RetQueue;
} R_CIO_UM_Channel_t;


R_CIO_UM_Channel_t *R_CIO_PRV_FindChannel(int Id);


#define QUEUE_NAME_LEN 32
#define TIMEOUT_MS 1000 /* 1000 milisecond */

#define CIO_MSG_MQ_MSG_SIZE      sizeof(r_cio_CmdMsg_t) // 20
#define CIO_RET_MQ_MSG_SIZE      sizeof(r_cio_ResultMsg_t) // 12

/*******************************************************************************
   Section: CIO typedef struct
 */

typedef struct {
    char Name[QUEUE_NAME_LEN];
    osal_mq_handle_t mq_handle;
} Cio_MqHandle_t;

/*******************************************************************************
   Function: GetMqHandl
 */
Cio_MqHandle_t *GetMqHandl(const char *Name);

/*******************************************************************************
   Function: InitMqHandl
 */
void InitMqHandl(osal_mq_handle_t handle);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_UM_MAIN_H */




