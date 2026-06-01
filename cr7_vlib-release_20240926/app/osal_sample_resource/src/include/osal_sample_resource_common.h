/*************************************************************************************************************
* OSAL Sample App
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    osal_sample_resource_common.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  Header file of Sample application to use OSAL Thread/Thread Sync/Message API
***********************************************************************************************************************/

#ifndef OSAL_SAMPLE_RESOURCE_COMMON_H_
#define OSAL_SAMPLE_RESOURCE_COMMON_H_

#include <stdio.h>
#include "rcar-xos/osal/r_osal.h"

#define OSAL_SAMPLE_INFO(...) {printf("%s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}
#define OSAL_SAMPLE_ERR(...)  {printf("error: %s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}

#define OSAL_RESOURCE_ID        0xf000U            /* check, if static env */

/* thread IDs */
#define MQ_MAIN_TASK_ID         OSAL_RESOURCE_ID + 0
#define THREAD_MAIN_TASK_ID     OSAL_RESOURCE_ID + 1
#define THREADSYNC_MAIN_TASK_ID OSAL_RESOURCE_ID + 2
#define SUB_THREAD_ID_NO1       OSAL_RESOURCE_ID + 3
#define SUB_THREAD_ID_NO2       OSAL_RESOURCE_ID + 4
#define SUB_THREAD_ID_NO3       OSAL_RESOURCE_ID + 5

/* message queue IDs */
#define MQ_ID_NO1               OSAL_RESOURCE_ID + 0

/* mutex IDs */
#define MUTEX_ID_NO1            OSAL_RESOURCE_ID + 0

/* condition variable IDs */
#define COND_ID_NO1             OSAL_RESOURCE_ID + 0

/* message queue config */
#define MAX_NUM_OF_MSG 2;
#define MSG_SIZE sizeof(uint32_t);

/* send message */
#define MSG_CONTENT 1234U

/* timeout (ms) */
#define TIMEOUT_MS 1000 // 1000 milisecond

/* timeout (ns) */
#define TIMEOUT_NS 100000000 // 100 milisecond
#define WAIT_TIME_NS 500000000 // 500 milisecond

/* timeout (s)  */
#define WAIT_TIME_SEC 1 // 1 sec

/* max ns */
#define MAX_NS 1000000000

typedef struct st_arg_cond_mutex {
    osal_cond_handle_t cond_handle;
    osal_mutex_handle_t mtx_handle;
} st_arg_cond_mutex_t;

#endif /* End of File */
