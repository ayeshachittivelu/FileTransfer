/*************************************************************************************************************
* OSAL Sample App
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    osal_sample_resource_main.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  Sample application to use OSAL Thread/Thread Sync/Message API
***********************************************************************************************************************/

#include "osal_sample_resource_common.h"

static int shared_data = 0;

int64_t mq_main_task(void *arg);
int64_t mq_first_worker_func(void *arg);
int64_t mq_second_worker_func(void *arg);
int64_t thread_main_task(void *arg);
int64_t thread_first_sub_task(void *arg);
int64_t thread_second_sub_task(void *arg);
int64_t threadsync_main_task(void *arg);
int64_t threadsync_first_worker_func(void *arg);
int64_t threadsync_second_worker_func(void *arg);

int app_ret = 0;

/**
 * @defgroup osal_sample_resource OSAL Application: Sample For OSAL Resource Usage
 * @{
 */
/**
 * @brief The main function of OSAL Sample Resource application
 * - Call R_OSAL_Initialize and call the resource_task function. Then call R_OSAL_Deinitialize
 * - The resource_task is divided into 3 main tasks as below:
 *      -# @link mq_main_task @endlink describes the function of Message queue control
 *      -# @link thread_main_task @endlink describes the function of thread creation
 *      -# @link threadsync_main_task @endlink describes the function of Mutex control and synchronize for conditional variables
 * 
 * @return 0 on success
 * @return -1 on failure
 */
void test_main(void* arg)
{
    (void)arg;

    printf("================== osal_sample_resource start ==================\n");
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    st_osal_thread_config_t thread_config;
    osal_thread_handle_t thread_handle = OSAL_THREAD_HANDLE_INVALID;
    int64_t thread_ret = 0;

    OSAL_SAMPLE_INFO("initialize osal\n");
    app_ret = R_OSAL_Initialize();
    if(OSAL_RETURN_OK != app_ret)
    {
        OSAL_SAMPLE_ERR("R_OSAL_Initialize was failed, app_ret = %d\n", app_ret);
        return(-1);
    }

    if (0 == app_ret) {
        /* create main task for OSAL Message Queue, Thread, ThreadSync APIs */
        osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread_config);

        if(OSAL_RETURN_OK != osal_ret)
        {

            app_ret = -1;
            OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("create main task for OSAL Message Queue APIs\n");
            thread_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread_config.task_name = "MQ main task";
            thread_config.func = mq_main_task;
            thread_config.userarg = NULL;
            thread_config.stack_size = 0x2000;

            osal_ret = R_OSAL_ThreadCreate(&thread_config, (osal_thread_id_t)MQ_MAIN_TASK_ID, &thread_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("mq_main_task creation was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                /* wait for mq_main_task finish */
                osal_ret = R_OSAL_ThreadJoin(thread_handle, &thread_ret);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("join mq_main_task failed, osal_ret = %d\n", osal_ret);
                }
                else if(0 != thread_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("mq_main_task operation was failed, thread_ret = %ld\n", (long)thread_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("mq_main_task operation is completed\n");
                    thread_handle = OSAL_THREAD_HANDLE_INVALID;
                }
            }

            OSAL_SAMPLE_INFO("create main task for OSAL Thread APIs\n");
            thread_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread_config.task_name = "Thread main task";
            thread_config.func = thread_main_task;
            thread_config.userarg = NULL;
            thread_config.stack_size = 0x2000;
            osal_ret = R_OSAL_ThreadCreate(&thread_config, (osal_thread_id_t)THREAD_MAIN_TASK_ID, &thread_handle);

            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("thread_main_task creation was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                /* wait for thread_main_task finish */
                osal_ret = R_OSAL_ThreadJoin(thread_handle, &thread_ret);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("join thread_main_task failed, osal_ret = %d\n", osal_ret);
                }
                else if(0 != thread_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("thread_main_task operation was failed, thread_ret = %ld\n", (long)thread_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("thread_main_task operation is completed\n");
                    thread_handle = OSAL_THREAD_HANDLE_INVALID;
                }
            }

            OSAL_SAMPLE_INFO("create main task for OSAL ThreadSync APIs\n");
            thread_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread_config.task_name = "ThreadSync main task";
            thread_config.func = threadsync_main_task;
            thread_config.userarg = NULL;
            thread_config.stack_size = 0x2000;
            osal_ret = R_OSAL_ThreadCreate(&thread_config, (osal_thread_id_t)THREADSYNC_MAIN_TASK_ID, &thread_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("threadsync_main_task creation was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                /* wait for threadsync_main_task finish */
                osal_ret = R_OSAL_ThreadJoin(thread_handle, &thread_ret);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("join threadsync_main_task failed, osal_ret = %d\n", osal_ret);
                }
                else if(0 != thread_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("threadsync_main_task operation was failed, thread_ret = %ld\n", (long)thread_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("threadsync_main_task operation is completed\n");
                    thread_handle = OSAL_THREAD_HANDLE_INVALID;
                }
            }
        }

        OSAL_SAMPLE_INFO("de-initialize osal\n");
        osal_ret = R_OSAL_Deinitialize();
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("R_OSAL_Deinitialize was failed, osal_ret = %d\n", osal_ret);
        }
    }

    if(0 == app_ret)
    {
        OSAL_SAMPLE_INFO("osal_sample_resource run successfully\n");
    }
    else
    {
        OSAL_SAMPLE_ERR("osal_sample_resource was failed\n");
    }

    printf("================== osal_sample_resource finish ==================\n");

}

