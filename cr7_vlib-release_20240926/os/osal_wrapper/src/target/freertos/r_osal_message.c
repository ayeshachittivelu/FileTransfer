/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_message.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper Message Manager for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"

/*******************************************************************************************************************//**
 * @var gs_osal_message_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_message_is_initialized = false;

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Functions_Message MessageQueue Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Check Message Queue Manager handle
* @param[in]        handle  To set the handle
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_handle(osal_mq_handle_t handle);

/*******************************************************************************************************************//**
* @brief            Check Message Queue Manager parameter
* @param[in]        handle  To set the handle
* @param[in]        p_buffer  To set the buffer address
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_param(osal_mq_handle_t handle, const void * p_buffer);

/*******************************************************************************************************************//**
* @brief            Check Message Queue with timestamp
* @param[in]        handle  To set the handle
* @param[in]        p_time_stamp  To set the time stamp
* @param[in]        p_buffer  To set the buffer address
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_with_timestamp(osal_mq_handle_t handle,
                                                         const st_osal_time_t *const p_time_stamp,
                                                         const void * p_buffer);

/*******************************************************************************************************************//**
* @brief            Check Message Queue with timeperiod
* @param[in]        handle  To set the handle
* @param[in]        time_period  To set the time period
* @param[in]        p_buffer  To set the buffer address
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_with_timeperiod(osal_mq_handle_t handle,
                                                          osal_milli_sec_t time_period,
                                                          const void * p_buffer);

/*******************************************************************************************************************//**
* @brief            Get time period
* @param[in]        p_time_stamp To set the time stamp
* @param[out]       p_differece_time  To set the address of difference time
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_get_time_period(const st_osal_time_t *const p_time_stamp,
                                                    osal_nano_sec_t *const p_differece_time);

/*******************************************************************************************************************//**
* @brief            Check Message Queue with timeperiod
* @param[in]        handle  To set the handle
* @param[in]        time_period  To set the time period
* @param[in]        p_buffer  To set the buffer address
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_send(osal_mq_handle_t handle, osal_milli_sec_t time_period, const void * p_buffer);

/*******************************************************************************************************************//**
* @brief            Receive Message
* @param[in]        handle  To set the handle
* @param[in]        time_period  To set the time period
* @param[out]       p_buffer  To set the buffer address
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_TIME
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_receive(osal_mq_handle_t handle, osal_milli_sec_t time_period, void *p_buffer);

/*******************************************************************************************************************//**
* @brief            Search the mq control corresponding to the ID.
* @param[in]        id The value of mq id.
* @param[in, out]   mq_control valid mq control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_find_control(osal_mq_id_t id, st_osal_mq_control_t **mq_control);

/*******************************************************************************************************************//**
* @brief            Set mq control.
* @param[in, out]   control Setting target control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_set_control(st_osal_mq_control_t *control);

/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_MqCreate()
* XOS1_OSAL_CD_CD_300
* [Covers: XOS1_OSAL_UD_UD_244]
* [Covers: XOS1_OSAL_UD_UD_245]
* [Covers: XOS1_OSAL_UD_UD_246]
* [Covers: XOS1_OSAL_UD_UD_242]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_042]
* [Covers: XOS1_OSAL_UD_UD_215]
* [Covers: XOS1_OSAL_UD_UD_216]
* [Covers: XOS1_OSAL_UD_UD_243]
* [Covers: XOS1_OSAL_UD_UD_311]
* [Covers: XOS1_OSAL_UD_UD_307]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqCreate(const st_osal_mq_config_t *const p_config, osal_mq_id_t mq_Id,
                                osal_mq_handle_t *const p_handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_mq_control_t *control = NULL;
    BaseType_t semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%lu, %p)\n", mq_Id, (void *)p_handle);

    if ((NULL == p_config) || (NULL == p_handle))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = osal_mq_find_control(mq_Id, &control);
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if(NULL == control)
        {
            OSAL_DEBUG("OSAL_RETURN_ID\n");
            osal_ret = OSAL_RETURN_ID;
        }
        else if (NULL == control->inner_mtx)
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
        semaphore_ret = xSemaphoreTake(control->inner_mtx, pdMS_TO_TICKS(control->inner_timeout));
        if (pdPASS != semaphore_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            osal_ret = osal_mq_set_control(control);
            semaphore_ret = xSemaphoreGive(control->inner_mtx);
            if (pdFAIL == semaphore_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        *p_handle = control;
    }
    else
    {
        if (NULL != p_handle)
        {
            *p_handle = OSAL_MQ_HANDLE_INVALID;
        }
    }

    OSAL_DEBUG("End (%u), handle:%p \n", osal_ret, (void *)control);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqCreate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqDelete()
* XOS1_OSAL_CD_CD_305
* [Covers: XOS1_OSAL_UD_UD_176]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_043]
* [Covers: XOS1_OSAL_UD_UD_101]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqDelete(osal_mq_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    UBaseType_t queue_spaces_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p)\n", (void *)handle);

    api_ret = osal_check_mq_handle(handle);

    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        semaphore_ret = xSemaphoreTake(handle->inner_mtx, pdMS_TO_TICKS(handle->inner_timeout));
        if (pdPASS != semaphore_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            queue_spaces_ret = uxQueueSpacesAvailable(handle->mqd);
            if (NULL == handle->usr_config)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else if ((handle->usr_config->config.max_num_msg != queue_spaces_ret) ||
                     (true == handle->receive_wait) ||
                     (true == handle->send_wait))
            {
                OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                osal_ret = OSAL_RETURN_BUSY;
            }
            else
            {
                handle->active = false;
            }

            semaphore_ret = xSemaphoreGive(handle->inner_mtx);
            if (pdFAIL == semaphore_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqDelete()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqSendUntilTimeStamp()
* XOS1_OSAL_CD_CD_308
* [Covers: XOS1_OSAL_UD_UD_177]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_102]
* [Covers: XOS1_OSAL_UD_UD_247]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqSendUntilTimeStamp(osal_mq_handle_t handle, const st_osal_time_t *const p_time_stamp,
                                            const void * p_buffer, size_t buffer_size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    osal_nano_sec_t time_period_nano = 0;
    osal_milli_sec_t time_period_milli;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p, %p, %zu)\n", (void *)handle, (void *)p_time_stamp, (void *)p_buffer, buffer_size);

    api_ret = osal_check_mq_with_timestamp(handle, p_time_stamp, p_buffer);

    if ((OSAL_RETURN_OK == api_ret) && (buffer_size != handle->usr_config->config.msg_size))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        osal_ret = osal_mq_get_time_period(p_time_stamp, &time_period_nano);
        if(OSAL_RETURN_OK == osal_ret)
        {
            /* round up to millisecond */
            time_period_milli = (time_period_nano + OSAL_MSECTONSEC - 1) / OSAL_MSECTONSEC;
            osal_ret = osal_mq_send(handle, time_period_milli, p_buffer);
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqSendUntilTimeStamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqSendForTimePeriod()
* XOS1_OSAL_CD_CD_311
* [Covers: XOS1_OSAL_UD_UD_178]
* [Covers: XOS1_OSAL_UD_UD_249]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_103]
* [Covers: XOS1_OSAL_UD_UD_248]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqSendForTimePeriod(osal_mq_handle_t handle, osal_milli_sec_t time_period,
                                           const void * p_buffer, size_t buffer_size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %ld, %p, %zu)\n", (void *)handle, time_period, (void *)p_buffer, buffer_size);

    api_ret = osal_check_mq_with_timeperiod(handle, time_period, p_buffer);

    if ((OSAL_RETURN_OK == api_ret) && (buffer_size != handle->usr_config->config.msg_size))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        osal_ret = osal_mq_send(handle, time_period, p_buffer);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqSendForTimePeriod()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqReceiveUntilTimeStamp()
