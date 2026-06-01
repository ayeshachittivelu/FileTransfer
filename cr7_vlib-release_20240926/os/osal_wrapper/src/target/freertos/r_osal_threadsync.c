/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_threadsync.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper Threadsync Manager for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"
#include "r_io.h"
#include "config/freertos/r_osal_resource_definition.h"
/*******************************************************************************************************************//**
 * @var gs_osal_threadsync_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_threadsync_is_initialized = false;

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Functions_ThreadSync ThreadSync Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Lock a mutex with timeout for specified time.
* @param[in]        handle  The handle acquired at R_OSAL_ThsyncMutexCreate ().
* @param[in]        wait_time Transfer The value for timeout by [msec] order.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_TIME
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_lock(osal_mutex_handle_t handle, osal_milli_sec_t wait_time);

/*******************************************************************************************************************//**
* @brief            Wait on a condition variable with timeout until time stamp.
* @param[in]        cond_handle  The handle acquired at R_OSAL_ThsyncCondCreate ().
* @param[in]        mutex_handle  The handle acquired at R_OSAL_ThsyncMutexCreate ().
* @param[in]        time_period  The value for timeout by [msec] order.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_TIME
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_condwait(osal_cond_handle_t cond_handle, osal_mutex_handle_t mutex_handle,
                                                   osal_milli_sec_t time_period);

/*******************************************************************************************************************//**
* @brief            Core processing waiting for condition variable.
* @param[in]        cond_handle  The handle acquired at R_OSAL_ThsyncCondCreate ().
* @param[in]        use_list  event list.
* @param[in]        time_period  The value for timeout by [msec] order.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_TIME
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_condwait_core(osal_cond_handle_t cond_handle, osal_mutex_handle_t mutex_handle,
                                                        st_osal_event_list_t *use_list, osal_milli_sec_t time_period);

/*******************************************************************************************************************//**
* @brief            Search the mutex control corresponding to the ID.
* @param[in]        id The value of mutex id.
* @param[in, out]   mutex_control valid mutex control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_find_control(osal_mutex_id_t id, st_osal_mutex_control_t **mutex_control);

/*******************************************************************************************************************//**
* @brief            Set mutex control.
* @param[in, out]   mutex_control Setting target control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_set_control(st_osal_mutex_control_t *mutex_control);

/*******************************************************************************************************************//**
* @brief            Search the mutex control corresponding to the ID.
* @param[in]        id The value of condition variavble id.
* @param[in, out]   cond_control valid condition variavble control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_find_control(osal_cond_id_t id, st_osal_cond_control_t **cond_control);

/*******************************************************************************************************************//**
* @brief            Set condition variable control.
* @param[in, out]   cond_control Setting target control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_set_control(st_osal_cond_control_t *cond_control);

/*******************************************************************************************************************//**
* @brief            Get event bit for waiting condition variable.
* @param[in]        handle Setting target control.
* @param[in]        flag true: all events / false: single event.
* @return           EventBits_t
***********************************************************************************************************************/
OSAL_STATIC EventBits_t osal_internal_cond_get_events(osal_cond_handle_t handle, bool flag);