int main(void)
{
    /* Start OS and initial thread */
    /* Note: parameter unused by AutoSAR, see TASK(maintask) */
    R_OSAL_StartOS(test_main);

    return(-1);
}

/**
 * @defgroup mq_main_task OSAL Message Queue APIs Usage
 * @{
 */
/**
 * @brief Message queue main task:<br>
 * (OSAL API UM reference: Chapter 2.4.1: Message send & receive)
 * - Initalize the mq configuration object by using R_OSAL_MqInitializeMqConfigSt
 * - Create a queue which size is "MSG_SIZE" and the maximum number of message is "MAX_NUM_OF_MSG"
 * - Initalize two thread configuration objects by using R_OSAL_ThreadInitializeThreadConfigSt
 * - Create 2 threads with 2 thread configuration objects above:
 *      -# The first thread is @link mq_first_worker_func @endlink which play a role in message sending thread
 *      -# The second thread is @link mq_second_worker_func @endlink  which play a role in message receiving thread
 * - Wait for 2 threads above finish by using R_OSAL_ThreadJoin
 * - Delete the created queue with steps as below:
 *      -# Check the queue is empty or not by using R_OSAL_MqIsEmpty
 *      -# If queue is not empty, delete all remaining message in this queue by using R_OSAL_MqReset
 *      -# And finaly, delete this queue by using R_OSAL_MqDelete
 * 
 * @param[in] arg a void pointer
 * @return 0 on success
 * @return -1 on failure
 */
int64_t mq_main_task(void *arg)
{
    (void)arg;
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    st_osal_mq_config_t mq_config;
    osal_mq_handle_t mq_handle = OSAL_MQ_HANDLE_INVALID;
    bool isEmpty = false;
    st_osal_thread_config_t thread1_config, thread2_config;
    osal_thread_handle_t thread1_handle = OSAL_THREAD_HANDLE_INVALID, thread2_handle = OSAL_THREAD_HANDLE_INVALID;
    int64_t thread1_ret = 0, thread2_ret = 0;

    printf("\n================== mq_main_task start ==================\n");
    OSAL_SAMPLE_INFO("initialize mq configuration\n");
    osal_ret = R_OSAL_MqInitializeMqConfigSt(&mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("message queue config initialization was failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        OSAL_SAMPLE_INFO("create a message queue with max_num_msg: 2 and msg_size: sizeof(int)\n");
        mq_config.max_num_msg = MAX_NUM_OF_MSG;
        mq_config.msg_size = MSG_SIZE;
        osal_ret = R_OSAL_MqCreate(&mq_config, (osal_mq_id_t)MQ_ID_NO1, &mq_handle);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("message queue creation was failed, osal_ret = %d\n", osal_ret);
        }
    }

    /* create 2 worker functions to send/receive message */
    if(0 == app_ret)
    {
        osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread1_config);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread2_config);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
            }
        }

        if (0 == app_ret)
        {
            OSAL_SAMPLE_INFO("create message sending thread\n");
            thread1_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread1_config.task_name = "sending thread";
            thread1_config.func = mq_first_worker_func;
            thread1_config.userarg = (void *)&mq_handle;
            thread1_config.stack_size = 0x2000;

            thread2_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread2_config.task_name = "receiving thread";
            thread2_config.func = mq_second_worker_func;
            thread2_config.userarg = (void *)&mq_handle;
            thread2_config.stack_size = 0x2000;

            osal_ret = R_OSAL_ThreadCreate(&thread1_config, (osal_thread_id_t)SUB_THREAD_ID_NO1, &thread1_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("worker function creation was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                OSAL_SAMPLE_INFO("create message receiving thread\n");
                osal_ret = R_OSAL_ThreadCreate(&thread2_config, (osal_thread_id_t)SUB_THREAD_ID_NO2, &thread2_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR ("worker function creation was failed, osal_ret = %d\n", osal_ret);
                }

                osal_ret = R_OSAL_ThreadJoin(thread1_handle, &thread1_ret);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("join mq_first_worker_func failed, osal_ret = %d\n", osal_ret);
                }
                else if(0 != thread1_ret)
                {
                    OSAL_SAMPLE_ERR("mq_first_worker_func operation was failed, thread_ret = %ld\n", (long)thread1_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("sending thread operation is completed\n");
                }

                if(0 == app_ret)
                {
                    osal_ret = R_OSAL_ThreadJoin(thread2_handle, &thread2_ret);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("join mq_second_worker_func failed, osal_ret = %d\n", osal_ret);
                    }
                    else if(0 != thread2_ret)
                    {
                        OSAL_SAMPLE_ERR("mq_second_worker_func operation was failed, thread_ret = %ld\n", (long)thread2_ret);
                    }
                    else
                    {
                        OSAL_SAMPLE_INFO("receiving thread operation is completed\n");
                    }
                }
            }
        }
    }

    /* delete created MQ */
    if(0 == app_ret)
    {
        OSAL_SAMPLE_INFO("check mq is empty or not\n");
        osal_ret = R_OSAL_MqIsEmpty(mq_handle, &isEmpty);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("message queue empty checking failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            if(false == isEmpty)
            {
                OSAL_SAMPLE_INFO("delete all remaining messages if mq is not empty\n");
                osal_ret = R_OSAL_MqReset(mq_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("reset message in MQ was failed, osal_ret = %d\n", osal_ret);
                }
            }
            if(0 == app_ret)
            {
                OSAL_SAMPLE_INFO("delete mq\n");
                osal_ret = R_OSAL_MqDelete(mq_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("delete MQ was failed, osal_ret = %d\n", osal_ret);
                }
            }
        }
    }

    printf("================== mq_main_task finish ==================\n\n");
    return app_ret;
}