* XOS1_OSAL_CD_CD_314
* [Covers: XOS1_OSAL_UD_UD_179]
* [Covers: XOS1_OSAL_UD_UD_250]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_047]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqReceiveUntilTimeStamp(osal_mq_handle_t handle, const st_osal_time_t *const p_time_stamp,
                                               void *p_buffer, size_t buffer_size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    osal_nano_sec_t time_period_nano = 0;
    osal_milli_sec_t time_period_milli;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p, %p, %zu)\n", (void *)handle, (void *)p_time_stamp, (void *)p_buffer, buffer_size);
    api_ret = osal_check_mq_with_timestamp(handle, p_time_stamp, p_buffer);

    if ((OSAL_RETURN_OK == api_ret) && (buffer_size != handle->usr_config->config.msg_size))
    {

        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        osal_ret = osal_mq_get_time_period(p_time_stamp, &time_period_nano);
        if(OSAL_RETURN_OK == osal_ret)
        {
            /* round up to millisecond */
            time_period_milli = (time_period_nano + OSAL_MSECTONSEC - 1) / OSAL_MSECTONSEC;
            osal_ret = osal_mq_receive(handle, time_period_milli, p_buffer);
        }
    }
    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqReceiveUntilTimeStamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqReceiveForTimePeriod()
