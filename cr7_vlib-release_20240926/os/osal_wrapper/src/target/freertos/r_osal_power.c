/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_power.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper Power Manager for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"

/*******************************************************************************************************************//**
 * @var gs_osal_pm_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_pm_is_initialized = false;

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Defines_PowerManager Power Manager Private macro definitions
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def OSAL_PMA_WAIT_TIME
 * PMA waiting time 1msec.
***********************************************************************************************************************/
#define OSAL_PMA_WAIT_TIME (1L)
#define OSAL_PMA_SLEEP_RETRY (1000L)

/** @} */

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Functions_PowerManager Power Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Check PM Manager handle
* @param[in]        handle  To set the handle
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_pm(osal_device_handle_t handle);

/*******************************************************************************************************************//**
* @brief            Get current Power Manager Status of specified device.
* @param[in]        pma_handle  To set the handle
* @param[in]        pma_hwaId  To set the PMA id
* @param[out]       p_state  To set the address of PMA status value.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_pm_get_state(st_pma_handle_t pma_handle, e_pma_hwa_id_t pma_hwaId,
                                                       e_osal_pm_state_t * const p_state);

/*******************************************************************************************************************//**
* @brief            Get PMA handle
* @param[out]       PMAhandle  To set the address of PMA handle
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_pma_get_handle(st_pma_handle_t *PMAhandle);

/*******************************************************************************************************************//**
* @brief            Set initilization power policy for PMA.
* @param[in]        handle  To set the handle
* @param[in]        pm_id  To set the PMA id
* @param[in]        policy     To set the power policy
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_CONF
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_set_initial_policy(st_pma_handle_t handle, e_pma_hwa_id_t pm_id,
                                                    e_osal_pm_policy_t policy);

/*******************************************************************************************************************//**
* @brief            Convert pma error to osal error.
* @param[in]        pma_ret  PMA error value
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_DEV
* @retval           OSAL_RETURN_CONF
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_pm_convert_error(e_pma_return_t pma_ret);

/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_PmSetRequiredState()
* XOS1_OSAL_CD_CD_701
* [Covers: XOS1_OSAL_UD_UD_191]
* [Covers: XOS1_OSAL_UD_UD_230]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_053]
* [Covers: XOS1_OSAL_UD_UD_135]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmSetRequiredState(osal_device_handle_t handle, e_osal_pm_required_state_t state,
                                          bool applyImmediate)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    e_pma_return_t pma_ret;
    st_pma_hwa_state_t pma_hwa_state;
    e_pma_reset_req_t  pma_reset_req;
    BaseType_t semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %d)\n", (void *)handle, (uint64_t)state, applyImmediate);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if ((OSAL_PM_REQUIRED_STATE_REQUIRED != state) &&
            (OSAL_PM_REQUIRED_STATE_RELEASED != state))
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                if (OSAL_PM_REQUIRED_STATE_REQUIRED == state)
                {
                    OSAL_DEBUG("Set Required PMID(%d)\n", handle->dev_info->pm_id);
                    pma_ret = R_PMA_RequestHwa(handle->pma_handle, handle->dev_info->pm_id);
                    pma_hwa_state.enReqState      = R_PMA_HWA_REQUESTED;
                    pma_hwa_state.enActualClkStat = R_PMA_STATE_ON;
                    pma_hwa_state.enActualPwrStat = R_PMA_STATE_ON;
                    pma_reset_req                 = R_PMA_UNDEFINED_RESET;
                }
                else /* OSAL_PM_REQUIRED_STATE_RELEASED == state*/
                {
                    OSAL_DEBUG("Set Release PMID(%d)\n", handle->dev_info->pm_id);
                    pma_ret = R_PMA_ReleaseHwa(handle->pma_handle, handle->dev_info->pm_id);
                    pma_hwa_state.enReqState      = R_PMA_HWA_RELEASED;
                    pma_hwa_state.enActualClkStat = R_PMA_UNDEFINED_STATE;
                    pma_hwa_state.enActualPwrStat = R_PMA_UNDEFINED_STATE;
                    pma_reset_req                 = R_PMA_UNDEFINED_RESET;

                }

                if (R_PMA_RESULT_OK == pma_ret)
                {
                    if (true == applyImmediate)
                    {
                        pma_ret = R_PMA_WaitForState(handle->pma_handle, handle->dev_info->pm_id, &pma_hwa_state, pma_reset_req,
                                                     OSAL_PMA_WAIT_TIME, OSAL_PMA_SLEEP_RETRY, 0);
                    }
                }

                switch (pma_ret)
                {
                    case R_PMA_RESULT_OK:
                        osal_ret = OSAL_RETURN_OK;
                        break;
                    case R_PMA_ERR_TIMEOUT:
                        osal_ret = OSAL_RETURN_TIME;
                        break;
                    case R_PMA_ERR_DEVICE:
                        osal_ret = OSAL_RETURN_DEV;
                        break;
                    default:
                        osal_ret = OSAL_RETURN_FAIL;
                        break;
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    if (OSAL_RETURN_DEV != osal_ret)  /* Not over write */
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                }
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmSetRequiredState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmGetRequiredState()
* XOS1_OSAL_CD_CD_704
* [Covers: XOS1_OSAL_UD_UD_192]
* [Covers: XOS1_OSAL_UD_UD_231]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_049]
* [Covers: XOS1_OSAL_UD_UD_136]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmGetRequiredState(osal_device_handle_t handle, e_osal_pm_required_state_t * const p_state)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_pma_return_t pma_ret;
    st_pma_hwa_state_t pma_state;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_state);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if (NULL == p_state)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                pma_ret = R_PMA_GetHwaState(handle->pma_handle, handle->dev_info->pm_id, &pma_state);

                if (R_PMA_RESULT_OK != pma_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if (R_PMA_HWA_RELEASED == pma_state.enReqState)
                    {
                        *p_state = OSAL_PM_REQUIRED_STATE_RELEASED;
                    }
                    else if(R_PMA_HWA_REQUESTED == pma_state.enReqState)
                    {
                        *p_state = OSAL_PM_REQUIRED_STATE_REQUIRED;
                    }
                    else
                    {
                        *p_state = OSAL_PM_REQUIRED_STATE_INVALID;
                    }
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u), RequiredState:%d \n", osal_ret, (int32_t)*p_state);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmGetRequiredState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmSetAndWaitForState()
* XOS1_OSAL_CD_CD_707
* [Covers: XOS1_OSAL_UD_UD_193]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_232]
* [Covers: XOS1_OSAL_UD_UD_137]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmWaitForState(osal_device_handle_t handle, e_osal_pm_state_t state,
                                            osal_milli_sec_t time_period)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_osal_pm_state_t pma_current_state;
    osal_milli_sec_t wait_time = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %u, %ld)\n", (void *)handle, (int32_t)state, time_period);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if ((OSAL_PM_STATE_PG != state) &&
            (OSAL_PM_STATE_CG != state) &&
            (OSAL_PM_STATE_ENABLED != state) &&
            (OSAL_PM_STATE_RESET != state) &&
            (OSAL_PM_STATE_READY != state))
        {
             OSAL_DEBUG("OSAL_RETURN_PAR\n");
             osal_ret = OSAL_RETURN_PAR;
        }
        else if (0 > time_period)
        {
             OSAL_DEBUG("OSAL_RETURN_PAR\n");
             osal_ret = OSAL_RETURN_PAR;
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
                api_ret = osal_internal_pm_get_state(handle->pma_handle, handle->dev_info->pm_id, &pma_current_state);
                if (OSAL_RETURN_OK != api_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    handle->pm_wait_state = true;
                    while ((wait_time < time_period) && (pma_current_state != state) && (OSAL_RETURN_OK == api_ret))
                    {
                        vTaskDelay(pdMS_TO_TICKS(OSAL_PMA_WAIT_TIME));
                        wait_time += OSAL_PMA_WAIT_TIME;
                        api_ret = osal_internal_pm_get_state(handle->pma_handle, handle->dev_info->pm_id,
                                                             &pma_current_state);
                        if (OSAL_RETURN_OK != api_ret)
                        {
                            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                            osal_ret = OSAL_RETURN_FAIL;
                        }
                    }
                    handle->pm_wait_state = false;

                    if ((wait_time >= time_period) && (OSAL_RETURN_OK == api_ret))
                    {
                        OSAL_DEBUG("OSAL_RETURN_TIME\n");
                        osal_ret = OSAL_RETURN_TIME;
                    }
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmSetAndWaitForState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmGetState()
* XOS1_OSAL_CD_CD_709
* [Covers: XOS1_OSAL_UD_UD_194]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_233]
* [Covers: XOS1_OSAL_UD_UD_051]
* [Covers: XOS1_OSAL_UD_UD_138]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmGetState(osal_device_handle_t handle, e_osal_pm_state_t * const p_state)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_osal_pm_state_t pma_current_state;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_state);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if (NULL == p_state)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                api_ret = osal_internal_pm_get_state(handle->pma_handle, handle->dev_info->pm_id, &pma_current_state);
                if (OSAL_RETURN_OK != api_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                    *p_state = OSAL_PM_STATE_INVALID;
                }
                else
                {
                    *p_state = pma_current_state;
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u), State:%d \n", osal_ret, (int32_t)*p_state);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmGetState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmGetLowestIntermediatePowerState()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmGetLowestIntermediatePowerState(osal_device_handle_t handle, e_osal_pm_state_t* const p_state)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_pma_return_t pma_ret;
    e_pma_power_state_t lowest_power;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_state);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if (NULL == p_state)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                pma_ret = R_PMA_GetLowestPowerState(handle->pma_handle, handle->dev_info->pm_id, &lowest_power);

                if (R_PMA_RESULT_OK != pma_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                    *p_state = OSAL_PM_STATE_INVALID;
                }
                else
                {
                    switch (lowest_power)
                    {
                        case R_PMA_STATE_PG:
                            *p_state = OSAL_PM_STATE_PG;
                            break;
                        case R_PMA_STATE_CG:
                            *p_state = OSAL_PM_STATE_CG;
                            break;
                        case R_PMA_STATE_ENABLED:
                            *p_state = OSAL_PM_STATE_ENABLED;
                            break;
                        default:
                            *p_state = OSAL_PM_STATE_INVALID;
                            break;
                    }
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmGetLowestIntermediatePowerState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmSetPolicy()
* XOS1_OSAL_CD_CD_412
* [Covers: XOS1_OSAL_UD_UD_195]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_234]
* [Covers: XOS1_OSAL_UD_UD_052]
* [Covers: XOS1_OSAL_UD_UD_139]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmSetPolicy(osal_device_handle_t handle, e_osal_pm_policy_t policy, bool applyImmediate)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_pma_return_t pma_ret;
    e_pma_power_policy_t pma_policy;
    e_pma_power_policy_t check_pma_policy = R_PMA_PG_MODE;
    int32_t              wait_count;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %u, %u)\n", (void *)handle, (int32_t)policy, applyImmediate);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if ((OSAL_PM_POLICY_PG != policy) &&
            (OSAL_PM_POLICY_CG != policy) &&
            (OSAL_PM_POLICY_HP != policy))
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                if (OSAL_PM_POLICY_PG == policy)
                {
                    pma_policy = R_PMA_PG_MODE;
                }
                else if(OSAL_PM_POLICY_CG == policy)
                {
                    pma_policy = R_PMA_CG_MODE;
                }
                else /*OSAL_PM_POLICY_HP == policy */
                {
                    pma_policy = R_PMA_HP_MODE;
                }

                pma_ret = R_PMA_SetPowerPolicy(handle->pma_handle, handle->dev_info->pm_id, pma_policy);

                if (R_PMA_RESULT_OK == pma_ret)
                {
                    osal_ret = OSAL_RETURN_OK;
                }
                else
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }

                if ((R_PMA_RESULT_OK == pma_ret) && (true == applyImmediate))
                {
                    for (wait_count = 0; (OSAL_RETURN_OK == osal_ret) && (OSAL_PMA_SLEEP_RETRY > wait_count); wait_count++)
                    {
                        pma_ret = R_PMA_GetPowerPolicy(handle->pma_handle, handle->dev_info->pm_id, &check_pma_policy);
                        if (pma_ret != R_PMA_RESULT_OK)
                        {
                           osal_ret = OSAL_RETURN_FAIL;
                        }
                        else if (pma_policy == check_pma_policy)
                        {
                            break;
                        }
                        else
                        {
                            vTaskDelay(pdMS_TO_TICKS(OSAL_PMA_WAIT_TIME));
                        }
                    }
                    if (OSAL_PMA_SLEEP_RETRY <= wait_count)
                    {
                        osal_ret = OSAL_RETURN_TIME;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }


                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmSetPolicy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmGetPolicy()
* XOS1_OSAL_CD_CD_715
* [Covers: XOS1_OSAL_UD_UD_196]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_235]
* [Covers: XOS1_OSAL_UD_UD_048]
* [Covers: XOS1_OSAL_UD_UD_140]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmGetPolicy(osal_device_handle_t handle, e_osal_pm_policy_t * const p_policy)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_pma_return_t pma_ret;
    e_pma_power_policy_t pma_policy;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_policy);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if (NULL == p_policy)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                pma_ret = R_PMA_GetPowerPolicy(handle->pma_handle, handle->dev_info->pm_id, &pma_policy);
                if (R_PMA_RESULT_OK != pma_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                    *p_policy = OSAL_PM_POLICY_INVALID;
                }
                else
                {
                    if (R_PMA_PG_MODE == pma_policy)
                    {
                        *p_policy = OSAL_PM_POLICY_PG;
                    }
                    else if (R_PMA_CG_MODE == pma_policy)
                    {
                        *p_policy = OSAL_PM_POLICY_CG;
                    }
                    else if (R_PMA_HP_MODE == pma_policy)
                    {
                        *p_policy = OSAL_PM_POLICY_HP;
                    }
                    else
                    {
                        *p_policy = OSAL_PM_POLICY_INVALID;
                    }
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u), Policy:%d \n", osal_ret, (int32_t)*p_policy);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmGetPolicy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmSetResetState()
* XOS1_OSAL_CD_CD_718
* [Covers: XOS1_OSAL_UD_UD_197]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_236]
* [Covers: XOS1_OSAL_UD_UD_054]
* [Covers: XOS1_OSAL_UD_UD_141]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmSetResetState(osal_device_handle_t handle, e_osal_pm_reset_state_t state)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    e_pma_return_t pma_ret;
    BaseType_t semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %u)\n", (void *)handle, (int32_t)state);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if ((OSAL_PM_RESET_STATE_APPLIED != state) &&
            (OSAL_PM_RESET_STATE_RELEASED != state))
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                if (OSAL_PM_RESET_STATE_APPLIED == state)
                {
                    OSAL_DEBUG("Set PMID(%d)'s  Apply Reset\n", handle->dev_info->pm_id);
                    pma_ret = R_PMA_ApplyResetHwa(handle->pma_handle, handle->dev_info->pm_id);
                }
                else /* (OSAL_PM_RESET_STATE_RELEASED == state) */
                {
                    OSAL_DEBUG("Set PMID(%d)'s  Release Reset\n", handle->dev_info->pm_id);
                    pma_ret = R_PMA_ReleaseResetHwa(handle->pma_handle, handle->dev_info->pm_id);
                }

                if (R_PMA_RESULT_OK == pma_ret)
                {
                    osal_ret = OSAL_RETURN_OK;
                }
                else if(R_PMA_ERR_RESET_CHK_NG == pma_ret)
                {
                    osal_ret = OSAL_RETURN_DEV;
                }
                else
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    if (OSAL_RETURN_DEV != osal_ret)  /* Not over write */
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                }
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmSetResetState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmGetResetState()
* XOS1_OSAL_CD_CD_721
* [Covers: XOS1_OSAL_UD_UD_198]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_237]
* [Covers: XOS1_OSAL_UD_UD_050]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_PmGetResetState(osal_device_handle_t handle, e_osal_pm_reset_state_t * const p_state)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    BaseType_t semaphore_ret;
    e_pma_return_t pma_ret;
    e_pma_reset_req_t pma_reset_state;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_state);

    api_ret = osal_check_pm(handle);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = api_ret;
    }
    else
    {
        if (NULL == p_state)
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
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
                pma_ret = R_PMA_GetReset(handle->pma_handle, handle->dev_info->pm_id, &pma_reset_state);

                if (R_PMA_RESULT_OK != pma_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if (R_PMA_RELEASED_RESET == pma_reset_state)
                    {
                        *p_state = OSAL_PM_RESET_STATE_RELEASED;
                    }
                    else if(R_PMA_APPLIED_RESET == pma_reset_state)
                    {
                        *p_state = OSAL_PM_RESET_STATE_APPLIED;
                    }
                    else
                    {
                        *p_state = OSAL_PM_RESET_STATE_INVALID;
                    }
                }

                semaphore_ret = xSemaphoreGive(handle->inner_mtx);
                if (pdFAIL == semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    OSAL_DEBUG("End (%u), ResetState:%d \n", osal_ret, (int32_t)*p_state);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_PmGetResetState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_PmSetPostClock()
***********************************************************************************************************************/

e_osal_return_t R_OSAL_PmSetPostClock(osal_device_handle_t device, bool clock_enable, uint32_t clock_divider)
{
    return OSAL_RETURN_OK;
}

/***********************************************************************************************************************
* End of function R_OSAL_PmSetPostClock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_check_pm()
* XOS1_OSAL_CD_CD_730
* [Covers: XOS1_OSAL_UD_UD_191]
* [Covers: XOS1_OSAL_UD_UD_192]
* [Covers: XOS1_OSAL_UD_UD_193]
* [Covers: XOS1_OSAL_UD_UD_194]
* [Covers: XOS1_OSAL_UD_UD_195]
* [Covers: XOS1_OSAL_UD_UD_196]
* [Covers: XOS1_OSAL_UD_UD_197]
* [Covers: XOS1_OSAL_UD_UD_198]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_pm(osal_device_handle_t handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_pm_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((false == handle->active) || (OSAL_DEVICE_HANDLE_ID != handle->handle_id) || (NULL == handle->dev_info))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == handle->pma_handle)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        osal_ret = OSAL_RETURN_OK;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_pm()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_pm_get_state()
* XOS1_OSAL_CD_CD_733
* [Covers: XOS1_OSAL_UD_UD_193]
* [Covers: XOS1_OSAL_UD_UD_194]
* [Covers: XOS1_OSAL_UD_UD_319]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_pm_get_state(st_pma_handle_t pma_handle, e_pma_hwa_id_t pma_hwaId,
                                                       e_osal_pm_state_t * const p_state)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t ret = OSAL_RETURN_OK;
    e_pma_return_t pma_ret;
    st_pma_hwa_state_t pma_state;
    e_pma_reset_req_t pma_reset_state;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_state)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        ret = OSAL_RETURN_PAR;
    }
    else
    {
        pma_ret = R_PMA_GetHwaState(pma_handle, pma_hwaId, &pma_state);
        if (R_PMA_RESULT_OK != pma_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            ret = OSAL_RETURN_FAIL;
            *p_state = OSAL_PM_STATE_INVALID;
        }
        else
        {
            if (R_PMA_HWA_RELEASED == pma_state.enReqState)
            {
                if (R_PMA_STATE_ON == pma_state.enActualPwrStat)
                {
                    if(R_PMA_STATE_ON == pma_state.enActualClkStat)
                    {
                        *p_state = OSAL_PM_STATE_ENABLED;
                    }
                    else
                    {
                        *p_state = OSAL_PM_STATE_CG;
                    }
                }
                else 
                {
                    if(R_PMA_STATE_ON == pma_state.enActualClkStat)
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        *p_state = OSAL_PM_STATE_INVALID;
                        ret = OSAL_RETURN_FAIL;  /* This case maybe not happen */
                    }
                    else
                    {
                        *p_state = OSAL_PM_STATE_PG;
                    }
                }
            }
            else
            {
                pma_ret = R_PMA_GetReset(pma_handle, pma_hwaId, &pma_reset_state);
                if (R_PMA_RESULT_OK == pma_ret)
                {
                    if (R_PMA_RELEASED_RESET == pma_reset_state)
                    {
                        *p_state = OSAL_PM_STATE_READY;
                    }
                    else
                    {
                        *p_state = OSAL_PM_STATE_RESET;
                    }
                }
                else
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    *p_state = OSAL_PM_STATE_INVALID;
                    ret = OSAL_RETURN_FAIL;
                }
            }
        }
    }

    return ret;
}
/***********************************************************************************************************************
* End of function osal_internal_pm_get_state()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_pm_initialize()
* XOS1_OSAL_CD_CD_723
* [Covers: XOS1_OSAL_UD_UD_199]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_021]
* [Covers: XOS1_OSAL_UD_UD_153]
* [Covers: XOS1_OSAL_UD_UD_206]
* [Covers: XOS1_OSAL_UD_UD_154]
* [Covers: XOS1_OSAL_UD_UD_020]
***********************************************************************************************************************/
e_osal_return_t osal_pm_initialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_device_control_t *control = NULL;
    st_pma_handle_t PMAhandle;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (true == gs_osal_pm_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_pm_is_initialized = true;
        osal_ret = osal_pma_get_handle(&PMAhandle);
        OSAL_DEBUG("PMAhandle (%p)\n", (void *)PMAhandle);
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        num_of_info = R_OSAL_DCFG_GetNumOfDevice();

        while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
        {
            api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
            if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->dev_info))
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                /* control->init_state is always true */
                /* because this function call after osal_io_initialize */
                /* So don't check init_state */

                control->pma_handle = NULL;
                if (OSAL_PMA_ID_NONE != (uint32_t)control->dev_info->pm_id)
                {
                    api_ret = osal_set_initial_policy(PMAhandle, control->dev_info->pm_id,
                                                      control->dev_info->initial_policy);

                    if (OSAL_RETURN_OK == api_ret)
                    {
                        /* control initialize*/
                        control->pma_handle = PMAhandle;
                    }
                    else
                    {
                        OSAL_DEBUG("Set initial policy error(%d)\n", api_ret);
                        osal_ret = api_ret;
                    }
                }
            }
            count++;
        }
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_pm_deinitialize();
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_pm_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_pm_deinitialize()
* XOS1_OSAL_CD_CD_729
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_008]
* [Covers: XOS1_OSAL_UD_UD_208]
* [Covers: XOS1_OSAL_UD_UD_332]
***********************************************************************************************************************/
e_osal_return_t osal_pm_deinitialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_device_control_t *control = NULL;
    e_pma_return_t pma_ret;
    st_pma_handle_t pma_handle = NULL;
 
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_pm_is_initialized = false;
    
    num_of_info = R_OSAL_DCFG_GetNumOfDevice();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* control->init_state is always true */
            /* because this function call after osal_io_deinitialize */
            /* So don't check init_state */

            if (NULL != control->pma_handle)
            {
                pma_handle = control->pma_handle;
                (void)R_PMA_UnlockHwa(control->pma_handle, control->dev_info->pm_id);
            }
            control->pma_handle = NULL;
        }
        count++;
    }

    if ((OSAL_RETURN_OK == osal_ret) && (NULL != pma_handle))
    {
        pma_ret = R_PMA_Quit(pma_handle);

        if ((R_PMA_ERR_INVALID_HANDLE == pma_ret) ||
            (R_PMA_ERR_INVALID_ARGUMENT == pma_ret) ||
            (R_PMA_ERR_RESULT_NG == pma_ret))
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    OSAL_DEBUG("End\n");

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_pm_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_pm_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_pm_deinitialize_pre(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_device_control_t *control = NULL;
    BaseType_t semaphore_ret;

    if (false == gs_osal_pm_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_pm_is_initialized = false;

        num_of_info = R_OSAL_DCFG_GetNumOfDevice();
        while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
        {
            api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
            if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->dev_info))
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(control->inner_mtx, pdMS_TO_TICKS(control->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if (true == control->pm_wait_state)
                    {
                        osal_ret = OSAL_RETURN_BUSY;
                    }
                    else
                    {
                    }

                    semaphore_ret = xSemaphoreGive(control->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        if (OSAL_RETURN_BUSY != osal_ret)  /* Not over write */
                        {
                            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                            osal_ret = OSAL_RETURN_FAIL;
                        }
                    }
                }
            }
            count++;
        }
    }

    if (OSAL_RETURN_BUSY == osal_ret)
    {
        gs_osal_pm_is_initialized = true;
    }
    else
    {
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_pm_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_pm_set_is_init()
***********************************************************************************************************************/
void osal_pm_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_pm_is_initialized = true;

    return;
}
/***********************************************************************************************************************
* End of function osal_pm_set_is_init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_pma_get_handle()
* XOS1_OSAL_CD_CD_731
* [Covers: XOS1_OSAL_UD_UD_199]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_pma_get_handle(st_pma_handle_t *PMAhandle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_pma_return_t pma_ret;
    st_pma_config_t pma_config;
    e_osal_return_t api_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    api_ret = R_OSAL_DCFG_GetPMAConfiguration(&pma_config);
    if (OSAL_RETURN_OK != api_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        pma_ret = R_PMA_Init(PMAhandle, &pma_config);
        switch (pma_ret)
        {
            case R_PMA_RESULT_OK:
                osal_ret = OSAL_RETURN_OK;
                break;
            case R_PMA_ERR_RESULT_NG:
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
                break;
            case R_PMA_ERR_INVALID_ARGUMENT:
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
                break;
            case R_PMA_ERR_NO_VACANT_HANDLE:
                OSAL_DEBUG("OSAL_RETURN_STATE\n");
                osal_ret = OSAL_RETURN_STATE;
                break;
            case R_PMA_ERR_RESET_TOGGLE_CHK_NG:
                OSAL_DEBUG("OSAL_RETURN_DEV\n");
                osal_ret = OSAL_RETURN_DEV;
                break;
            default:
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
                break;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_pma_get_handle()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_set_initail_policy()
* XOS1_OSAL_CD_CD_734
* [Covers: XOS1_OSAL_UD_UD_199]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_set_initial_policy(st_pma_handle_t handle, e_pma_hwa_id_t pm_id,
                                                    e_osal_pm_policy_t policy)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_pma_return_t pma_ret;
    e_pma_power_policy_t pma_policy;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_PM_POLICY_PG == policy)
    {
        pma_policy = R_PMA_PG_MODE;
    }
    else if (OSAL_PM_POLICY_CG == policy)
    {
        pma_policy = R_PMA_CG_MODE;
    }
    else if (OSAL_PM_POLICY_HP == policy)
    {
        pma_policy = R_PMA_HP_MODE;
    }
    else
    {
        OSAL_DEBUG("OSAL_RETURN_CONF\n");
        osal_ret = OSAL_RETURN_CONF;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        pma_ret = R_PMA_GetLockHwa(handle, pm_id);
        osal_ret = osal_pm_convert_error(pma_ret);
    }
    
    if (OSAL_RETURN_OK == osal_ret)
    {
        pma_ret = R_PMA_SetPowerPolicy(handle, pm_id, pma_policy);
        osal_ret = osal_pm_convert_error(pma_ret);

        if (OSAL_RETURN_OK == osal_ret)
        {
            pma_ret = R_PMA_RequestHwa(handle, pm_id);
            osal_ret = osal_pm_convert_error(pma_ret);
        }

        pma_ret = R_PMA_UnlockHwa(handle, pm_id);
        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = osal_pm_convert_error(pma_ret);
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_set_initail_policy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_pm_convert_error()
* XOS1_OSAL_CD_CD_732
* [Covers: XOS1_OSAL_UD_UD_199]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_pm_convert_error(e_pma_return_t pma_ret)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    switch (pma_ret)
    {
        case R_PMA_RESULT_OK:
            osal_ret = OSAL_RETURN_OK;
            break;
        case R_PMA_ERR_BUS_IF_CHK_NG:
            OSAL_DEBUG("OSAL_RETURN_DEV\n");
            osal_ret = OSAL_RETURN_DEV;
            break;
        case R_PMA_ERR_INVALID_ARGUMENT:
            OSAL_DEBUG("OSAL_RETURN_CONF\n");
            osal_ret = OSAL_RETURN_CONF;
            break;
        default:
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
            break;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_pm_convert_error()
***********************************************************************************************************************/