/**
 * @brief Sending thread:
 * - Get the configuration of created message queue
 * - Send two messages to the queue with steps as below:
 *      -# Check the queue is full or not by using R_OSAL_MqIsFull
 *      -# If the queue is not full, send messages which content is "MSG_CONTENT" to queue:
 *          - Send fist message by using R_OSAL_MqSendForTimePeriod
 *          - Send second message by using R_OSAL_MqSendUntilTimeStamp
 * 
 * @param[in] arg a void pointer which is the address of message queue handle
 * @return 0 on success
 * @return -1 on failure
 */
int64_t mq_first_worker_func(void *arg)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    osal_mq_handle_t *p_mq_handle = (osal_mq_handle_t *)arg;
    st_osal_mq_config_t mq_config;
    st_osal_time_t current_time_stamp;
    bool isFull = false;
    uint32_t send_buffer = MSG_CONTENT;
    uint64_t i;

    /* get the config of MQ */
    osal_ret = R_OSAL_MqGetConfig(*p_mq_handle, &mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("mq get config failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        /* send 2 messages to MQ */
        for(i = 0; i < mq_config.max_num_msg; i++)
        {
            /* check message queue is full or not */
            osal_ret = R_OSAL_MqIsFull(*p_mq_handle, &isFull);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("message queue full checking failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                /* send message to queue with time-out (time period format) */
                if(true != isFull)
                {
                    if(0 == i)
                    {
                        OSAL_SAMPLE_INFO("send the first message which content is 1234 to queue\n")
                        osal_ret = R_OSAL_MqSendForTimePeriod(*p_mq_handle, TIMEOUT_MS, &send_buffer, mq_config.msg_size);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("sending message to MQ was failed, osal_ret = %d\n", osal_ret);
                        }
                        else
                        {
                            OSAL_SAMPLE_INFO("first message was sent successfully\n");
                        }
                    }
                    else /* 1 == i */
                    {
                        osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
                        }
                        else
                        {
                            current_time_stamp.tv_sec += WAIT_TIME_SEC;
                            current_time_stamp.tv_nsec += WAIT_TIME_NS;
                            if(MAX_NS <= current_time_stamp.tv_nsec)
                            {
                                current_time_stamp.tv_sec += 1;
                                current_time_stamp.tv_nsec %= MAX_NS;
                            }

                            send_buffer += 1;
                            OSAL_SAMPLE_INFO("send the second message which content is 1235 to queue\n")
                            osal_ret = R_OSAL_MqSendUntilTimeStamp(*p_mq_handle, &current_time_stamp, &send_buffer, mq_config.msg_size);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("sending message to MQ was failed, osal_ret = %d\n", osal_ret);
                            }
                            else
                            {
                                OSAL_SAMPLE_INFO("second message was sent successfully\n");
                            }
                        }
                    }
                }
                else
                {
                    OSAL_SAMPLE_ERR("MQ has already been full \n");
                }
            }
        }
    }

    return app_ret;
}