* XOS1_OSAL_CD_CD_316
* [Covers: XOS1_OSAL_UD_UD_180]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_104]
* [Covers: XOS1_OSAL_UD_UD_251]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqReceiveForTimePeriod(osal_mq_handle_t handle, osal_milli_sec_t time_period,
                                              void *p_buffer, size_t buffer_size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %ld, %p, %zu)\n", (void *)handle, time_period, (void *)p_buffer, buffer_size);

    api_ret = osal_check_mq_with_timeperiod(handle, time_period, p_buffer);

    if ((OSAL_RETURN_OK == api_ret) && (buffer_size != handle->usr_config->config.msg_size))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        osal_ret = osal_mq_receive(handle, time_period, p_buffer);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqReceiveForTimePeriod()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqIsFull()
* XOS1_OSAL_CD_CD_319
* [Covers: XOS1_OSAL_UD_UD_181]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_046]
* [Covers: XOS1_OSAL_UD_UD_105]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqIsFull(osal_mq_handle_t handle, bool *const p_result)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t queue_full_ret;
    UBaseType_t queue_spaces_ret;
    bool p_is_isr = false;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_result);

    api_ret = osal_check_mq_handle(handle);

    if ((OSAL_RETURN_OK == api_ret) && (NULL == p_result))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(&p_is_isr);
        if(true == p_is_isr) /* ISR context*/
        {
            queue_full_ret = xQueueIsQueueFullFromISR(handle->mqd);
            if (pdFALSE != queue_full_ret)  /* full */
            {
                *p_result = true;
            }
            else /* not full */
            {
                *p_result = false;
            }
        }
        else /* task context */
        {
            queue_spaces_ret = uxQueueSpacesAvailable(handle->mqd);
            if (0U == queue_spaces_ret)  /* number of free spaces */
            {
                *p_result = true;
            }
            else
            {
                *p_result = false;
            }
        }

    }

    OSAL_DEBUG("End (%u), handle:%d\n", osal_ret, *p_result);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqIsFull()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqIsEmpty()
* XOS1_OSAL_CD_CD_322
* [Covers: XOS1_OSAL_UD_UD_182]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_045]
* [Covers: XOS1_OSAL_UD_UD_106]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqIsEmpty(osal_mq_handle_t handle, bool *const p_result)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t queue_empty_ret;
    UBaseType_t queue_spaces_ret;
    bool p_is_isr = false;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_result);

    api_ret = osal_check_mq_handle(handle);

    if ((OSAL_RETURN_OK == api_ret) && (NULL == p_result))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(&p_is_isr);
        if(true == p_is_isr) /* ISR context*/
        {
            queue_empty_ret = xQueueIsQueueEmptyFromISR(handle->mqd);
            if (pdFALSE != queue_empty_ret)  /* empty */
            {
                *p_result = true;
            }
            else /* not empty */
            {
                *p_result = false;
            }
        }
        else /* task context */
        {
            queue_spaces_ret = uxQueueSpacesAvailable(handle->mqd);
            if (handle->usr_config->config.max_num_msg == queue_spaces_ret)
            {
                *p_result = true;
            }
            else
            {
                *p_result = false;
            }
        }

    }

    OSAL_DEBUG("End (%u), handle:%d\n", osal_ret, *p_result);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqIsEmpty()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqGetConfig()