/*******************************************************************************************************************//**
* @brief            Initialize Mutex Synchronization Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_initialize(void);

/*******************************************************************************************************************//**
* @brief            Initialize Cond Synchronization Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Mutex Synchronization Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Thread Synchronization Manager (mutex).
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Cond Synchronization Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Thread Synchronization Manager (condition).
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_deinitialize_pre(void);

/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMutexCreate()
* XOS1_OSAL_CD_CD_200
* [Covers: XOS1_OSAL_UD_UD_164]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_066]
* [Covers: XOS1_OSAL_UD_UD_087]
* [Covers: XOS1_OSAL_UD_UD_211]
* [Covers: XOS1_OSAL_UD_UD_213]
* [Covers: XOS1_OSAL_UD_UD_310]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMutexCreate(osal_mutex_id_t mutex_id, osal_mutex_handle_t *const p_handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_mutex_control_t *mutex_control = NULL;
    BaseType_t freertos_ret;
    TickType_t inner_time;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%lu, %p)\n", mutex_id, (void *)p_handle);

    if (NULL == p_handle)
    {
        OSAL_DEBUG("Mutex Open: OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = osal_mutex_find_control(mutex_id, &mutex_control);
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if(NULL == mutex_control)
        {
            OSAL_DEBUG("Mutex Open: OSAL_RETURN_ID\n");
            osal_ret = OSAL_RETURN_ID;
        }
        else if (NULL == mutex_control->inner_mtx)
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
        R_OSAL_Internal_MillisecToTick(mutex_control->inner_timeout, &inner_time);
        freertos_ret = xSemaphoreTake(mutex_control->inner_mtx, inner_time);
        if(pdPASS == freertos_ret)
        {
            osal_ret = osal_mutex_set_control(mutex_control);
            if (OSAL_RETURN_OK == osal_ret)
            {
                *p_handle = mutex_control;
            }
            freertos_ret = xSemaphoreGive(mutex_control->inner_mtx);
            if(pdFAIL == freertos_ret)
            {
                mutex_control->active = false;
                OSAL_DEBUG("Mutex Open: OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
        else
        {
            OSAL_DEBUG("Mutex Open: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    if ((OSAL_RETURN_OK != osal_ret) && (NULL != p_handle))
    {
        *p_handle = OSAL_MUTEX_HANDLE_INVALID;
    }

    OSAL_DEBUG("End (%u, %p)\n", osal_ret, (void *)*p_handle);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMutexCreate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMutexDestroy()
* XOS1_OSAL_CD_CD_205
* [Covers: XOS1_OSAL_UD_UD_165]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_068]
* [Covers: XOS1_OSAL_UD_UD_089]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMutexDestroy(osal_mutex_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    BaseType_t freertos_ret;
    TickType_t inner_time;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p)\n", (void *)handle);

    if (false == gs_osal_threadsync_is_initialized)
    {
        if (NULL != handle)
        {
            if (NULL != handle->usr_config)
            {
                if ((OSAL_MUTEX_DRIVER_CPG != handle->usr_config->id) && ( OSAL_MUTEX_DRIVER_SYSC != handle->usr_config->id))
                {
                    OSAL_DEBUG("Mutex Destroy:: OSAL_RETURN_STATE\n");
                    osal_ret = OSAL_RETURN_STATE;                    
                }
            }
            else
            {
                OSAL_DEBUG("Mutex Destroy:: OSAL_RETURN_STATE\n");
                osal_ret = OSAL_RETURN_STATE;       
            }
        }
        else
        {
            OSAL_DEBUG("Mutex Destroy:: OSAL_RETURN_STATE\n");
            osal_ret = OSAL_RETURN_STATE;       
        }
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("Mutex Destroy: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_MUTEX_HANDLE_ID != handle->handle_id) || (false == handle->active))
    {
        OSAL_DEBUG("Mutex Destroy: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (true == handle->use)
    {
        OSAL_DEBUG("Mutex Destroy: OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        R_OSAL_Internal_MillisecToTick(handle->inner_timeout, &inner_time);

        freertos_ret = xSemaphoreTake(handle->inner_mtx, inner_time);
        if(pdPASS == freertos_ret)
        {
            handle->active = false;

            freertos_ret = xSemaphoreGive(handle->inner_mtx);
            if(pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("Mutex Destroy: OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }

        }
        else
        {
            OSAL_DEBUG("Mutex Open: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }

    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMutexDestroy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMutexLockUntilTimeStamp()
* XOS1_OSAL_CD_CD_208
* [Covers: XOS1_OSAL_UD_UD_167]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_092]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMutexLockUntilTimeStamp(osal_mutex_handle_t handle,
                                                     const st_osal_time_t *const p_time_stamp)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    osal_milli_sec_t wait_time;
    st_osal_time_t time_stamp;
    osal_nano_sec_t nano_sec;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p)\n", (void *)handle);

    if (NULL == p_time_stamp)
    {
        OSAL_DEBUG("Mutex Lock TimeStamp: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = R_OSAL_Internal_CheckTimestamp(p_time_stamp);
        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = R_OSAL_Internal_GetTimestamp(&time_stamp);
            if(OSAL_RETURN_OK == osal_ret)
            {
                osal_ret = R_OSAL_Internal_GetDiffTime(p_time_stamp, &time_stamp, &nano_sec);
                if(OSAL_RETURN_OK == osal_ret)
                {
                    /* round up to millisecond */
                    wait_time = (nano_sec + OSAL_MSECTONSEC - 1) / OSAL_MSECTONSEC;

                    osal_ret = osal_internal_lock(handle, wait_time);
                }
            }
            else
            {
                OSAL_DEBUG("Mutex Lock TimeStamp: OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMutexLockUntilTimeStamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMutexLockForTimePeriod()
* XOS1_OSAL_CD_CD_210
* [Covers: XOS1_OSAL_UD_UD_166]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_090]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMutexLockForTimePeriod(osal_mutex_handle_t handle, osal_milli_sec_t time_period)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p)\n", (void *)handle);

    if (0 > time_period)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = osal_internal_lock(handle, time_period);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMutexLockForTimePeriod()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMutexTryLock()