/**
 * @brief Receiving thread:
 * - Get the configuration of created message queue
 * - Receive two messages from the queue:
 *      -# Receive fist message by using R_OSAL_MqReceiveForTimePeriod.<br>
 *        The content of receive buffer need to be equal to "MSG_CONTENT".
 *      -# Receive second message by using R_OSAL_MqReceiveUntilTimeStamp.<br>
 *        The content of receive buffer need to be equal to "MSG_CONTENT".
 * 
 * @param[in] arg a void pointer which is the address of message queue handle
 * @return 0 on success
 * @return -1 on failure
 */
/* receiving thread */
int64_t mq_second_worker_func(void *arg)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    osal_mq_handle_t *p_mq_handle = (osal_mq_handle_t *)arg;
    st_osal_mq_config_t mq_config;
    st_osal_time_t current_time_stamp;
    uint32_t receive_buffer = 0;
    uint64_t i;

    /* get the config of MQ */
    osal_ret = R_OSAL_MqGetConfig(*p_mq_handle, &mq_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("mq get config failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        for(i = 0; i < mq_config.max_num_msg; i++)
        {
            if(0 == i)
            {
                OSAL_SAMPLE_INFO("receive first message from queue. The content of received buffer: 1234\n")
                osal_ret = R_OSAL_MqReceiveForTimePeriod(*p_mq_handle, (osal_milli_sec_t)TIMEOUT_MS, (void *)&receive_buffer, mq_config.msg_size);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("receiving message from MQ was failed, osal_ret = %d\n", osal_ret);
                }
                else if((int)MSG_CONTENT != receive_buffer)
                {
                    OSAL_SAMPLE_ERR("received message (%ld) was not correct with sent message (%d)\n", receive_buffer, (int)MSG_CONTENT);
                }
                else
                {
                    /* received message is correct with sent message */
                    OSAL_SAMPLE_INFO("first message received successfully\n");
                }
            }
            else /* 1 == i */
            {
                osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    current_time_stamp.tv_sec += WAIT_TIME_SEC;
                    current_time_stamp.tv_nsec += WAIT_TIME_NS;
                    if(MAX_NS <= current_time_stamp.tv_nsec)
                    {
                        current_time_stamp.tv_sec += 1;
                        current_time_stamp.tv_nsec %= MAX_NS;
                    }

                    OSAL_SAMPLE_INFO("receive second message from queue. The content of received buffer: 1235\n")
                    osal_ret = R_OSAL_MqReceiveUntilTimeStamp(*p_mq_handle, &current_time_stamp, (void *)&receive_buffer, mq_config.msg_size);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("receiving message from MQ was failed, osal_ret = %d\n", osal_ret);
                    }
                    else if((int)(MSG_CONTENT + 1) != receive_buffer)
                    {
                        OSAL_SAMPLE_ERR("received message (%ld) was not correct with sent message (%d)\n", receive_buffer, (int)(MSG_CONTENT + 1));
                    }
                    else
                    {
                        /* received message is correct with sent message */
                        OSAL_SAMPLE_INFO("second message received successfully\n");
                    }
                }
            }
        }
    }

    return app_ret;
}
/**@} mq*/

/**
 * @defgroup thread_main_task OSAL Thread APIs Usage
 * @{
 */
/**
 @brief Thread main task:<br>
 * (OSAL API UM reference: Chapter 2.2.1: Create & Join function)
 * - Initalize two thread configuration objects by using R_OSAL_ThreadInitializeThreadConfigSt
 * - Create two thread with 2 thread configuration objects above:
 *      -# The first thread is @link thread_first_sub_task @endlink
 *      -# The second thread is @link thread_second_sub_task @endlink
 * - Wait for 2 threads above finish by using R_OSAL_ThreadJoin
 * 
 * @param[in] arg a void pointer
 * @return 0 on success
 * @return -1 on failure
 */