* XOS1_OSAL_CD_CD_325
* [Covers: XOS1_OSAL_UD_UD_252]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_107]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqGetConfig(osal_mq_handle_t handle, st_osal_mq_config_t *const p_config)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_config);

    api_ret = osal_check_mq_handle(handle);

    if ((OSAL_RETURN_OK == api_ret) && (NULL == p_config))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if (NULL != handle->usr_config)
        {
            p_config->max_num_msg = handle->usr_config->config.max_num_msg;
            p_config->msg_size = handle->usr_config->config.msg_size;
        }
        else
        {
            OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
            osal_ret = OSAL_RETURN_HANDLE;
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqGetConfig()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqReset()
* XOS1_OSAL_CD_CD_327
* [Covers: XOS1_OSAL_UD_UD_183]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_108]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqReset(osal_mq_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t queue_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p)\n", (void *)handle);

    api_ret = osal_check_mq_handle(handle);

    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        queue_ret = xQueueReset(handle->mqd);
        if (pdPASS != queue_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqReset()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_initialize()
* XOS1_OSAL_CD_CD_329
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_010]
* [Covers: XOS1_OSAL_UD_UD_011]
* [Covers: XOS1_OSAL_UD_UD_012]
* [Covers: XOS1_OSAL_UD_UD_044]
* [Covers: XOS1_OSAL_UD_UD_145]
* [Covers: XOS1_OSAL_UD_UD_321]
***********************************************************************************************************************/
e_osal_return_t osal_mq_initialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info = 0;
    uint32_t count = 0;
    st_osal_mq_control_t *control = NULL;
    SemaphoreHandle_t mtx;
    QueueHandle_t mqd;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");
    if (true == gs_osal_message_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_message_is_initialized = true;
        num_of_info = R_OSAL_RCFG_GetNumOfMq();
    }
    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetMqInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->usr_config))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* check Message queue configuration */
            if ((0U == control->usr_config->config.max_num_msg) ||
                (0U == control->usr_config->config.msg_size))
            {
                OSAL_DEBUG("OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else /* Configuration check PASS */
            {
                mqd = xQueueCreate(control->usr_config->config.max_num_msg, control->usr_config->config.msg_size);
                if (NULL == mqd)
                {
                    OSAL_DEBUG("OSAL_RETURN_MEM\n");
                    osal_ret = OSAL_RETURN_MEM;
                }
                else
                {
                    mtx = xSemaphoreCreateBinary();
                    if (NULL == mtx)
                    {
                        OSAL_DEBUG("OSAL_RETURN_MEM\n");
                        osal_ret = OSAL_RETURN_MEM;
                    }
                    else
                    {
                        /* control initialize*/
                        control->handle_id = OSAL_MESSAGE_HANDLE_ID;
                        control->mqd = mqd;
                        control->inner_mtx = mtx;
                        (void)xSemaphoreGive(mtx);
                        control->active = false;
                        control->send_wait = false;
                        control->receive_wait = false;
                    }
                }
            }
        }

        count++;
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_mq_deinitialize();
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_deinitialize()
* XOS1_OSAL_CD_CD_335
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_006]
* [Covers: XOS1_OSAL_UD_UD_044]
* [Covers: XOS1_OSAL_UD_UD_330]
***********************************************************************************************************************/
e_osal_return_t osal_mq_deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mq_control_t *control = NULL;


    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_message_is_initialized = false;

    num_of_info = R_OSAL_RCFG_GetNumOfMq();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetMqInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {

            if (NULL != control->mqd)
            {
                vQueueDelete(control->mqd);
                control->mqd = NULL;
            }

            if (NULL != control->inner_mtx)
            {
                vSemaphoreDelete(control->inner_mtx);
                control->inner_mtx = NULL;
            }

            control->handle_id = 0;
            control->active = false;
            control->send_wait = false;
            control->receive_wait = false;
        }
        count++;
    }

    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_mq_deinitialize_pre(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mq_control_t *control = NULL;


    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_message_is_initialized = false;

    num_of_info = R_OSAL_RCFG_GetNumOfMq();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetMqInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (true == control->active)
            {
                osal_ret = OSAL_RETURN_BUSY;
            }
        }
        count++;
    }

    if (OSAL_RETURN_BUSY == osal_ret)
    {
        gs_osal_message_is_initialized = true;
    }
    else
    {
        /* Do nothing */
    }

    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_set_is_init()
