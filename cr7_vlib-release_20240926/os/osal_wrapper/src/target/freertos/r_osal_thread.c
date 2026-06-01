/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_thread.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper Thread Manager for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"

/*******************************************************************************************************************//**
 * @var gs_osal_thread_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_thread_is_initialized = false;

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Functions_Thread Thread Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Internal Thread emtry function.
* @param[in]        arg  argument for thread function.
* @retval           none
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_thread(void *arg);

/*******************************************************************************************************************//**
* @brief            Search the thread control corresponding to the ID.
* @param[in]        id The value of thread id.
* @param[in, out]   thread_control valid thread control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_thread_find_control(osal_thread_id_t id, st_osal_thread_control_t **thread_control);

/*******************************************************************************************************************//**
* @brief            Set thread control.
* @param[in]        p_config The address of st_osal_thread_config_t.
* @param[in, out]   thread_control Setting target control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_thread_set_control(st_osal_thread_config_t const *p_config,
                                                    st_osal_thread_control_t *thread_control);

/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadCreate()
* XOS1_OSAL_CD_CD_100
* [Covers: XOS1_OSAL_UD_UD_075]
* [Covers: XOS1_OSAL_UD_UD_158]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_076]
* [Covers: XOS1_OSAL_UD_UD_209]
* [Covers: XOS1_OSAL_UD_UD_055]
* [Covers: XOS1_OSAL_UD_UD_240]
* [Covers: XOS1_OSAL_UD_UD_210]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadCreate(const st_osal_thread_config_t *const p_config, osal_thread_id_t thread_id,
                                    osal_thread_handle_t *const p_handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_thread_control_t *thread_control = NULL;
    BaseType_t freertos_ret;
    TickType_t inner_time;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%lu, %p)\n", thread_id, (void *)p_handle);

    if (false == gs_osal_thread_is_initialized)
    {
        OSAL_DEBUG("Thread Open: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if ((NULL == p_config) || (NULL == p_handle))
    {
        OSAL_DEBUG("Thread Open: OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = osal_thread_find_control(thread_id, &thread_control);
    }

    if ((NULL != thread_control) && (true == thread_control->use))
    {
        OSAL_DEBUG("Thread Open: OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (NULL == thread_control)
        {
            OSAL_DEBUG("Thread Open: OSAL_RETURN_ID\n");
            osal_ret = OSAL_RETURN_ID;
        }
        else if (NULL == thread_control->inner_mtx)
        {
            OSAL_DEBUG("OSAL_RETURN_STATE\n");
            osal_ret = OSAL_RETURN_STATE;
        }
        else
        {
            ;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        R_OSAL_Internal_MillisecToTick(thread_control->inner_timeout, &inner_time);
        freertos_ret = xSemaphoreTake(thread_control->inner_mtx, inner_time);
        if(pdPASS == freertos_ret)
        {
            osal_ret = osal_thread_set_control(p_config, thread_control);
            if (OSAL_RETURN_OK == osal_ret)
            {
                thread_control->joined = false;
                *p_handle = thread_control;
            }
            freertos_ret = xSemaphoreGive(thread_control->inner_mtx);
            if(pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("Thread Open: OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
        else
        {
            OSAL_DEBUG("Thread Open: OSAL_RETURN_BUSY\n");
            osal_ret = OSAL_RETURN_BUSY;
        }
    }

    if ((OSAL_RETURN_OK != osal_ret) && (OSAL_RETURN_BUSY != osal_ret))
    {
        if (NULL != p_handle)
        {
            *p_handle = OSAL_THREAD_HANDLE_INVALID;
        }
    }

    OSAL_DEBUG("End (%u, %p)\n", osal_ret, (void *)*p_handle);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadCreate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadSleepUntilTimeStamp()
* XOS1_OSAL_CD_CD_106
* [Covers: XOS1_OSAL_UD_UD_159]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_080]
* [Covers: XOS1_OSAL_UD_UD_309]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadSleepUntilTimeStamp(const st_osal_time_t * const p_time_stamp)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_time_t current_time_stamp;
    osal_nano_sec_t difference_nano = 0;
    TickType_t set_tick = 0;
    osal_milli_sec_t difference_milli;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (false == gs_osal_thread_is_initialized)
    {
        OSAL_DEBUG("Thread Sleep: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_time_stamp)
    {
        OSAL_DEBUG("Thread Sleep TimeStamp: OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = R_OSAL_Internal_CheckTimestamp(p_time_stamp);
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = R_OSAL_Internal_GetTimestamp(&current_time_stamp);
        if(OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = R_OSAL_Internal_GetDiffTime(p_time_stamp, &current_time_stamp, &difference_nano);
            if(OSAL_RETURN_OK == osal_ret)
            {
                /* round up to millisecond */
                difference_milli = (difference_nano + OSAL_MSECTONSEC - 1) / OSAL_MSECTONSEC;
                R_OSAL_Internal_MillisecToTick(difference_milli, &set_tick);

                vTaskDelay(set_tick);
            }
            else
            {
                OSAL_DEBUG("Thread Sleep TimeStamp: OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
        else
        {
            OSAL_DEBUG("Thread Sleep TimeStamp: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadSleepUntilTimeStamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadSleepForTimePeriod()
* XOS1_OSAL_CD_CD_110
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_160]
* [Covers: XOS1_OSAL_UD_UD_082]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadSleepForTimePeriod(osal_milli_sec_t time_period)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    TickType_t sleep_tick = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (false == gs_osal_thread_is_initialized)
    {
        OSAL_DEBUG("Thread SleepPeriod: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (0 > time_period)
    {
        OSAL_DEBUG("Thread SleepPeriod: OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        R_OSAL_Internal_MillisecToTick(time_period, &sleep_tick);
        vTaskDelay(sleep_tick);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadSleepForTimePeriod()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadSelf()
* XOS1_OSAL_CD_CD_112
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_161]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadSelf(osal_thread_handle_t* const p_handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    TaskHandle_t current_handle;
    uint32_t count = 0;
    size_t num_of_info;
    bool find_flag = false;
    st_osal_thread_control_t *thread_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");
    if (false == gs_osal_thread_is_initialized)
    {
        OSAL_DEBUG("Thread Self: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_handle)
    {
        OSAL_DEBUG("Thread Self: OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        current_handle = xTaskGetCurrentTaskHandle();
        num_of_info = R_OSAL_RCFG_GetNumOfThread();
    }

    while ((osal_ret == OSAL_RETURN_OK) && (count < num_of_info) && (false == find_flag))
    {
        api_ret = R_OSAL_RCFG_GetThreadInfo(count, &thread_control);

        if ((OSAL_RETURN_OK != api_ret) || (NULL == thread_control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (NULL == thread_control->thread)
            {
                OSAL_DEBUG("OSAL_RETURN_STATE\n");
                osal_ret = OSAL_RETURN_STATE;
            }
            else if (current_handle == thread_control->thread)
            {
                OSAL_DEBUG("Thread Self: find thread\n");
                find_flag = true;
                *p_handle = thread_control;
            }
            else /* continue processing */
            {
                ;
            }
        }
        count++;
    }

    if ((OSAL_RETURN_OK == osal_ret) && (false == find_flag))
    {
        OSAL_DEBUG("Thread Self: OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }

    OSAL_DEBUG("End (%u, %p)\n", osal_ret, (void*)*p_handle);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadSelf()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadEqual()
* XOS1_OSAL_CD_CD_113
* [Covers: XOS1_OSAL_UD_UD_238]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_083]
* [Covers: XOS1_OSAL_UD_UD_084]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadEqual(osal_thread_handle_t handle1, osal_thread_handle_t handle2, bool *const p_result)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p, %p)\n", (void *)handle1, (void *)handle2);
    if ( false == gs_osal_thread_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if ((NULL == handle1) || (NULL == handle2))
    {
        OSAL_DEBUG("Thread Equal: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_THREAD_HANDLE_ID != handle1->handle_id) || (OSAL_THREAD_HANDLE_ID != handle2->handle_id))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if(NULL == p_result)
    {
        OSAL_DEBUG("Thread Equal: OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        if (handle1 != handle2)
        {
            *p_result = false;
        }
        else
        {
            *p_result = true;
        }
    }

    OSAL_DEBUG("End (%u, %d)\n", osal_ret, (int32_t)*p_result);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadEqual()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadJoin()
* XOS1_OSAL_CD_CD_116
* [Covers: XOS1_OSAL_UD_UD_162]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_239]
* [Covers: XOS1_OSAL_UD_UD_086]
* [Covers: XOS1_OSAL_UD_UD_085]
* [Covers: XOS1_OSAL_UD_UD_056]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadJoin(osal_thread_handle_t handle, int64_t *const p_return_value)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p)\n", (void *)handle);

    if (false == gs_osal_thread_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (OSAL_THREAD_HANDLE_ID != handle->handle_id)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (true == handle->joined)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == handle->parent_thread)
    {
        handle->parent_thread = xTaskGetCurrentTaskHandle();
        OSAL_DEBUG("[parent_thread]%p\n", (void*)handle->parent_thread);

        if (handle->parent_thread == handle->thread)
        {
            OSAL_DEBUG("Can not join itself.\n");
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            if(true == handle->active)
            {
                OSAL_DEBUG("JOIN wait start\n");
                (void) xTaskNotifyWait(0U, 0U, NULL, portMAX_DELAY);
                /* This API always returns pdTRUE because using portMAX_DELAY. */

                OSAL_DEBUG("JOIN return\n");

                if (NULL != p_return_value)
                {
                    *p_return_value = handle->thread_ret;
                }
            }
            else
            {
                OSAL_DEBUG("thread had already finished\n");
                if (NULL != p_return_value)
                {
                    *p_return_value = handle->thread_ret;
                }
            }
            handle->use    = false;
            handle->joined = true;
        }

        handle->parent_thread = NULL;
    }
    else
    {
        OSAL_DEBUG("Another thread has already join waited.\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }

    OSAL_DEBUG("End (%u, %ld)\n", osal_ret, *p_return_value);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadJoin()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_thread_initialize()
* XOS1_OSAL_CD_CD_120
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_013]
* [Covers: XOS1_OSAL_UD_UD_022]
* [Covers: XOS1_OSAL_UD_UD_142]
* [Covers: XOS1_OSAL_UD_UD_143]
* [Covers: XOS1_OSAL_UD_UD_144]
* [Covers: XOS1_OSAL_UD_UD_320]
***********************************************************************************************************************/
e_osal_return_t osal_thread_initialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info = 0;
    uint32_t count = 0;
    st_osal_thread_control_t *control = NULL;
    BaseType_t freertos_ret;
    SemaphoreHandle_t mtx;
    configSTACK_DEPTH_TYPE stack_size;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Thread Init start\n");

    if (true == gs_osal_thread_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_thread_is_initialized = true;
        if (((configSTACK_DEPTH_TYPE) - 1) > 0)
        {
            stack_size = (configSTACK_DEPTH_TYPE)-1;
        }
        else
        {
            stack_size = ((configSTACK_DEPTH_TYPE)-1)&(configSTACK_DEPTH_TYPE)(~((configSTACK_DEPTH_TYPE)1U << (sizeof(configSTACK_DEPTH_TYPE) * 8 - 1)));
        }

        num_of_info = R_OSAL_RCFG_GetNumOfThread();
        OSAL_DEBUG("Get num_of_info: %zu\n", num_of_info);
    }

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetThreadInfo(count, &control);
        OSAL_DEBUG("Get thread_control: %p\n", (void *)control);

        if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->usr_config))
        {
            OSAL_DEBUG("Thread Init: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {

            /* check Thread configuration */
            if ((OSAL_THREAD_PRIORITY_HIGHEST < control->usr_config->config.priority) ||
                (OSAL_THREAD_PRIORITY_LOWEST > control->usr_config->config.priority))
            {
                OSAL_DEBUG("Thread Init: OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else if ((0U >= OSAL_CONV_BYTE_TO_WORD(control->usr_config->config.stack_size)) ||
                     (stack_size < OSAL_CONV_BYTE_TO_WORD(control->usr_config->config.stack_size)))
            {
                OSAL_DEBUG("Thread Init: OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }

            else /* Configuration check PASS */
            {
                freertos_ret = xTaskCreate(
                            osal_internal_thread,
                            control->usr_config->config.task_name,
                            (configSTACK_DEPTH_TYPE)OSAL_CONV_BYTE_TO_WORD(control->usr_config->config.stack_size),
                            (void *)control,
                            (UBaseType_t)OSAL_CONV_PRIORITY((uint16_t)(control->usr_config->config.priority)),
                            &control->thread);
                if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == freertos_ret)
                {
                    OSAL_DEBUG("Thread Init: OSAL_RETURN_MEM\n");
                    osal_ret = OSAL_RETURN_MEM;
                }
                else
                {
                    mtx = xSemaphoreCreateBinary();
                    if (NULL == mtx)
                    {
                        OSAL_DEBUG("Thread Init: OSAL_RETURN_MEM\n");
                        osal_ret = OSAL_RETURN_MEM;
                    }
                    else
                    {
                        /* control initialize*/
                        control->handle_id = OSAL_THREAD_HANDLE_ID;
                        control->inner_mtx = mtx;
                        (void)xSemaphoreGive(mtx);
                        control->use    = false;
                        control->active = false;
                        control->joined = false;
                        control->thread_ret = 0;
                        control->config.func = control->usr_config->config.func;
                        control->config.userarg = control->usr_config->config.userarg;
                        control->config.task_name = NULL;
                        control->config.priority = OSAL_THREAD_PRIORITY_TYPE0;
                        control->config.stack_size = 0;
                    }
                }
            }
        }

        count++;
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_thread_deinitialize();
    }

    OSAL_DEBUG("Thread Init end (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_thread_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_thread_deinitialize()
* XOS1_OSAL_CD_CD_126
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_003]
* [Covers: XOS1_OSAL_UD_UD_327]
***********************************************************************************************************************/
e_osal_return_t osal_thread_deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_thread_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_thread_is_initialized = false;

    num_of_info = R_OSAL_RCFG_GetNumOfThread();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetThreadInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (NULL != control->thread)
            {
                vTaskDelete(control->thread);
                control->thread = NULL;
            }

            if (NULL != control->inner_mtx)
            {
                vSemaphoreDelete(control->inner_mtx);
                control->inner_mtx = NULL;
            }

            control->handle_id = 0;
            control->use    = false;
            control->active = false;
            control->joined = false;
            control->thread_ret = 0;
        }
        count++;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_thread_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_thread_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_thread_deinitialize_pre(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_thread_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_thread_is_initialized = false;

    num_of_info = R_OSAL_RCFG_GetNumOfThread();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetThreadInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (true == control->use)
            {
                osal_ret = OSAL_RETURN_BUSY;
            }
        }
        count++;
    }

    if (OSAL_RETURN_BUSY == osal_ret)
    {
        gs_osal_thread_is_initialized = true;
    }
    else
    {
        /* Do nothing */
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_thread_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_thread_set_is_init()

***********************************************************************************************************************/
void osal_thread_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_thread_is_initialized = true;

    return;
}
/***********************************************************************************************************************
* End of function osal_thread_set_is_init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_thread()
* XOS1_OSAL_CD_CD_127
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_163]
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_thread(void *arg)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    st_osal_thread_control_t *control = (st_osal_thread_control_t *)arg;
    BaseType_t freertos_ret;
    TickType_t inner_time;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    while(1)
    {
        (void)xTaskNotifyWait(0U, 0U, NULL, portMAX_DELAY);
        /* This API always returns pdTRUE because using portMAX_DELAY. */

        R_OSAL_Internal_MillisecToTick(control->inner_timeout, &inner_time);
        do {
            freertos_ret = xSemaphoreTake(control->inner_mtx, inner_time);
        }while (pdFAIL == freertos_ret);

        if (NULL != control->config.func)
        {
            control->thread_ret = control->config.func(control->config.userarg);
        }

        control->active = false;

        if (NULL != control->parent_thread)
        {
            (void)xTaskNotify(control->parent_thread, 0, eNoAction);
            /* Even if the interrupt is generated after checking the active*/
            /* flag and the Thread end is notified from the Internal Thread */
            /* before waiting, the API is guaranteed to be terminated by */
            /* the xTaskNotifyWait () specification */
            /* (Refer to FreeRTOS Reference Manual). */
            /* This API always returns pdPASS because using eNoAction. */
        }

        (void)xSemaphoreGive(control->inner_mtx);
        /* Loop until the previous process locks inner_mtx. */
        /* Therefore, this API always returns pdPASS. */
    }
}
/***********************************************************************************************************************
* End of function osal_internal_thread()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_thread_find_control()
* XOS1_OSAL_CD_CD_128
* [Covers: XOS1_OSAL_UD_UD_158]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_thread_find_control(osal_thread_id_t id, st_osal_thread_control_t **thread_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_thread_control_t *control = NULL;
    st_osal_thread_control_t *local_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_RCFG_GetNumOfThread();

    while ((count < num_of_info) && (OSAL_RETURN_OK == osal_ret) && (NULL == local_control))
    {
        api_ret = R_OSAL_RCFG_GetThreadInfo(count, &control);
        if ((OSAL_RETURN_OK == api_ret) && (NULL != control) && (NULL != control->usr_config))
        {
            if (id == control->usr_config->id)
            {
                local_control = control;
            }
        }
        else
        {
            OSAL_DEBUG("Thread Open: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }

        count++;
    }

    *thread_control = local_control;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_thread_find_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_thread_set_control()
* XOS1_OSAL_CD_CD_129
* [Covers: XOS1_OSAL_UD_UD_158]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_thread_set_control(st_osal_thread_config_t const *p_config,
                                                    st_osal_thread_control_t *thread_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    BaseType_t freertos_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_thread_is_initialized)
    {
        OSAL_DEBUG("Thread Open: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (true == thread_control->active)
    {
        OSAL_DEBUG("Thread Open: OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else
    {
        if (NULL != p_config)
        {
            if (NULL != p_config->func)
            {
                thread_control->config.func = p_config->func;
            }
            if (NULL != p_config->userarg)
            {
                thread_control->config.userarg = p_config->userarg;
            }
        }

        thread_control->thread_ret = 0;

        freertos_ret = xTaskNotify(thread_control->thread, 0, eNoAction);
        if(pdPASS == freertos_ret)
        {
            thread_control->use    = true;
            thread_control->active = true;
        }
        else
        {
            OSAL_DEBUG("Thread Open: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_thread_set_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThreadInitializeThreadConfigSt()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThreadInitializeThreadConfigSt(st_osal_thread_config_t* const p_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_config)
    {
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        p_config->func          = NULL;
        p_config->userarg       = NULL;
        p_config->priority      = OSAL_THREAD_PRIORITY_TYPE0;
        p_config->task_name     = "";
        p_config->stack_size    = 0;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThreadInitializeThreadConfigSt()
***********************************************************************************************************************/