int64_t thread_main_task(void *arg)
{
    (void)arg;
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    st_osal_thread_config_t thread1_config, thread2_config;
    osal_thread_handle_t thread1_handle = OSAL_THREAD_HANDLE_INVALID, thread2_handle = OSAL_THREAD_HANDLE_INVALID;
    int64_t thread1_ret = 0, thread2_ret = 0;

    printf("\n================== thread_main_task start ==================\n");
    osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread1_config);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread2_config);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
        }
    }

    if(0 == app_ret)
    {
        thread1_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
        thread1_config.task_name = "sub-thread-no1";
        thread1_config.func = thread_first_sub_task;
        thread1_config.userarg = NULL;
        thread1_config.stack_size = 0x2000;

        OSAL_SAMPLE_INFO("create first child thread \n");
        osal_ret = R_OSAL_ThreadCreate(&thread1_config, (osal_thread_id_t)SUB_THREAD_ID_NO1, &thread1_handle);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("thread_first_sub_task creation was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            thread2_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread2_config.task_name = "sub-thread-no2";
            thread2_config.func = thread_second_sub_task;
            thread2_config.userarg = (void *)&thread1_handle;
            thread2_config.stack_size = 0x2000;

            OSAL_SAMPLE_INFO("create second child thread \n");
            osal_ret = R_OSAL_ThreadCreate(&thread2_config, (osal_thread_id_t)SUB_THREAD_ID_NO2, &thread2_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR ("worker function creation was failed, osal_ret = %d\n", osal_ret);
            }

            /* wait for thread_second_sub_task finish */
            osal_ret = R_OSAL_ThreadJoin(thread2_handle, &thread2_ret);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("join thread_second_sub_task failed, osal_ret = %d\n", osal_ret);
            }
            else if(0 != thread2_ret)
            {
                OSAL_SAMPLE_ERR("thread_second_sub_task operation was failed, thread_ret = %ld\n", (long)thread2_ret);
            }
            else
            {
                OSAL_SAMPLE_INFO("second child thread operation is completed \n");
            }

            if(0 == app_ret)
            {
                /* wait for thread_first_sub_task finish */
                osal_ret = R_OSAL_ThreadJoin(thread1_handle, &thread1_ret);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("join thread_first_sub_task failed, osal_ret = %d\n", osal_ret);
                }
                else if(0 != thread1_ret)
                {
                    OSAL_SAMPLE_ERR("thread_first_sub_task operation was failed, thread_ret = %ld\n", (long)thread1_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("first child thread operation is completed \n");
                }
            }
        }
    }

    printf("================== thread_main_task finish ==================\n\n");
    return app_ret;
}

/**
 * @brief First sub-thread:
 * - This thread does nothing.<br>
 * Its handle will be used to compare with the thread handle of the second thread.
 * 
 * @param[in] arg a void pointer
 * @return 0 on success.<br>
 * (This function only return on success)
 */
int64_t thread_first_sub_task(void *arg)
{
    (void) arg;

    return 0;
}

/**
 * @brief Second sub-thread:
 * - Suspend thread by 2500ms:
 *      -# First 1000ms, using R_OSAL_ThreadSleepForTimePeriod
 *      -# Next 1500ms, using R_OSAL_ThreadSleepUntilTimeStamp
 * - Compare the itself thread handle with the thread handle of @link thread_first_sub_task @endlink with steps as below:
 *      -# Get the itself thread handle by using R_OSAL_ThreadSelf
 *      -# Compare with thread handle of @link thread_first_sub_task @endlink by using R_OSAL_ThreadEqual
 *      -# The expected value of "isEqual" is "false"
 * 
 * @param[in] arg a void pointer which is the address of @link thread_first_sub_task @endlink thread handle
 * @return 0 on success.
 * @return -1 on failure
 */
int64_t thread_second_sub_task(void *arg)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    osal_thread_handle_t self_thread_handle = OSAL_THREAD_HANDLE_INVALID;
    osal_thread_handle_t *first_thread_handle = (osal_thread_handle_t *)arg;
    st_osal_time_t current_time_stamp;
    st_osal_time_t past_time_stamp;
    osal_nano_sec_t time_stamp;
    bool isEqual = false;

    /* suspend thread 2500ms */
    OSAL_SAMPLE_INFO("suspend second child thread by 2500ms \n");
    osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &past_time_stamp);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
    }

    OSAL_SAMPLE_INFO("First suspend time is 1000ms \n");
    osal_ret = R_OSAL_ThreadSleepForTimePeriod((osal_milli_sec_t)TIMEOUT_MS);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("thread suspending was failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("Second suspend time is 1500ms \n");
            current_time_stamp.tv_sec += WAIT_TIME_SEC;
            current_time_stamp.tv_nsec += WAIT_TIME_NS;
            if(MAX_NS <= current_time_stamp.tv_nsec)
            {
                current_time_stamp.tv_sec += 1;
                current_time_stamp.tv_nsec %= MAX_NS;
            }
            osal_ret = R_OSAL_ThreadSleepUntilTimeStamp(&current_time_stamp);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("thread suspending was failed, osal_ret = %d\n", osal_ret);
            }
        }
    }

    osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
    }

    osal_ret = R_OSAL_ClockTimeCalculateTimeDifference(&current_time_stamp, &past_time_stamp, &time_stamp);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("Time difference calculation was failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        OSAL_SAMPLE_INFO("Suspend time is %ldnsec\n", (long)time_stamp);
    }

    if(0 == app_ret)
    {
        OSAL_SAMPLE_INFO("Get the thread handle of second child thread \n");
        osal_ret = R_OSAL_ThreadSelf(&self_thread_handle);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("getting self-handle was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("Compare the thread handle of second child thread with first child thread\n");
            osal_ret = R_OSAL_ThreadEqual(self_thread_handle, *first_thread_handle, &isEqual);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("R_OSAL_ThreadEqual operation failed, osal_ret = %d\n", osal_ret);
            }
            else if(true == isEqual)
            {
                OSAL_SAMPLE_ERR("comparing thread handle was failed\n");
            }
            else
            {
                /* Do nothing */
            }
        }
    }

    return app_ret;
}
/**@} thread*/