* XOS1_OSAL_CD_CD_213
* [Covers: XOS1_OSAL_UD_UD_168]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_093]
* [Covers: XOS1_OSAL_UD_UD_241]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMutexTryLock(osal_mutex_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
 
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p)\n", (void *)handle);

    osal_ret = osal_internal_lock(handle, 0);
    if (OSAL_RETURN_TIME == osal_ret)
    {
        OSAL_DEBUG("OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMutexTryLock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMutexUnlock()
* XOS1_OSAL_CD_CD_216
* [Covers: XOS1_OSAL_UD_UD_169]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_071]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMutexUnlock(osal_mutex_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    BaseType_t freertos_ret;
    bool is_isr = false;
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
    xTaskHandle current_task, mutex_owner_task;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start (%p)\n", (void *)handle);

    if (false == gs_osal_threadsync_is_initialized)
    {
        if (NULL != handle)
        {
            if (NULL != handle->usr_config)
            {
                if ((OSAL_MUTEX_DRIVER_CPG != handle->usr_config->id) && ( OSAL_MUTEX_DRIVER_SYSC != handle->usr_config->id))
                {
                    OSAL_DEBUG("Mutex UnLock:: OSAL_RETURN_STATE\n");
                    osal_ret = OSAL_RETURN_STATE;                    
                }
            }
            else
            {
                OSAL_DEBUG("Mutex UnLock:: OSAL_RETURN_STATE\n");
                osal_ret = OSAL_RETURN_STATE;       
            }
        }
        else
        {
            OSAL_DEBUG("Mutex UnLock:: OSAL_RETURN_STATE\n");
            osal_ret = OSAL_RETURN_STATE;       
        }
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_MUTEX_HANDLE_ID != handle->handle_id) || (false == handle->active))
    {
        OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(&is_isr);
        if (true == is_isr)
        {
            //current_task = xTaskGetCurrentTaskHandle();
            mutex_owner_task = xSemaphoreGetMutexHolderFromISR(handle->mtx);
            if ( (xTaskHandle)0 != mutex_owner_task)
            {
            OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_FAIL(task)\n");
            osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                if (true == handle->use)
                {
                    handle->use = false;
                    freertos_ret = xSemaphoreGiveFromISR(handle->mtx, &pxHigherPriorityTaskWoken);
                    if (pdTRUE != freertos_ret)
                    {
                        handle->use = true;
                        OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                   portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
                }
                else
                {
                    OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
        else
        {
            current_task = xTaskGetCurrentTaskHandle();
            mutex_owner_task = xSemaphoreGetMutexHolder(handle->mtx);
            if ( current_task != mutex_owner_task)
            {
            OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_FAIL(task)\n");
            osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                if (true == handle->use)
                {
                    handle->use = false;
                    freertos_ret = xSemaphoreGive(handle->mtx);
                    if (pdFAIL == freertos_ret)
                    {
                        handle->use = true;
                        OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                }
                else
                {
                    OSAL_DEBUG("Mutex UnLock: OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMutexUnlock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncCondCreate()
* XOS1_OSAL_CD_CD_218
* [Covers: XOS1_OSAL_UD_UD_170]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_058]
* [Covers: XOS1_OSAL_UD_UD_094]
* [Covers: XOS1_OSAL_UD_UD_212]
* [Covers: XOS1_OSAL_UD_UD_214]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncCondCreate(osal_cond_id_t cond_id, osal_cond_handle_t *const p_handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_cond_control_t *cond_control = NULL;
    BaseType_t freertos_ret;
    TickType_t inner_time;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");
    if (NULL == p_handle)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = osal_cond_find_control(cond_id, &cond_control);

    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if(NULL == cond_control)
        {
            OSAL_DEBUG("OSAL_RETURN_ID\n");
            osal_ret = OSAL_RETURN_ID;
        }
        else if (NULL == cond_control->inner_mtx)
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
        R_OSAL_Internal_MillisecToTick(cond_control->inner_timeout, &inner_time);
        freertos_ret = xSemaphoreTake(cond_control->inner_mtx, inner_time);
        if(pdPASS == freertos_ret)
        {
            osal_ret = osal_cond_set_control(cond_control);
            if (OSAL_RETURN_OK == osal_ret)
            {
                *p_handle = cond_control;
            }
            freertos_ret = xSemaphoreGive(cond_control->inner_mtx);
            if(pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
        else
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    if ((OSAL_RETURN_OK != osal_ret) && (NULL != p_handle))
    {
        *p_handle = OSAL_COND_HANDLE_INVALID;
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncCondCreate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncCondDestroy()
* XOS1_OSAL_CD_CD_223
* [Covers: XOS1_OSAL_UD_UD_171]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_059]
* [Covers: XOS1_OSAL_UD_UD_060]
* [Covers: XOS1_OSAL_UD_UD_095]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncCondDestroy(osal_cond_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    BaseType_t freertos_ret;
    TickType_t inner_time;
    bool api_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (false == gs_osal_threadsync_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_COND_HANDLE_ID != handle->handle_id) || (false == handle->active))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        osal_ret = OSAL_RETURN_OK;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        R_OSAL_Internal_MillisecToTick(handle->inner_timeout, &inner_time);

        freertos_ret = xSemaphoreTake(handle->inner_mtx, inner_time);
        if(pdPASS == freertos_ret)
        {
            api_ret = R_OSAL_Internal_CheckListEmpty(handle->wait_list);
            if (false == api_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                osal_ret = OSAL_RETURN_BUSY;
            }
            else
            {
                (void)xEventGroupClearBits(handle->event, OSAL_EVENT_CLEAR_BITS);

                handle->wait_list = NULL;
                handle->empty_list = NULL;
                handle->active = false;
            }

            freertos_ret = xSemaphoreGive(handle->inner_mtx);
            if(pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
        else
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }
    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncCondDestroy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncCondWaitForTimePeriod()
* XOS1_OSAL_CD_CD_227
* [Covers: XOS1_OSAL_UD_UD_172]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_062]
* [Covers: XOS1_OSAL_UD_UD_063]
* [Covers: XOS1_OSAL_UD_UD_096]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncCondWaitForTimePeriod(osal_cond_handle_t cond_handle, osal_mutex_handle_t mutex_handle,
                                                   osal_milli_sec_t time_period)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (0 > time_period)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = osal_internal_condwait(cond_handle, mutex_handle, time_period);
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncCondWaitForTimePeriod()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncCondWaitUntilTimeStamp()
* XOS1_OSAL_CD_CD_232
* [Covers: XOS1_OSAL_UD_UD_173]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_064]
* [Covers: XOS1_OSAL_UD_UD_065]
* [Covers: XOS1_OSAL_UD_UD_098]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncCondWaitUntilTimeStamp(osal_cond_handle_t cond_handle, osal_mutex_handle_t mutex_handle,
                                                    const st_osal_time_t *const p_time_stamp)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    osal_milli_sec_t wait_time;
    st_osal_time_t time_stamp;
    osal_nano_sec_t nano_sec;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (NULL == p_time_stamp)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = R_OSAL_Internal_CheckTimestamp(p_time_stamp);
        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = R_OSAL_Internal_GetTimestamp(&time_stamp);
            if(OSAL_RETURN_OK == osal_ret)
            {

                osal_ret = R_OSAL_Internal_GetDiffTime(p_time_stamp, &time_stamp, &nano_sec);
                if(OSAL_RETURN_OK == osal_ret)
                {
                    /* round up to millisecond */
                    wait_time = (nano_sec + OSAL_MSECTONSEC - 1) / OSAL_MSECTONSEC;
                    osal_ret = osal_internal_condwait(cond_handle, mutex_handle, wait_time);
                }
                else
                {
                    /* UD10 FAIL SIDE */
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }

            }
            else
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
        else
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
        }
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncCondWaitUntilTimeStamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncCondBroadcast()
* XOS1_OSAL_CD_CD_236
* [Covers: XOS1_OSAL_UD_UD_174]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_057]
* [Covers: XOS1_OSAL_UD_UD_099]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncCondBroadcast(osal_cond_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    bool is_isr = false;
    BaseType_t freertos_ret;
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
    TickType_t wait_time_tick;
    EventBits_t event_bits = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (false == gs_osal_threadsync_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_COND_HANDLE_ID != handle->handle_id) || (false == handle->active))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(&is_isr);
        if(true == is_isr)
        {
            freertos_ret = xSemaphoreTakeFromISR(handle->inner_mtx, &pxHigherPriorityTaskWoken);
            if (pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                event_bits = osal_internal_cond_get_events(handle, true);

                freertos_ret = xSemaphoreGiveFromISR(handle->inner_mtx, &pxHigherPriorityTaskWoken);
                if (pdTRUE != freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }

            if ((OSAL_RETURN_OK == osal_ret) && (0U != event_bits))
            {
                freertos_ret = xEventGroupSetBitsFromISR(handle->event, event_bits, &pxHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

                if (pdFAIL == freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
        else
        {
            R_OSAL_Internal_MillisecToTick(handle->inner_timeout, &wait_time_tick);
            freertos_ret = xSemaphoreTake(handle->inner_mtx, wait_time_tick);
            if (pdFALSE == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                event_bits = osal_internal_cond_get_events(handle, true);

                freertos_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }

            if ((OSAL_RETURN_OK == osal_ret) && (0U != event_bits))
            {
                (void)xEventGroupSetBits(handle->event, event_bits);
            }
        }
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncCondBroadcast()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncCondSignal()
* XOS1_OSAL_CD_CD_239
* [Covers: XOS1_OSAL_UD_UD_175]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_061]
* [Covers: XOS1_OSAL_UD_UD_100]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncCondSignal(osal_cond_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    bool is_isr = false;
    BaseType_t freertos_ret;
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
    TickType_t wait_time_tick;
    EventBits_t event_bits = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (false == gs_osal_threadsync_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_COND_HANDLE_ID != handle->handle_id) || (false == handle->active))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(&is_isr);
        if(true == is_isr)
        {
            freertos_ret = xSemaphoreTakeFromISR(handle->inner_mtx, &pxHigherPriorityTaskWoken);
            if (pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                event_bits = osal_internal_cond_get_events(handle, false);

                freertos_ret = xSemaphoreGiveFromISR(handle->inner_mtx, &pxHigherPriorityTaskWoken);
                if (pdTRUE != freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }

            if ((OSAL_RETURN_OK == osal_ret) && (0U != event_bits))
            {
                freertos_ret = xEventGroupSetBitsFromISR(handle->event, event_bits, &pxHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

                if (pdFAIL == freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
        else
        {
            R_OSAL_Internal_MillisecToTick(handle->inner_timeout, &wait_time_tick);
            freertos_ret = xSemaphoreTake(handle->inner_mtx, wait_time_tick);
            if (pdFALSE == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                event_bits = osal_internal_cond_get_events(handle, false);

                freertos_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }

            if ((OSAL_RETURN_OK == osal_ret) && (0U != event_bits))
            {
                (void)xEventGroupSetBits(handle->event, event_bits);
            }
        }
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncCondSignal()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_ThsyncMemoryBarrier()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_ThsyncMemoryBarrier(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (true != gs_osal_threadsync_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        reg_dmb();
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_ThsyncMemoryBarrier()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mutex_initialize()
* XOS1_OSAL_CD_CD_242
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_025]
* [Covers: XOS1_OSAL_UD_UD_015]
* [Covers: XOS1_OSAL_UD_UD_146]
* [Covers: XOS1_OSAL_UD_UD_322]
***********************************************************************************************************************/
e_osal_return_t osal_mutex_initialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mutex_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Mutex Init start\n");

    num_of_info = R_OSAL_RCFG_GetNumOfMutex();
    OSAL_DEBUG("Get num_of_info: %zu\n", num_of_info);

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetMutexInfo(count, &control);
        OSAL_DEBUG("Get mutex_control: %p\n", (void *)control);

        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("Mutex Init: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            control->mtx = xSemaphoreCreateMutex();
            if (NULL == control->mtx)
            {
                OSAL_DEBUG("Mutex Init: OSAL_RETURN_MEM\n");
                osal_ret = OSAL_RETURN_MEM;
            }
            else
            {
                control->inner_mtx = xSemaphoreCreateBinary();
                if (NULL == control->inner_mtx)
                {
                    OSAL_DEBUG("Mutex Init: OSAL_RETURN_MEM\n");
                    osal_ret = OSAL_RETURN_MEM;
                }
                else
                {
                    /* control initialize*/
                    OSAL_DEBUG("Mutex Init: control initialize success(%u)\n", count);
                    control->handle_id = OSAL_MUTEX_HANDLE_ID;
                    control->active = false;
                    control->use = false;
                    (void)xSemaphoreGive(control->inner_mtx);
                }
            }
        }

        count++;
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_mutex_deinitialize();
    }

    OSAL_DEBUG("Threadsync Init end (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mutex_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_cond_initialize()
* XOS1_OSAL_CD_CD_245
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_016]
* [Covers: XOS1_OSAL_UD_UD_023]
* [Covers: XOS1_OSAL_UD_UD_024]
* [Covers: XOS1_OSAL_UD_UD_147]
* [Covers: XOS1_OSAL_UD_UD_148]
* [Covers: XOS1_OSAL_UD_UD_323]
***********************************************************************************************************************/
e_osal_return_t osal_cond_initialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_cond_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    num_of_info = R_OSAL_RCFG_GetNumOfCond();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetCondInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("Cond Init: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            control->event = xEventGroupCreate();
            if (NULL == control->event)
            {
                OSAL_DEBUG("Cond Init: OSAL_RETURN_MEM\n");
                osal_ret = OSAL_RETURN_MEM;
            }
            else
            {
                control->inner_mtx = xSemaphoreCreateBinary();
                if (NULL == control->inner_mtx)
                {
                    OSAL_DEBUG("Cond Init: OSAL_RETURN_MEM\n");
                    osal_ret = OSAL_RETURN_MEM;
                }
                else
                {
                    /* cond_control initialize*/
                    OSAL_DEBUG("Cond Init: control initialize success(%u)\n", count);
                    control->handle_id = OSAL_COND_HANDLE_ID;
                    control->active = false;
                    (void)xSemaphoreGive(control->inner_mtx);
                }
            }
        }

        count++;
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_cond_deinitialize();
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_cond_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mutex_deinitialize()
* XOS1_OSAL_CD_CD_251
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_004]
* [Covers: XOS1_OSAL_UD_UD_328]
***********************************************************************************************************************/
e_osal_return_t osal_mutex_deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mutex_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    num_of_info = R_OSAL_RCFG_GetNumOfMutex();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetMutexInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (NULL != control->mtx)
            {
                vSemaphoreDelete(control->mtx);
                control->mtx = NULL;
            }

            if (NULL != control->inner_mtx)
            {
                vSemaphoreDelete(control->inner_mtx);
                control->inner_mtx = NULL;
            }

            control->handle_id = 0;
            control->active = false;
            control->use = false;
        }
        count++;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mutex_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mutex_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_mutex_deinitialize_pre(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mutex_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    num_of_info = R_OSAL_RCFG_GetNumOfMutex();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetMutexInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if ((true == control->active) && (( OSAL_MUTEX_DRIVER_CPG != control->usr_config->id) && ( OSAL_MUTEX_DRIVER_SYSC != control->usr_config->id)))
               {
                OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                osal_ret = OSAL_RETURN_BUSY;
               }
        }
        count++;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mutex_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_cond_deinitialize()
* XOS1_OSAL_CD_CD_253
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_005]
* [Covers: XOS1_OSAL_UD_UD_329]
***********************************************************************************************************************/
e_osal_return_t osal_cond_deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_cond_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    num_of_info = R_OSAL_RCFG_GetNumOfCond();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetCondInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (NULL != control->event)
            {
                vEventGroupDelete(control->event);
                control->event = NULL;
            }

            if (NULL != control->inner_mtx)
            {
                vSemaphoreDelete(control->inner_mtx);
                control->inner_mtx = NULL;
            }

            control->handle_id = 0;
            control->active = false;
        }
        count++;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_cond_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_cond_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_cond_deinitialize_pre(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_cond_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    num_of_info = R_OSAL_RCFG_GetNumOfCond();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetCondInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (true == control->active)
               {
                OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                osal_ret = OSAL_RETURN_BUSY;
               }
        }
        count++;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_cond_deinitialize_pre()
***********************************************************************************************************************/


/***********************************************************************************************************************
* Start of function osal_internal_lock()
* XOS1_OSAL_CD_CD_254
* [Covers: XOS1_OSAL_UD_UD_166]
* [Covers: XOS1_OSAL_UD_UD_167]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_lock(osal_mutex_handle_t handle, osal_milli_sec_t wait_time)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    bool is_isr = false;
    BaseType_t pxHigherPriorityTaskWoken = pdFAIL;
    BaseType_t freertos_ret;
    TickType_t tickcount;
    xTaskHandle current_task, mutex_owner_task;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_threadsync_is_initialized)
    {
        if (NULL != handle)
        {
            if (NULL != handle->usr_config)
            {
                if ((OSAL_MUTEX_DRIVER_CPG != handle->usr_config->id) && ( OSAL_MUTEX_DRIVER_SYSC != handle->usr_config->id))
                {
                    OSAL_DEBUG("Mutex UnLock:: OSAL_RETURN_STATE\n");
                    osal_ret = OSAL_RETURN_STATE;                    
                }
            }
            else
            {
                OSAL_DEBUG("Mutex UnLock:: OSAL_RETURN_STATE\n");
                osal_ret = OSAL_RETURN_STATE;       
            }
        }
        else
        {
            OSAL_DEBUG("Mutex UnLock:: OSAL_RETURN_STATE\n");
            osal_ret = OSAL_RETURN_STATE;       
        }
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("Mutex internal_lock: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (OSAL_MUTEX_HANDLE_ID != handle->handle_id)
    {
        OSAL_DEBUG("Mutex internal_lock: OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(&is_isr);
        if (true == is_isr)
        {
#if 0
            current_task = xTaskGetCurrentTaskHandle();
            mutex_owner_task = xSemaphoreGetMutexHolderFromISR(handle->mtx);
            if ( (xTaskHandle)0 == mutex_owner_task)
            {
            OSAL_DEBUG("Mutex internal_lock: OSAL_RETURN_FAIL(task)\n");
            osal_ret = OSAL_RETURN_FAIL;
            }
            else
#endif
            {
                freertos_ret = xSemaphoreTakeFromISR(handle->mtx, &pxHigherPriorityTaskWoken);
                if (pdFAIL == freertos_ret)
                {
                    OSAL_DEBUG("Mutex internal_lock: OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_BUSY;
                }
                else
                {
                    /* Check if the block is released after the de-initialization. */
                    if (false == gs_osal_threadsync_is_initialized)
                    {
                        osal_ret = OSAL_RETURN_STATE;
                        (void)xSemaphoreGiveFromISR(handle->mtx, &pxHigherPriorityTaskWoken);
                    }
                    else
                    {
                        /* Do nothing */
                    }

                    if ((OSAL_RETURN_OK == osal_ret) && (false == handle->active))
                    {
                        osal_ret = OSAL_RETURN_HANDLE;
                        (void)xSemaphoreGiveFromISR(handle->mtx, &pxHigherPriorityTaskWoken);
                    }
                    else
                    {
                        handle->use = true;
                    }
                }
                portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
            }
        }
        else
        {
            current_task = xTaskGetCurrentTaskHandle();
            mutex_owner_task = xSemaphoreGetMutexHolder(handle->mtx);
            if ( current_task == mutex_owner_task)
            {
                OSAL_DEBUG("Mutex internal_lock: OSAL_RETURN_FAIL(task)\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                R_OSAL_Internal_MillisecToTick(wait_time, &tickcount);
                freertos_ret = xSemaphoreTake(handle->mtx, tickcount);
                if (pdFAIL == freertos_ret)
                {
                    OSAL_DEBUG("Mutex internal_lock: OSAL_RETURN_TIME\n");
                    osal_ret = OSAL_RETURN_TIME;
                }
                else
                {
                    /* Check if the block is released after the de-initialization. */
                    if ((false == gs_osal_threadsync_is_initialized) && (( OSAL_MUTEX_DRIVER_CPG != handle->usr_config->id) && ( OSAL_MUTEX_DRIVER_SYSC != handle->usr_config->id)))
                    {
                        osal_ret = OSAL_RETURN_STATE;
                        (void)xSemaphoreGive(handle->mtx);
                    }
                    else
                    {
                        /* Do nothing */
                    }

                    if ((OSAL_RETURN_OK == osal_ret) && (false == handle->active))
                    {
                        osal_ret = OSAL_RETURN_HANDLE;
                        (void)xSemaphoreGive(handle->mtx);
                    }
                    else
                    {
                        handle->use = true;
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_lock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_condwait_core()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_condwait_core(osal_cond_handle_t cond_handle, osal_mutex_handle_t mutex_handle,
                                                        st_osal_event_list_t *use_list, osal_milli_sec_t time_period)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret, osal_ret_internal;
    BaseType_t freertos_ret;
    TickType_t wait_time_tick;
    EventBits_t event_bits;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/

    /* UD.9 / 8 */
    R_OSAL_Internal_MillisecToTick(cond_handle->inner_timeout, &wait_time_tick);
    freertos_ret = xSemaphoreTake(cond_handle->inner_mtx, wait_time_tick);
    if(pdPASS != freertos_ret)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        osal_ret = R_OSAL_Internal_AddList(&cond_handle->wait_list, use_list);
        if (OSAL_RETURN_OK != osal_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        freertos_ret = xSemaphoreGive(cond_handle->inner_mtx);
        if (pdFAIL == freertos_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else if (OSAL_RETURN_OK != osal_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            ;   /* (pdTRUE == freertos_ret) && (OSAL_RETURN_OK == osal_ret) */
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        R_OSAL_Internal_MillisecToTick(time_period, &wait_time_tick);
        /* UD.11 / 9*/
        freertos_ret = xSemaphoreGive(mutex_handle->mtx);
        if (pdPASS != freertos_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
       {
            event_bits = xEventGroupWaitBits(cond_handle->event, use_list->bits, pdTRUE, pdFALSE,
                                             wait_time_tick);
            R_OSAL_Internal_MillisecToTick(cond_handle->inner_cond_timeout, &wait_time_tick);
            freertos_ret = xSemaphoreTake(mutex_handle->mtx, wait_time_tick);
            if (pdFAIL == freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            /* UD.12 / 10*/
            if ((use_list->bits & event_bits) != use_list->bits)
            {
                OSAL_DEBUG("OSAL_RETURN_TIME\n");
                osal_ret = OSAL_RETURN_TIME;
                R_OSAL_Internal_MillisecToTick(cond_handle->inner_timeout, &wait_time_tick);
                freertos_ret = xSemaphoreTake(cond_handle->inner_mtx, wait_time_tick);

                if(pdPASS != freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    (void)R_OSAL_Internal_RemoveList(&cond_handle->wait_list, use_list);
                    (void)R_OSAL_Internal_AddList(&cond_handle->empty_list, use_list);
                    freertos_ret = xSemaphoreGive(cond_handle->inner_mtx);
                    if (pdFAIL == freertos_ret)
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                }
            }
            else
            {
                R_OSAL_Internal_MillisecToTick(cond_handle->inner_timeout, &wait_time_tick);
                /* UD.13 / 11 */
                freertos_ret = xSemaphoreTake(cond_handle->inner_mtx, wait_time_tick);
                if(pdPASS != freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    /* UD.14 / 12 */
                    osal_ret_internal = R_OSAL_Internal_AddList(&cond_handle->empty_list, use_list);
                    /* UD.15 / 13 */
                    freertos_ret = xSemaphoreGive(cond_handle->inner_mtx);
                    if (pdFAIL == freertos_ret)
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else if (OSAL_RETURN_OK != osal_ret_internal)
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        ;   /* (pdTRUE == freertos_ret) && (OSAL_RETURN_OK == osal_ret) */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_condwait_core()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_condwait()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_condwait(osal_cond_handle_t cond_handle, osal_mutex_handle_t mutex_handle,
                                                   osal_milli_sec_t time_period)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    UBaseType_t lock_task;
    BaseType_t freertos_ret;
    TickType_t wait_time_tick;
    st_osal_event_list_t *use_list;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (false == gs_osal_threadsync_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == cond_handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((OSAL_COND_HANDLE_ID != cond_handle->handle_id) || (false == cond_handle->active))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        if (NULL == mutex_handle)
        {
            OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else if ((OSAL_MUTEX_HANDLE_ID != mutex_handle->handle_id) || (false == mutex_handle->active))
        {
            OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            osal_ret = OSAL_RETURN_OK;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /*This is a to the UD proc number(TimeStamp/Period) */
        /* UD.2 / 2 */
        lock_task = uxSemaphoreGetCount(mutex_handle->mtx);
        if (1U == lock_task)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            R_OSAL_Internal_MillisecToTick(cond_handle->inner_timeout, &wait_time_tick);
            /* UD.3 / 3 */
            freertos_ret = xSemaphoreTake(cond_handle->inner_mtx, wait_time_tick);
            if (pdPASS != freertos_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                /* UD.4 / 4 */
                use_list = R_OSAL_Internal_GetHeadFromList(&cond_handle->empty_list);

                /* UD.6 / 6 */
                freertos_ret = xSemaphoreGive(cond_handle->inner_mtx);
                if (pdPASS != freertos_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else if (NULL == use_list)   /* UD.5 / 5*/
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    /* UD.7 / 7 */
                    osal_ret = osal_internal_condwait_core(cond_handle, mutex_handle, use_list, time_period);

                    /* Check if the block is released after the de-initialization. */
                    if (false == gs_osal_threadsync_is_initialized)
                    {
                        osal_ret = OSAL_RETURN_STATE;
                        (void)xSemaphoreGive(mutex_handle->mtx);
                    }
                    else
                    {
                        /* Do nothing */
                    }

                    if ((OSAL_RETURN_OK == osal_ret) && (false == mutex_handle->active))
                    {
                        osal_ret = OSAL_RETURN_HANDLE;
                        (void)xSemaphoreGive(mutex_handle->mtx);
                    }
                    else
                    {
                        mutex_handle->use = true;
                    }
                }
            }
        }
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_condwait()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mutex_find_control()
* XOS1_OSAL_CD_CD_255
* [Covers: XOS1_OSAL_UD_UD_164]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_find_control(osal_mutex_id_t id, st_osal_mutex_control_t **mutex_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_mutex_control_t *control = NULL;
    st_osal_mutex_control_t *local_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_RCFG_GetNumOfMutex();

    while ((count < num_of_info) && (OSAL_RETURN_OK == osal_ret) && (NULL == local_control))
    {
        api_ret = R_OSAL_RCFG_GetMutexInfo(count, &control);
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

    *mutex_control = local_control;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mutex_find_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_mutex_set_control()
* XOS1_OSAL_CD_CD_256
* [Covers: XOS1_OSAL_UD_UD_164]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_mutex_set_control(st_osal_mutex_control_t *mutex_control)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_threadsync_is_initialized)
    {
        OSAL_DEBUG("Mutex Open: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (true == mutex_control->active)
    {
        OSAL_DEBUG("Mutex Open: OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else if (true == mutex_control->use)
    {
        OSAL_DEBUG("Mutex Open: OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else
    {
        mutex_control->active = true;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_mutex_set_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_cond_find_control()
* XOS1_OSAL_CD_CD_257
* [Covers: XOS1_OSAL_UD_UD_170]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_find_control(osal_cond_id_t id, st_osal_cond_control_t **cond_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_cond_control_t *control = NULL;
    st_osal_cond_control_t *local_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_RCFG_GetNumOfCond();

    while ((count < num_of_info) && (OSAL_RETURN_OK == osal_ret) && (NULL == local_control))
    {
        api_ret = R_OSAL_RCFG_GetCondInfo(count, &control);
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

    *cond_control = local_control;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_cond_find_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_cond_set_control()
* XOS1_OSAL_CD_CD_258
* [Covers: XOS1_OSAL_UD_UD_170]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_cond_set_control(st_osal_cond_control_t *cond_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    uint32_t count;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_threadsync_is_initialized)
    {
        OSAL_DEBUG("Mutex Open: OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (true == cond_control->active)
    {
        OSAL_DEBUG("Mutex Open: OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else
    {
        count = 0;
        cond_control->wait_list = NULL;
        cond_control->empty_list = NULL;

        while ((OSAL_MAX_EVENT_BITS > count) && (OSAL_RETURN_OK == osal_ret))
        {
            cond_control->bits_pattern[count].bits = ((EventBits_t)1U << count);
            cond_control->bits_pattern[count].next = NULL;
            osal_ret = R_OSAL_Internal_AddList(&cond_control->empty_list, &cond_control->bits_pattern[count]);

            if (OSAL_RETURN_FAIL == osal_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            count++;
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            (void)xEventGroupClearBits(cond_control->event, OSAL_EVENT_CLEAR_BITS);
            cond_control->active = true;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_cond_set_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_cond_get_events()
* XOS1_OSAL_CD_CD_259
* [Covers: XOS1_OSAL_UD_UD_174]
* [Covers: XOS1_OSAL_UD_UD_175]
***********************************************************************************************************************/
OSAL_STATIC EventBits_t osal_internal_cond_get_events(osal_cond_handle_t handle, bool flag)
{
    EventBits_t event_bits = 0;
    st_osal_event_list_t *use_list;

    do {
        use_list = R_OSAL_Internal_GetHeadFromList(&handle->wait_list);
        if (NULL != use_list)
        {
            event_bits |= use_list->bits;
        }
        else
        {
            flag = false;
        }
    } while (true == flag);

    return event_bits;
}
/***********************************************************************************************************************
* End of function osal_internal_cond_get_events()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_threadsync_initialize()
***********************************************************************************************************************/
e_osal_return_t osal_threadsync_initialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false != gs_osal_threadsync_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_threadsync_is_initialized = true;

        osal_ret = osal_cond_initialize();
        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = osal_mutex_initialize();
        }
        else
        {
            /* Do nothing */
        }

        if (OSAL_RETURN_OK != osal_ret)
        {
            (void)osal_threadsync_deinitialize();
        }
        else
        {
            /* Do nothing */
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_threadsync_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_threadsync_deinitialize()
***********************************************************************************************************************/
e_osal_return_t osal_threadsync_deinitialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_threadsync_is_initialized = false;

    osal_ret = osal_cond_deinitialize();
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_mutex_deinitialize();
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_threadsync_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_threadsync_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_threadsync_deinitialize_pre(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (true != gs_osal_threadsync_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_threadsync_is_initialized = false;

        osal_ret = osal_cond_deinitialize_pre();
        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = osal_mutex_deinitialize_pre();
        }
        else
        {
            /* Do nothing */
        }
    }

    if (OSAL_RETURN_BUSY == osal_ret)
    {
        gs_osal_threadsync_is_initialized = true;
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_threadsync_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_threadsync_set_is_init()
***********************************************************************************************************************/
void osal_threadsync_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_threadsync_is_initialized = true;

    return;
}
/***********************************************************************************************************************
* End of function osal_threadsync_set_is_init()
***********************************************************************************************************************/