***********************************************************************************************************************/
void osal_mq_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_message_is_initialized = true;

    return;
}

/***********************************************************************************************************************
* Start of function osal_check_mq_handle()
* XOS1_OSAL_CD_CD_336
* [Covers: XOS1_OSAL_UD_UD_176]
* [Covers: XOS1_OSAL_UD_UD_181]
* [Covers: XOS1_OSAL_UD_UD_182]
* [Covers: XOS1_OSAL_UD_UD_252]
* [Covers: XOS1_OSAL_UD_UD_183]
* [Covers: XOS1_OSAL_UD_UD_177]
* [Covers: XOS1_OSAL_UD_UD_179]
* [Covers: XOS1_OSAL_UD_UD_178]
* [Covers: XOS1_OSAL_UD_UD_180]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_handle(osal_mq_handle_t handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_message_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((false == handle->active) || (OSAL_MESSAGE_HANDLE_ID != handle->handle_id))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        osal_ret = OSAL_RETURN_OK;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_mq_handle()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_check_mq_param()
* XOS1_OSAL_CD_CD_337
* [Covers: XOS1_OSAL_UD_UD_177]
* [Covers: XOS1_OSAL_UD_UD_179]
* [Covers: XOS1_OSAL_UD_UD_178]
* [Covers: XOS1_OSAL_UD_UD_180]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_param(osal_mq_handle_t handle, const void * p_buffer)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_check_mq_handle(handle);

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (NULL == p_buffer)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_mq_param()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_check_mq_with_timestamp()
* XOS1_OSAL_CD_CD_338
* [Covers: XOS1_OSAL_UD_UD_177]
* [Covers: XOS1_OSAL_UD_UD_179]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_with_timestamp(osal_mq_handle_t handle,
                                                         const st_osal_time_t *const p_time_stamp,
                                                         const void * p_buffer)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_check_mq_param(handle, p_buffer);

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (NULL == p_time_stamp)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
        }
        else
        {
            api_ret = R_OSAL_Internal_CheckTimestamp(p_time_stamp);
            if (OSAL_RETURN_OK != api_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_PAR\n");
                osal_ret = OSAL_RETURN_PAR;
            }
            else
            {
                osal_ret = OSAL_RETURN_OK;
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_mq_with_timestamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_check_mq_with_timeperiod()
* XOS1_OSAL_CD_CD_339
* [Covers: XOS1_OSAL_UD_UD_178]
* [Covers: XOS1_OSAL_UD_UD_180]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_mq_with_timeperiod(osal_mq_handle_t handle,
                                                          osal_milli_sec_t time_period,
                                                          const void * p_buffer)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_check_mq_param(handle, p_buffer);

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (0 > time_period)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_mq_with_timeperiod()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_get_time_period()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_get_time_period(const st_osal_time_t *const p_time_stamp,
                                                    osal_nano_sec_t *const p_differece_time)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_time_t current_time_stamp;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    
    osal_ret = R_OSAL_Internal_GetTimestamp(&current_time_stamp);

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = R_OSAL_Internal_GetDiffTime(p_time_stamp, &current_time_stamp, p_differece_time);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_get_time_period()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_send()
* XOS1_OSAL_CD_CD_340
* [Covers: XOS1_OSAL_UD_UD_177]
* [Covers: XOS1_OSAL_UD_UD_178]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_send(osal_mq_handle_t handle, osal_milli_sec_t time_period, const void * p_buffer)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    BaseType_t mq_ret;
    TickType_t tick_time = 0;
    bool p_is_isr = true;
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_GetISRContext(&p_is_isr);

    if(true == p_is_isr) /* ISR context*/
    {
        handle->send_wait = true;
        mq_ret = xQueueSendFromISR(handle->mqd, p_buffer, &pxHigherPriorityTaskWoken);
        handle->send_wait = false;

        if (errQUEUE_FULL == mq_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_BUSY\n");
            osal_ret = OSAL_RETURN_BUSY;
        }
        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
    else /* task context */
    {
        R_OSAL_Internal_MillisecToTick(time_period, &tick_time);
        handle->send_wait = true;
        mq_ret = xQueueSend(handle->mqd, p_buffer, tick_time);
        handle->send_wait = false;

        if (errQUEUE_FULL == mq_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_TIME\n");
            osal_ret = OSAL_RETURN_TIME;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_send()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_receive()
* XOS1_OSAL_CD_CD_341
* [Covers: XOS1_OSAL_UD_UD_179]
* [Covers: XOS1_OSAL_UD_UD_180]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_receive(osal_mq_handle_t handle, osal_milli_sec_t time_period, void *p_buffer)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    BaseType_t mq_ret;
    TickType_t tick_time = 0;
    bool p_is_isr = true;
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_GetISRContext(&p_is_isr);
    if(true == p_is_isr) /* ISR context*/
    {
        handle->receive_wait = true;
        mq_ret = xQueueReceiveFromISR(handle->mqd, p_buffer, &pxHigherPriorityTaskWoken);
        handle->receive_wait = false;

        if (pdFAIL == mq_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_BUSY\n");
            osal_ret = OSAL_RETURN_BUSY;
        }
        portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
    else /* task context */
    {
         R_OSAL_Internal_MillisecToTick(time_period, &tick_time);
         handle->receive_wait = true;
         mq_ret = xQueueReceive(handle->mqd, p_buffer, tick_time);
         handle->receive_wait = false;

        if (errQUEUE_EMPTY == mq_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_TIME\n");
            osal_ret = OSAL_RETURN_TIME;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_receive()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_find_control()
* XOS1_OSAL_CD_CD_342
* [Covers: XOS1_OSAL_UD_UD_164]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_find_control(osal_mq_id_t id, st_osal_mq_control_t **mq_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mq_control_t *control = NULL;
    st_osal_mq_control_t *local_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_RCFG_GetNumOfMq();

    while ((count < num_of_info) && (OSAL_RETURN_OK == osal_ret) && (NULL == local_control))
    {
        api_ret = R_OSAL_RCFG_GetMqInfo(count, &control);
        if ((OSAL_RETURN_OK == api_ret) && (NULL != control) && (NULL != control->usr_config))
        {
            if (id == control->usr_config->id)
            {
                local_control = control;
            }
        }
        else
        {
            OSAL_DEBUG("Mutex Open: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }

        count++;
    }

    *mq_control = local_control;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_find_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mq_set_control()
* XOS1_OSAL_CD_CD_343
* [Covers: XOS1_OSAL_UD_UD_164]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mq_set_control(st_osal_mq_control_t *control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    BaseType_t queue_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if ((false == gs_osal_message_is_initialized) || (NULL == control->mqd))
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (true == control->active)
    {
        OSAL_DEBUG("OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else
    {
        control->active = true;

        queue_ret = xQueueReset(control->mqd);
        if (pdPASS != queue_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mq_set_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MqInitializeMqConfigSt()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MqInitializeMqConfigSt(st_osal_mq_config_t* const p_config)
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
        p_config->max_num_msg   = 0;
        p_config->msg_size      = 0;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MqInitializeMqConfigSt()
***********************************************************************************************************************/