/**
 * @defgroup threadsync_main_task OSAL Thread Synchronization APIs Usage
 * @{
 */
/**
 * @brief Thread synchronization main task:<br>
 * (OSAL API UM reference: Chapter 2.3.1: Mutex function and Chapter 2.3.2: Condition variable function)
 * - Create a mutex by using R_OSAL_ThsyncMutexCreate
 * - Create a condition variable by using R_OSAL_ThsyncCondCreate
 * - Try to lock the created mutex by using R_OSAL_ThsyncMutexTryLock
 * - Unlock after locking successfully by using R_OSAL_ThsyncMutexUnlock
 * - Initalize two thread configuration objects by using R_OSAL_ThreadInitializeThreadConfigSt
 * - Create two thread with 2 thread configuration objects above:
 *      -# The first thread is @link threadsync_first_worker_func @endlink which play a role in condition waiting thread
 *      -# The second thread is @link threadsync_second_worker_func @endlink which play a role in condition waking up thread
 * - Wait for 2 threads above finish by using R_OSAL_ThreadJoin
 * 
 * @param[in] arg a void pointer
 * @return 0 on success
 * @return -1 on failure
 */
int64_t threadsync_main_task(void *arg)
{
    (void)arg;
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    osal_mutex_handle_t mtx_handle = OSAL_MUTEX_HANDLE_INVALID;
    osal_cond_handle_t cond_handle = OSAL_COND_HANDLE_INVALID;
    st_osal_thread_config_t thread1_config, thread2_config;
    osal_thread_handle_t thread1_handle = OSAL_THREAD_HANDLE_INVALID, thread2_handle = OSAL_THREAD_HANDLE_INVALID;
    int64_t thread1_ret = 0, thread2_ret = 0;
    st_arg_cond_mutex_t pass_arg;

    printf("\n================== threadsync_main_task start ==================\n");
    OSAL_SAMPLE_INFO("create a mutex\n");
    osal_ret = R_OSAL_ThsyncMutexCreate((osal_mutex_id_t)MUTEX_ID_NO1, &mtx_handle);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("mutex creation was failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        OSAL_SAMPLE_INFO("create a condition variable\n");
        osal_ret = R_OSAL_ThsyncCondCreate((osal_cond_id_t)COND_ID_NO1, &cond_handle);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("condition creation was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            pass_arg.cond_handle = cond_handle;
            pass_arg.mtx_handle = mtx_handle;
        }
    }

    if(0 == app_ret)
    {
        OSAL_SAMPLE_INFO("lock the created mutex with R_OSAL_ThsyncMutexTryLock\n");
        osal_ret = R_OSAL_ThsyncMutexTryLock(mtx_handle);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("mutex lock failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("unlock the mutex\n");
            osal_ret = R_OSAL_ThsyncMutexUnlock(mtx_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("mutex unlock failed, osal_ret = %d\n", osal_ret);
            }
        }
    }

    /* create 2 worker functions which use the shared data */
    if(0 == app_ret)
    {
        osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread1_config);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            osal_ret = R_OSAL_ThreadInitializeThreadConfigSt(&thread2_config);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("thread config initialization was failed, osal_ret = %d\n", osal_ret);
            }
        }

        if (0 == app_ret)
        {
            thread1_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread1_config.task_name = "waiting thread";
            thread1_config.func = threadsync_first_worker_func;
            thread1_config.userarg = (void *)&pass_arg;
            thread1_config.stack_size = 0x2000;

            thread2_config.priority = OSAL_THREAD_PRIORITY_TYPE10;
            thread2_config.task_name = "wakeup thread";
            thread2_config.func = threadsync_second_worker_func;
            thread2_config.userarg = (void *)&pass_arg;
            thread2_config.stack_size = 0x2000;

            OSAL_SAMPLE_INFO("create condition waiting thread\n");
            osal_ret = R_OSAL_ThreadCreate(&thread1_config, (osal_thread_id_t)SUB_THREAD_ID_NO1, &thread1_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("worker function creation was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                OSAL_SAMPLE_INFO("create condition waking up thread\n");
                osal_ret = R_OSAL_ThreadCreate(&thread2_config, (osal_thread_id_t)SUB_THREAD_ID_NO2, &thread2_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR ("worker function creation was failed, osal_ret = %d\n", osal_ret);
                }

                osal_ret = R_OSAL_ThreadJoin(thread1_handle, &thread1_ret);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("join threadsync_first_worker_func failed, osal_ret = %d\n", osal_ret);
                }
                else if(0 != thread1_ret)
                {
                    OSAL_SAMPLE_ERR("threadsync_first_worker_func operation was failed, thread_ret = %ld\n", (long)thread1_ret);
                }
                else
                {
                   OSAL_SAMPLE_INFO("condition waiting thread operation is completed\n");
                }

                if(0 == app_ret)
                {
                    osal_ret = R_OSAL_ThreadJoin(thread2_handle, &thread2_ret);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("join threadsync_second_worker_func failed, osal_ret = %d\n", osal_ret);
                    }
                    else if(0 != thread2_ret)
                    {
                        OSAL_SAMPLE_ERR("threadsync_second_worker_func operation was failed, thread_ret = %ld\n", (long)thread2_ret);
                    }
                    else
                    {
                        OSAL_SAMPLE_INFO("condition waking up thread operation is completed\n");
                    }
                    osal_ret = R_OSAL_ThsyncCondDestroy(cond_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("R_OSAL_ThsyncCondDestroy failed, osal_ret = %d\n", osal_ret);
                    }
                    osal_ret = R_OSAL_ThsyncMutexDestroy(mtx_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("R_OSAL_ThsyncCondDestroy failed, osal_ret = %d\n", osal_ret);
                    }

                }
            }
        }
    }

    printf("================== threadsync_main_task finish ==================\n\n");
    return app_ret;
}

/**
 * @brief Condition waiting thread:
 * - Waiting on a condition variable in 2 times:
 *      -# The first waiting time with the steps as below:
 *          - Lock mutex by using R_OSAL_ThsyncMutexLockForTimePeriod
 *          - Waiting on the condition by using R_OSAL_ThsyncCondWaitForTimePeriod
 *          - Unlock mutex by using R_OSAL_ThsyncMutexUnlock
 *      -# The second waiting time with the steps as below:
 *          - Lock mutex by using R_OSAL_ThsyncMutexLockUntilTimeStamp
 *          - Waiting on the condition by using R_OSAL_ThsyncCondWaitUntilTimeStamp
 *          - Unlock mutex by using R_OSAL_ThsyncMutexUnlock
 * 
 * @paramp[in] arg a void pointer which is the address of st_arg_cond_mutex_t
 * @return 0 on success
 * @return -1 on failure
 */
int64_t threadsync_first_worker_func(void *arg)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    st_arg_cond_mutex_t *p_local_arg = (st_arg_cond_mutex_t *)arg;
    st_osal_time_t current_time_stamp;
    int i;

    for(i = 0; ((i < 2) && (app_ret == 0)); i++)
    {
        if(0 == i)
        {
            OSAL_SAMPLE_INFO("waiting thread, lock mutex\n");
            osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(p_local_arg->mtx_handle, (osal_milli_sec_t)TIMEOUT_MS);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("mutex lock failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                while(0 == shared_data)
                {
                    OSAL_SAMPLE_INFO("waiting thread, 1st waiting with condition variable\n");
                    osal_ret = R_OSAL_ThsyncCondWaitForTimePeriod(p_local_arg->cond_handle, p_local_arg->mtx_handle, (osal_milli_sec_t)TIMEOUT_MS);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("condition wait failed, osal_ret = %d\n", osal_ret);
                        break;
                    }
                }

                OSAL_SAMPLE_INFO("waiting thread, unlock mutex\n");
                osal_ret = R_OSAL_ThsyncMutexUnlock(p_local_arg->mtx_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("mutex unlock failed, osal_ret = %d\n", osal_ret);
                }
            }
        }
        else /* 1 == i */
        {
            osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                current_time_stamp.tv_sec += WAIT_TIME_SEC;
                current_time_stamp.tv_nsec += WAIT_TIME_NS;
                if(MAX_NS <= current_time_stamp.tv_nsec)
                {
                    current_time_stamp.tv_sec += 1;
                    current_time_stamp.tv_nsec %= MAX_NS;
                }
                OSAL_SAMPLE_INFO("waiting thread, lock mutex\n");
                osal_ret = R_OSAL_ThsyncMutexLockUntilTimeStamp(p_local_arg->mtx_handle, &current_time_stamp);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("mutex lock failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    while((0 != shared_data) && (0 == app_ret))
                    {
                        osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
                            break;
                        }
                        else
                        {
                            current_time_stamp.tv_sec += WAIT_TIME_SEC;
                            current_time_stamp.tv_nsec += WAIT_TIME_NS;
                            if(MAX_NS <= current_time_stamp.tv_nsec)
                            {
                                current_time_stamp.tv_sec += 1;
                                current_time_stamp.tv_nsec %= MAX_NS;
                            }
                            OSAL_SAMPLE_INFO("waiting thread, 2nd waiting with condition variable\n");
                            osal_ret = R_OSAL_ThsyncCondWaitUntilTimeStamp(p_local_arg->cond_handle, p_local_arg->mtx_handle, &current_time_stamp);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("condition wait failed, osal_ret = %d\n", osal_ret);
                                break;
                            }
                        }
                    }

                    OSAL_SAMPLE_INFO("waiting thread, unlock mutex\n");
                    osal_ret = R_OSAL_ThsyncMutexUnlock(p_local_arg->mtx_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("mutex unlock failed, osal_ret = %d\n", osal_ret);
                    }
                }
            }
        }
    }

    return app_ret;
}

/**
 * @brief Condition waking up thread:
 * - waking up the condition variable in 2 times:
 *      -# The first waking up time with the steps as below:
 *          - Lock mutex by using R_OSAL_ThsyncMutexLockForTimePeriod
 *          - waking up the condition by using R_OSAL_ThsyncCondSignal
 *          - Unlock mutex by using R_OSAL_ThsyncMutexUnlock
 *          - Sleep in 10ms in order to make a change for the @link threadsync_first_worker_func @endlink run first in the second "for" loop.
 *      -# The second waking time with the steps as below:
 *          - Lock mutex by using R_OSAL_ThsyncMutexLockUntilTimeStamp
 *          - Waiting on the condition by using R_OSAL_ThsyncCondBroadcast
 *          - Unlock mutex by using R_OSAL_ThsyncMutexUnlock
 * 
 * @param[in] arg a void pointer which is the address of st_arg_cond_mutex_t
 * @return 0 on success
 * @return -1 on failure
 */
int64_t threadsync_second_worker_func(void *arg)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t app_ret = 0;
    st_arg_cond_mutex_t *p_local_arg = (st_arg_cond_mutex_t *)arg;
    st_osal_time_t current_time_stamp;
    int i;

    for(i = 0; ((i < 2) && (app_ret == 0)); i++)
    {
        if(0 == i)
        {
            OSAL_SAMPLE_INFO("waking up thread, lock mutex\n");
            osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(p_local_arg->mtx_handle, (osal_milli_sec_t)TIMEOUT_MS);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("mutex lock failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                shared_data += 1;
                OSAL_SAMPLE_INFO("waking up thread, 1st waking up the waiting thread\n");
                osal_ret = R_OSAL_ThsyncCondSignal(p_local_arg->cond_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("condition signal failed, osal_ret = %d\n", osal_ret);
                }

                OSAL_SAMPLE_INFO("waking up thread, unlock mutex\n");
                osal_ret = R_OSAL_ThsyncMutexUnlock(p_local_arg->mtx_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("mutex unlock failed, osal_ret = %d\n", osal_ret);
                }
            }

            if(0 == app_ret)
            {
                /* make a change for threadsync_first_worker_func start before threadsync_second_worker_func in the second loop */
                osal_ret = R_OSAL_ThreadSleepForTimePeriod((osal_milli_sec_t)(TIMEOUT_MS/100));
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("thread suspending failed, osal_ret = %d\n", osal_ret);
                }
            }
        }
        else /* 1 == i */
        {
            osal_ret = R_OSAL_ClockTimeGetTimeStamp(OSAL_CLOCK_TYPE_HIGH_RESOLUTION, &current_time_stamp);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("getting the current timestamp was failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                current_time_stamp.tv_sec += WAIT_TIME_SEC;
                current_time_stamp.tv_nsec += WAIT_TIME_NS;
                if(MAX_NS <= current_time_stamp.tv_nsec)
                {
                    current_time_stamp.tv_sec += 1;
                    current_time_stamp.tv_nsec %= MAX_NS;
                }

                OSAL_SAMPLE_INFO("waking up thread, lock mutex\n");
                osal_ret = R_OSAL_ThsyncMutexLockUntilTimeStamp(p_local_arg->mtx_handle, &current_time_stamp);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("mutex lock failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    shared_data -= 1;
                    OSAL_SAMPLE_INFO("waking up thread, 2nd waking up the waiting thread\n");
                    osal_ret = R_OSAL_ThsyncCondBroadcast(p_local_arg->cond_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("condition broadcast failed, osal_ret = %d\n", osal_ret);
                    }

                    OSAL_SAMPLE_INFO("waking up thread, unlock mutex\n");
                    osal_ret = R_OSAL_ThsyncMutexUnlock(p_local_arg->mtx_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("mutex unlock failed, osal_ret = %d\n", osal_ret);
                    }
                }
            }
        }
    }

    return app_ret;
}
/**@} threadsync*/
/**@} sample resource*/
