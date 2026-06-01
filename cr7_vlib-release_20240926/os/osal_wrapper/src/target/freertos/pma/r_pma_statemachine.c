/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022]  Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : Internal function for PM API
***********************************************************************************************************************/

/*======================================================================================================================
Includes <System Includes> , "Project Includes"
======================================================================================================================*/
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <r_cpg_api.h>
#include <r_sysc_api.h>

#include "target/freertos/pma/r_pma_common.h"
#include "target/freertos/pma/r_pma_statemachine.h"
#include "target/freertos/pma/r_pma_hwdepend.h"

#include "target/freertos/r_osal_common.h"

/*======================================================================================================================
Private macro definitions
======================================================================================================================*/
#define R_PMA_SM_TASK_STACK_SIZE  (4096U)            /** equivalent to configMINIMAL_STACK_SIZE */
/** #define R_PMA_SM_QUEUE_LENGTH     R_PMA_HWA_ID_NUM*/
#define R_PMA_SM_QUEUE_LENGTH     (64U)
#define R_PMA_SM_QUEUE_ITEM_SIZE  sizeof(st_msg_to_task_t)

/* PRQA S 3472 2 # To reduce function overhead */
#define R_PMA_SM_SET_BIT64(bit) (powNotifyFlag |=  (1ULL << (uint32_t)(bit)))
#define R_PMA_SM_DEL_BIT64(bit) (powNotifyFlag &= ~(1ULL << (uint32_t)(bit)))

/**#define R_PMA_COMMON_CLK_POSTCKCR   (4U)*/
/**#define R_PMA_COMMON_CLK_DIVIDER    (64U)*/

#define R_PMA_WAIT_TICK (1000U)

/*======================================================================================================================
Private data types
======================================================================================================================*/
/*======================================================================================================================
Private function prototypes
======================================================================================================================*/
static e_result_t R_PMA_SM_SetupTask(R_PMA_PRIORITY taskPriority);
static e_result_t R_PMA_SM_SetupQueue(R_PMA_QUE_HANDLE *pTaskQueue);
static e_result_t R_PMA_SM_CheckUserMatchHwa(st_pma_handle_t handle, const st_hwa_t *pHwa);

static e_result_t R_PMA_SM_RequestHWA(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
static e_result_t R_PMA_SM_ReleaseHWA(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
static e_result_t R_PMA_SM_ApplyResetHWA(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
static e_result_t R_PMA_SM_ReleaseResetHWA(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_result_t R_PMA_SM_ChangePolicyHWA(st_pma_handle_t handle, e_pma_hwa_id_t  hwaId, e_pma_power_policy_t policy);
static e_result_t R_PMA_SM_UpdateLowestPowerState(st_pma_handle_t handle, e_pma_hwa_id_t hwaid);
static e_result_t R_PMA_SM_CheckDepState(st_pma_handle_t handle, uint64_t mask);

static e_result_t R_PMA_SM_PolicyToStatus(e_pma_hwa_id_t  hwaId, const st_hwa_t * pHwa, e_pma_power_policy_t policy);
static e_result_t R_PMA_SM_PolicyToStatusWithoutPower(e_pma_hwa_id_t  hwaId, const st_hwa_t * pHwa, e_pma_power_policy_t policy);
static void R_PMA_SM_Task(void *pvParameters);
/*======================================================================================================================
Private global variables
======================================================================================================================*/

R_PMA_QUE_HANDLE R_PMA_SM_GxTaskQueue;
R_PMA_TSK_HANDLE R_PMA_SM_GxThread;
R_PMA_SEM_HANDLE R_PMA_SM_GxMutex;
static R_PMA_TIMEOUT_T R_PMA_SM_GxWaitTick;

static uint64_t powNotifyFlag;

uint64_t pma_soc_type;
e_pma_hwa_id_t hwa_idmax;
e_pma_hwa_id_t hwa_idmin;
static e_pma_hwa_id_t hwa_idimpr;
e_cpg_domain_id_t cpg_idmax;
e_cpg_domain_id_t cpg_idmin;
static e_cpg_domain_id_t cpg_idimpr;
e_sysc_domain_id_t sysc_idmax;
e_sysc_domain_id_t sysc_idmin;

extern int32_t *R_PMA_CPG_MOD_ID_TABLE;
extern int32_t *R_PMA_SYSC_PDR_ID_TABLE;
extern uint64_t *sysc_mask_tbl;
extern st_hwa_func_tbl_t *R_PMA_SM_GstRegStatChangeFuncTbl;

/***********************************************************************************************************************
* Start of function R_PMA_SM_Init()
***********************************************************************************************************************/
e_result_t R_PMA_SM_Init(R_PMA_QUE_HANDLE     *pTaskQueue,
                         const st_pma_param_t *pParam)
{
    e_result_t LenResult;

    if ((NULL != pTaskQueue)
       && (NULL != pParam))
    {
        LenResult = R_PMA_SM_SetupQueue(pTaskQueue);
        R_PMA_SM_GxWaitTick = (R_PMA_TIMEOUT_T)pParam->timeout_ticks;
    }
    else
    {
        LenResult = R_PMA_NG;
    }
    if (R_PMA_OK == LenResult)
    {
        pma_soc_type = R_OSAL_DCFG_GetSocType();
        switch (pma_soc_type)
        {
            case OSAL_DEVICE_SOC_TYPE_E3:
            case OSAL_DEVICE_SOC_TYPE_H3:
            case OSAL_DEVICE_SOC_TYPE_M3:
            case OSAL_DEVICE_SOC_TYPE_M3N:
                hwa_idmax  = R_PMA_HWA_ID_NUM;
                hwa_idmin  = R_PMA_HWA_ID_IVDP1C;
                hwa_idimpr = R_PMA_HWA_ID_INVALID;
                cpg_idmax  = R_PMA_CPG_DOMAIN_ID_NUM;
                cpg_idmin  = R_PMA_CPG_DOMAIN_ID_IVDP1C;
                cpg_idimpr = R_PMA_CPG_DOMAIN_ID_INVALID;
                sysc_idmax = R_PMA_SYSC_DOMAIN_ID_NUM;
                sysc_idmin = R_PMA_SYSC_DOMAIN_ID_A3IR;
                break;
            default:
                LenResult = R_PMA_NG;
        }
    }
    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_InitSyscDomain();
    }
    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_InitCpgDomain();
    }
    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_InitHwa();
    }
    if (R_PMA_OK == LenResult)
    {
        R_PMA_SYSC_Init();
        R_PMA_CPG_Init();
    }
    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_HwStateOff();
    }
    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_SetupTask(pParam->task_priority);/* PRQA S 4442  #No problem because it is an enum and an integer cast */
    }
    /* PRQA S 2880,2992,2996 -- */
    return LenResult;
/* PRQA S 9104,9107,9109,9110 3 # The initialization function has already multiple sub-functions to reduce complexity.
                             We have decided that further division is not desirable from perspective of maintainability and readability.
*/
}
/***********************************************************************************************************************
* End of function R_PMA_SM_Init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetupTask()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_SetupTask(R_PMA_PRIORITY taskPriority)
{
    e_result_t   LenResult;
    e_osal_return_t LenOsalRet;
    st_osal_inner_thread_attrinfo_t osal_inner_thread_attrinfo;
    osal_inner_thread_attr_t osal_inner_thread_attr;
    st_osal_inner_thread_config_t osal_inner_thread_config;

    osal_inner_thread_attrinfo.policy = OSAL_SCHED_FIFO;
    osal_inner_thread_attrinfo.priority = taskPriority;
    osal_inner_thread_attrinfo.stack_size = R_PMA_SM_TASK_STACK_SIZE;;

    LenOsalRet = R_OSAL_Internal_SetThreadAttr(&osal_inner_thread_attrinfo, &osal_inner_thread_attr);
    if (OSAL_RETURN_OK == LenOsalRet)
    {
        osal_inner_thread_config.func = R_PMA_SM_Task;
        osal_inner_thread_config.attr = &osal_inner_thread_attr;
        osal_inner_thread_config.userarg = NULL;
        LenOsalRet = R_OSAL_Internal_ThreadCreate(&osal_inner_thread_config, &R_PMA_SM_GxThread);
    }

    if (LenOsalRet == OSAL_RETURN_OK)
    {
        LenResult = R_PMA_OK;
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetupTask()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetupQueue()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_SetupQueue(R_PMA_QUE_HANDLE *pTaskQueue)
{
    e_result_t LenResult = R_PMA_OK;
    e_osal_return_t LenOsalRet;
    st_osal_inner_mq_config_t osal_inner_mq_config;

    osal_inner_mq_config.max_num_msg = R_PMA_SM_QUEUE_LENGTH;
    osal_inner_mq_config.msg_size = R_PMA_SM_QUEUE_ITEM_SIZE;

    if (NULL != pTaskQueue)
    {
        LenOsalRet = R_OSAL_Internal_MqCreate(&osal_inner_mq_config, &R_PMA_SM_GxTaskQueue);
        if ((NULL != R_PMA_SM_GxTaskQueue) && (LenOsalRet == OSAL_RETURN_OK))
        {
            *pTaskQueue = R_PMA_SM_GxTaskQueue;
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetupQueue()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_Task()
***********************************************************************************************************************/
static void R_PMA_SM_Task(void *pvParameters) /* PRQA S 3673 # Required for threading */
{
    static e_result_t LenResult;
    e_osal_return_t  LenOsalRet;
    st_msg_to_task_t LstMsgToTask;
    bool             LbIsTerminate = false;

    (void)pvParameters; /** this parameter is not used */

    while (true)
    {
        LenResult  = R_PMA_OK;
        /* PRQA S 0314 1 # The 1st parameter must be (void *) and there is no side side-effect in this case. */
        (void)memset((void*)&LstMsgToTask, 0, sizeof(st_msg_to_task_t));

        /* PRQA S 0315 1 # The 3rd parameter must be (void *) and there is no side side-effect in this case. */
        LenOsalRet = R_OSAL_Internal_MqTimedReceive(R_PMA_SM_GxTaskQueue, (osal_milli_sec_t)1000, &LstMsgToTask, sizeof(st_msg_to_task_t));
        if (LenOsalRet == OSAL_RETURN_OK)
        {
            LenOsalRet = R_OSAL_Internal_MutexTimedLock(R_PMA_SM_GxMutex, R_PMA_SM_GxWaitTick);
            if (LenOsalRet == OSAL_RETURN_OK)
            {
                switch (LstMsgToTask.enProcId)
                {
                    case R_PMA_PROCESS_ID_REQUEST_HWA:
                        LenResult = R_PMA_SM_RequestHWA(LstMsgToTask.stPmaHandle, LstMsgToTask.enHwaId);
                        break;

                    case R_PMA_PROCESS_ID_RELEASE_HWA:
                        LenResult = R_PMA_SM_ReleaseHWA(LstMsgToTask.stPmaHandle, LstMsgToTask.enHwaId);
                        break;

                    case R_PMA_PROCESS_ID_APPLY_RESET:
                        LenResult = R_PMA_SM_ApplyResetHWA(LstMsgToTask.stPmaHandle, LstMsgToTask.enHwaId);
                        break;

                    case R_PMA_PROCESS_ID_RELEASE_RESET:
                        LenResult = R_PMA_SM_ReleaseResetHWA(LstMsgToTask.stPmaHandle, LstMsgToTask.enHwaId);
                        break;

                    case R_PMA_PROCESS_ID_CHANGE_POLICY:
                        LenResult = R_PMA_SM_ChangePolicyHWA(LstMsgToTask.stPmaHandle, LstMsgToTask.enHwaId, LstMsgToTask.enPolicy);
                        break;

                    case R_PMA_PROCESS_ID_EXIT_THREAD:
                        LbIsTerminate = true;
                        break;

                    default:
                        LenResult = R_PMA_NG;
                        break;
                }

                LenOsalRet = R_OSAL_Internal_MutexUnlock(R_PMA_SM_GxMutex);

                if (LenOsalRet != OSAL_RETURN_OK)
                {
                    LenResult = R_PMA_NG;
                }
                if (true ==LbIsTerminate)
                {
                    (void)xTaskNotify(LstMsgToTask.xUserTaskHandle, (uint32_t)LenResult, eSetValueWithOverwrite);
                    break;
                }
            }
            else
            {
                /* PRQA S 2982 1 # It is not a problem because it is used at the point where I am commenting now */
                LenResult = R_PMA_TIMEOUT;
            }
            (void)xTaskNotify(LstMsgToTask.xUserTaskHandle, (uint32_t)LenResult, eSetValueWithOverwrite);
        }
/* PRQA S 2053 11 # Advance start for future updates */
/**
        else if (LenOsalRet !=OSAL_RETURN_TIME)
        {
            printf("R_OSAL_Internal_MqTimedReceive NG : %d\n", LenOsalRet);
            fflush(0);
        }
        else
        {
            printf("MqTimeOut \n");
            fflush(0);
        }
*/

    }

    vTaskDelete(NULL);
    (void)LenResult;
//    return (void*)&LenResult;   /* PRQA S 0314 #This is an output to OSAL wrapper */
} /* PRQA S 9107 # Since it is a task process, it is necessary to call multiple functions separately */
/***********************************************************************************************************************
* End of function R_PMA_SM_Task()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_CheckDepState()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_CheckDepState(st_pma_handle_t handle, uint64_t mask)
{
    e_result_t LenResult = R_PMA_OK;
    uint32_t   Luicount  = 0U;

    for(; 64U > Luicount; Luicount++)
    {
        if (0U != ((powNotifyFlag & mask) & (uint64_t)(1ULL << Luicount)))
        {
            /* PRQA S 4424,4342 1 # No problem because it is an enum and an integer cast */
            LenResult = R_PMA_SM_UpdateLowestPowerState(handle, (e_pma_hwa_id_t)Luicount);
            if (R_PMA_OK != LenResult)
            {
                break;
            }
        }
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_CheckDepState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_UpdateLowestPowerState()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_UpdateLowestPowerState(st_pma_handle_t handle, e_pma_hwa_id_t hwaid)
{
    st_pma_hwa_state_t HwaState;
    st_hwa_t * LpHwa;
    e_result_t LenResult;
    e_result_t LenLockResult;

    LpHwa = R_PMA_SM_GetHwa(hwaid);
    if (LpHwa!=NULL)
    {
        LenLockResult = R_PMA_SM_GetLockHwa(handle, hwaid);
        if ((R_PMA_OK == LenLockResult) || (R_PMA_ALREADY_IN_USE == LenLockResult))
        {
            LenResult = R_PMA_SM_GetHwaState(handle, hwaid, &HwaState);
            if (R_PMA_OK == LenLockResult)
            {
                LenLockResult = R_PMA_SM_UnlockHwa(handle, hwaid);
            }
            if ((R_PMA_OK == LenResult) && ((R_PMA_OK == LenLockResult) || (R_PMA_ALREADY_IN_USE == LenLockResult)))
            {
                /** HP clock ON && Power ON */
                if ((HwaState.enActualClkStat == R_PMA_STATE_ON) && (HwaState.enActualPwrStat == R_PMA_STATE_ON))
                {
                    /** do nothing */
                }
                /** CG clock OFF && power ON */
                else if ((HwaState.enActualClkStat == R_PMA_STATE_OFF) && (HwaState.enActualPwrStat == R_PMA_STATE_ON))
                {
                    if (R_PMA_STATE_ENABLED == LpHwa->enLowestPwrState)
                    {
                        LpHwa->enLowestPwrState = R_PMA_STATE_CG;
                    }
                }
                /** PG clock OFF && power OFF */
                else if ((HwaState.enActualClkStat == R_PMA_STATE_OFF) && (HwaState.enActualPwrStat == R_PMA_STATE_OFF))
                {
                    if (R_PMA_STATE_PG != LpHwa->enLowestPwrState)
                    {
                        LpHwa->enLowestPwrState = R_PMA_STATE_PG;
                    }
                }
                else
                {
                    /** clock ON && Power OFF */
                    /** error */
                    LenResult = R_PMA_NG;
                }
            }
            else
            {
                LenResult = R_PMA_NG;
            }
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }
    return LenResult;
}/* PRQA S 9110 # It is a series of processes and is necessary so as not to reduce the readability based on design */
/***********************************************************************************************************************
* End of function R_PMA_SM_UpdateLowestPowerState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetLowestPowerState()
***********************************************************************************************************************/
/* PRQA S 1505 2 # Designed as an internal common function, not a static function in anticipation of future expansion */
/* PRQA S 3408 1 # Because the SoC-dependent processing is divided by the file, it cannot be avoided */
e_result_t R_PMA_SM_SetLowestPowerState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId)
{
    e_result_t LenResult;
    st_pma_hwa_state_t HwaState;
    st_hwa_t * LpHwa;

    LpHwa = R_PMA_SM_GetHwa(hwaId);
    LenResult = R_PMA_SM_GetHwaState(handle, hwaId, &HwaState);
    if (R_PMA_OK == LenResult)
    {
        if (HwaState.enActualPwrStat == R_PMA_STATE_ON)
        {
            if (HwaState.enActualClkStat == R_PMA_STATE_ON)
            {
                LpHwa->enLowestPwrState = R_PMA_STATE_ENABLED;
            }
            else
            {
                LpHwa->enLowestPwrState = R_PMA_STATE_CG;
            }
        }
        else
        {
            if (HwaState.enActualClkStat == R_PMA_STATE_OFF)
            {
                LpHwa->enLowestPwrState = R_PMA_STATE_PG;
            }
            else
            {
                /** error */
                LenResult = R_PMA_NG;
            }
        }

    }
    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetLowestPowerState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetLockHwa()
***********************************************************************************************************************/
e_result_t R_PMA_SM_GetLockHwa(st_pma_handle_t handle,
                               e_pma_hwa_id_t  hwaId)
{
    e_result_t LenResult = R_PMA_OK;
    st_hwa_t * LpHwa;

    if ((NULL != handle)
       && (hwa_idmax > hwaId))
    {
        /** do nothing */
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    if (R_PMA_OK == LenResult)
    {
        LpHwa = R_PMA_SM_GetHwa(hwaId);

        if (NULL == LpHwa->handle)
        {
            LpHwa->handle = handle;
        }
        else
        {
            LenResult = R_PMA_ALREADY_IN_USE;
        }
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetLockHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_UnlockHwa()
***********************************************************************************************************************/
e_result_t R_PMA_SM_UnlockHwa(st_pma_handle_t handle,
                              e_pma_hwa_id_t  hwaId)
{
    e_result_t LenResult;
    st_hwa_t * LpHwa;

    if ((NULL != handle)
       && (hwa_idmax > hwaId))
    {
        LpHwa     = R_PMA_SM_GetHwa(hwaId);
        LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

        if (R_PMA_OK == LenResult)
        {
            LpHwa->handle = NULL;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_UnlockHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetHwaState()
***********************************************************************************************************************/
e_result_t R_PMA_SM_GetHwaState(st_pma_handle_t     handle,
                                e_pma_hwa_id_t      hwaId,
                                st_pma_hwa_state_t *pHwaState)
{
    e_result_t       LenResult;
    const st_hwa_t * LpHwa;
    /* PRQA S 5209 3 #This is an output from BSP */
    r_cpg_Error_t   cpgRet;
    r_sysc_Error_t  pwrRet = R_SYSC_ERR_SUCCESS;
    bool             clkStat;
    uint32_t         pwrStat = 0;


    if ((NULL != handle)
         && (hwa_idmax > hwaId)
         && (NULL != pHwaState))
    {
        LpHwa     = R_PMA_SM_GetHwa(hwaId);
        LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);


        if (R_PMA_OK == LenResult)
        {
            cpgRet = R_PMA_CPG_GetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[LpHwa->pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[LpHwa->pCpgDomain->id]), &clkStat);
            if ((NULL != LpHwa->pSyscDomain)
                 && (R_CPG_ERR_SUCCESS == cpgRet))
            {
                /*
                 * Function power_is_off used to check the current power state of a module
                 *
                 * Input Parameters:
                 *     int pdr_number: Power Domain number
                 *
                 * Return Value :
                 *     1 - Power is OFF
                 *     0 - Power is ON
                 *  < 0 - Failure
                 */
                pwrRet = R_PMA_SYSC_GetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[LpHwa->pSyscDomain->id], false, &pwrStat);
            }
            else
            {
                /** do nothing */
            }

            if ((R_CPG_ERR_SUCCESS == cpgRet) && (R_SYSC_ERR_SUCCESS == pwrRet))
            {
                pHwaState->enReqState      = LpHwa->enReqStat;
                /* PRQA S 5209 2 # This is an output from BSP */
                pHwaState->enActualPwrStat = (1 == pwrStat) ? R_PMA_STATE_OFF : R_PMA_STATE_ON;
                pHwaState->enActualClkStat = (clkStat) ? R_PMA_STATE_OFF : R_PMA_STATE_ON;
            }
            else
            {
                LenResult = R_PMA_NG;
            }
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetHwaState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetResetState()
***********************************************************************************************************************/
e_result_t R_PMA_SM_GetResetState(st_pma_handle_t    handle,
                                  e_pma_hwa_id_t     hwaId,
                                  e_pma_reset_req_t *pRstState)
{
    e_result_t LenResult;
    const st_hwa_t * LpHwa;


    if ((NULL != handle)
       && (hwa_idmax > hwaId)
       && (NULL != pRstState))
    {
        LpHwa     = R_PMA_SM_GetHwa(hwaId);
        LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

        if (R_PMA_OK == LenResult)
        {
            *pRstState = LpHwa->pCpgDomain->enRstStat;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetResetState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetPowerPolicy()
***********************************************************************************************************************/
/* PRQA S 1505 1 # No problem because it is possible to be called from other modules based on design for future update*/
e_result_t R_PMA_SM_SetPowerPolicy(st_pma_handle_t      handle,
                                   e_pma_hwa_id_t       hwaId,
                                   e_pma_power_policy_t policy)
{
    e_result_t LenResult;
    st_hwa_t * LpHwa;

    if ((NULL != handle)
       && (hwa_idmax > hwaId)
       && ((R_PMA_PG_MODE == policy) || (R_PMA_CG_MODE == policy) || (R_PMA_HP_MODE == policy)))
    {
        LpHwa     = R_PMA_SM_GetHwa(hwaId);
        LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

        if (R_PMA_OK == LenResult)
        {
            if ((NULL == LpHwa->pSyscDomain)
               && (R_PMA_PG_MODE == policy))
            {
                LpHwa->enPwrPolicy = R_PMA_CG_MODE;
            }
            else
            {
                LpHwa->enPwrPolicy = policy;
            }
            LpHwa->enApparentMode = policy;

            if (LpHwa->enReqStat != R_PMA_HWA_REQUESTED)
            {
                LpHwa->enPrevStbMode = LpHwa->enPwrPolicy;
            }
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetPowerPolicy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetPowerPolicy()
***********************************************************************************************************************/
e_result_t R_PMA_SM_GetPowerPolicy(st_pma_handle_t       handle,
                                   e_pma_hwa_id_t        hwaId,
                                   e_pma_power_policy_t *pPolicy)
{
    e_result_t LenResult;
    const st_hwa_t * LpHwa;

    if ((NULL != handle)
       && (hwa_idmax > hwaId)
       && (NULL != pPolicy))
    {
        LpHwa = R_PMA_SM_GetHwa(hwaId);
        LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

        if (R_PMA_OK == LenResult)
        {
            *pPolicy = LpHwa->enApparentMode;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetPowerPolicy()
***********************************************************************************************************************/



/***********************************************************************************************************************
* Start of function R_PMA_SM_CheckUserMatchHwa()
***********************************************************************************************************************/
/* PRQA S 9106 2 # Since it is created as a common function, calls from multiple functions are expected */
/* PRQA S 3673 1 # Tool error, handle is a pointer to const */
static e_result_t R_PMA_SM_CheckUserMatchHwa(st_pma_handle_t handle,
                                             const st_hwa_t *pHwa)
{
    e_result_t LenResult;

    if ((NULL != handle) && (NULL != pHwa))
    {
        if (pHwa->handle == handle)
        {
            LenResult = R_PMA_OK;
        }
        else
        {
            if (NULL != pHwa->handle)
            {
                LenResult = R_PMA_INVALID_HANDLE;
            }
            else
            {
                LenResult = R_PMA_INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_CheckUserMatchHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetSyscDomain()
***********************************************************************************************************************/
st_sysc_domain_t *R_PMA_SM_GetSyscDomain(e_sysc_domain_id_t syscId)
{
    /* PRQA S 4342 2 # No problem because it is an enum and an integer cast */
    /* PRQA S 0686 2 # It is recognized as an Exception 1 of Rule 9.3. */
    static st_sysc_domain_t LstSyscDomain[R_PMA_SYSC_DOMAIN_ID_NUM] = {{ (e_sysc_domain_id_t)0U, 0U, NULL }};

    st_sysc_domain_t * LpDomain;

    if (sysc_idmax > syscId)
    {
        LpDomain = &LstSyscDomain[syscId];
    }
    else
    {
        LpDomain = NULL;
    }

    return LpDomain;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetSyscDomain()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetCpgDomain()
***********************************************************************************************************************/
st_cpg_domain_t *R_PMA_SM_GetCpgDomain(e_cpg_domain_id_t cpgId)
{
    /* PRQA S 4342 3 # No problem because it is an enum and an integer cast */
    /* PRQA S 0686 2 # It is recognized as an Exception 1 of Rule 9.3. */
    static st_cpg_domain_t LstCpgDomain[R_PMA_CPG_DOMAIN_ID_NUM] =
        {{ (e_cpg_domain_id_t)0U, 0U, (e_pma_reset_req_t)0U, NULL }};

    st_cpg_domain_t * LpDomain;

    if (cpg_idmax > cpgId)
    {
        LpDomain = &LstCpgDomain[cpgId];
    }
    else
    {
        LpDomain = NULL;
    }

    return LpDomain;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetCpgDomain()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetHwa()
***********************************************************************************************************************/
/* PRQA S 9106 1 # Since it is created as a common function, calls from multiple functions are expected */
st_hwa_t *R_PMA_SM_GetHwa(e_pma_hwa_id_t hwaId)
{
    /* PRQA S 4342 4 # No problem because it is an enum and an integer cast */
    /* PRQA S 0686 3 # It is recognized as an Exception 1 of Rule 9.3. */
    static st_hwa_t LstHwa[R_PMA_HWA_ID_NUM] =
        {{ NULL, (e_pma_hwa_req_t)0U, 0U, (e_pma_power_policy_t)0U,
           (e_pma_power_policy_t)0U, (e_pma_power_policy_t)0U, NULL, NULL, (e_pma_power_state_t)0U }};

    st_hwa_t * LpHwa;

    if (hwa_idmax > hwaId)
    {
        LpHwa = &LstHwa[hwaId];
    }
    else
    {
        LpHwa = NULL;
    }

    return LpHwa;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetClockStat()
***********************************************************************************************************************/
/* PRQA S 9106 1 # Since it is created as a common function, calls from multiple functions are expected */
e_result_t R_PMA_SM_SetClockStat(st_cpg_domain_t * pCpgDomain, bool bStat)
{
    e_result_t LenResult   = R_PMA_OK;
    r_cpg_Error_t   cpgRet;
    bool             clkStat;

    if (true == bStat)
    {
        if (0xffUL > pCpgDomain->refCnt)
        {
            pCpgDomain->refCnt++;
            cpgRet = R_PMA_CPG_GetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[pCpgDomain->id]), &clkStat);
            if ((true == clkStat) && (R_CPG_ERR_SUCCESS== cpgRet))
            {
                /** Clock ON */
                cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[pCpgDomain->id]), false);
                if (R_CPG_ERR_SUCCESS != cpgRet)
                {
                    LenResult = R_PMA_NG;      /* PRQA S 4432 #This is an output from BSP */
                    pCpgDomain->refCnt--;
                }
            }
            else if(false == clkStat) /* PRQA S 1881 #This is an output from BSP */
            {
                /* Already ON -> do nothing */
            }
            else
            {
                LenResult = R_PMA_NG;      /* PRQA S 4432 #This is an output from BSP */
                pCpgDomain->refCnt--;
            }
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else if (false == bStat)
    {
        if (0U < pCpgDomain->refCnt)
        {
            pCpgDomain->refCnt--;
        }

        if (0U == pCpgDomain->refCnt)
        {
            /** Clock Off */
            cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[pCpgDomain->id]), true);
            if (R_CPG_ERR_SUCCESS != cpgRet)
            {
                LenResult = R_PMA_NG; 
                pCpgDomain->refCnt++;
            }
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }
    
    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetClockStat()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetPowerStat()
***********************************************************************************************************************/
/* PRQA S 9106 1 # Since it is created as a common function, calls from multiple functions are expected */
e_result_t R_PMA_SM_SetPowerStat(st_sysc_domain_t * pSyscDomain, bool bStat)
{
    /* PRQA S 5209 2 #This is an output from BSP */
    e_result_t LenResult  = R_PMA_OK;
    r_sysc_Error_t  pwrRet;  
    portTickType startTick, endTick, tickDiff;
    uint32_t         pwrStat;

    if (true == bStat)
    {
        if (0xffUL > pSyscDomain->refCnt)
        {
            pSyscDomain->refCnt++;

            pwrRet = R_PMA_SYSC_SetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pSyscDomain->id], bStat);
            if (R_SYSC_ERR_SUCCESS != pwrRet)
            {
                LenResult  = R_PMA_NG;
                pSyscDomain->refCnt--;
            }
            else
            {
                pwrStat = 0;
                tickDiff = 0;
                pwrRet = R_PMA_SYSC_GetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pSyscDomain->id], true, &pwrStat);
                startTick = xTaskGetTickCount();
                endTick = startTick;
                while ((0 == pwrStat) && (R_SYSC_ERR_SUCCESS == pwrRet) && (R_PMA_WAIT_TICK > tickDiff))
                {
                    pwrRet = R_PMA_SYSC_GetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pSyscDomain->id], true, &pwrStat);
                    endTick = xTaskGetTickCount();
                    tickDiff = endTick - startTick;
                }

                if ((R_SYSC_ERR_SUCCESS != pwrRet) || (R_PMA_WAIT_TICK <= tickDiff))
                {
                    LenResult  = R_PMA_NG;
                    printf("Waiting for State after calling R_PMA_SYSC_SetPowerStat (Power ON) is error.\n");
                }
            }
        }
        else
        {
            LenResult  = R_PMA_NG;
        }

    }
    else
    {
        if (0U < pSyscDomain->refCnt)
        {

            pSyscDomain->refCnt--;

            if (0U == pSyscDomain->refCnt)
            {
                pwrRet = R_PMA_SYSC_SetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pSyscDomain->id], bStat);
                if (R_SYSC_ERR_SUCCESS != pwrRet)
                {
                    LenResult  = R_PMA_NG;
                    pSyscDomain->refCnt++;
                }
                else
                {
                    pwrStat = 0;
                    tickDiff = 0;
                    pwrRet = R_PMA_SYSC_GetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pSyscDomain->id], false, &pwrStat);
                    startTick = xTaskGetTickCount();
                    endTick = startTick;

                    while ((0 == pwrStat) && (R_SYSC_ERR_SUCCESS == pwrRet) && (R_PMA_WAIT_TICK > tickDiff))
                    {
                        pwrRet = R_PMA_SYSC_GetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pSyscDomain->id], false, &pwrStat);
                        endTick = xTaskGetTickCount();
                        tickDiff = endTick - startTick;
                    }

                    if ((R_SYSC_ERR_SUCCESS != pwrRet) || (R_PMA_WAIT_TICK <= tickDiff))
                    {
                        LenResult  = R_PMA_NG;
                        printf("Waiting for State after calling R_PMA_SYSC_SetPowerStat (Power OFF) is error.\n");
                    }
                }
            }
        }

    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetPowerStat()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_RequestHWA()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_RequestHWA(st_pma_handle_t handle,
                                      e_pma_hwa_id_t  hwaId)
{
    e_result_t LenResult;
    st_hwa_t * LpHwa;

    LpHwa     = R_PMA_SM_GetHwa(hwaId);
    LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

    if (R_PMA_OK == LenResult)
    {
        if (0xffULL > LpHwa->refCnt)
        {
            LpHwa->refCnt++;
            LenResult = R_PMA_SM_GstRegStatChangeFuncTbl[hwaId].funcToReady(LpHwa);
            if (R_PMA_OK == LenResult)
            {
                LpHwa->enReqStat = R_PMA_HWA_REQUESTED;
                /* PRQA S 3469 2 # To reduce function overhead */
                /* PRQA S 4461 1 # QAC false positives, uint64_t not recognized as 64-bit */
                R_PMA_SM_DEL_BIT64(hwaId);
            }
            else
            {
                LpHwa->refCnt--;
            }
        }
        else
        {
            LenResult = R_PMA_NG;
        }

    }
    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_RequestHWA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ReleaseHWA()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_ReleaseHWA(st_pma_handle_t handle,
                                      e_pma_hwa_id_t  hwaId)
{
    e_result_t LenResult;
    st_hwa_t * LpHwa;

    LpHwa     = R_PMA_SM_GetHwa(hwaId);
    LenResult = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

    if (R_PMA_OK == LenResult)
    {
        if (0ULL < LpHwa->refCnt)
        {
            LpHwa->refCnt--;
        }

        if (R_PMA_HWA_RELEASED != LpHwa->enReqStat)
        {
            if (R_PMA_PG_MODE == LpHwa->enPwrPolicy)
            {
                LenResult = R_PMA_SM_GstRegStatChangeFuncTbl[hwaId].funcToPG(LpHwa);
            }
            else if (R_PMA_CG_MODE == LpHwa->enPwrPolicy)
            {
                LenResult = R_PMA_SM_ChangeToCG(LpHwa);
            }
            else
            {
                /** do nothing in HP-MODE */
            }

            if (R_PMA_OK == LenResult)
            {
                LenResult = R_PMA_SM_SetLowestPowerState(handle, hwaId);
                /* PRQA S 3469 2 # To reduce function overhead */
                /* PRQA S 4461 1 # QAC false positives, uint64_t not recognized as 64-bit */
                R_PMA_SM_SET_BIT64(hwaId);
                LpHwa->enPrevStbMode = LpHwa->enPwrPolicy;
                LpHwa->enReqStat     = R_PMA_HWA_RELEASED;
                if ((R_PMA_OK == LenResult) && (LpHwa->pSyscDomain != NULL))
                {
                    LenResult = R_PMA_SM_CheckDepState(handle, sysc_mask_tbl[LpHwa->pSyscDomain->id]);
                }
            }
            else
            {
                LpHwa->refCnt++;
            }
        }
    }
    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ReleaseHWA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ApplyResetHWA()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_ApplyResetHWA(st_pma_handle_t handle,
                                         e_pma_hwa_id_t  hwaId)
{
    e_result_t     LenResult = R_PMA_OK;
    const st_hwa_t * LpHwa;
    r_cpg_Error_t   cpgRet;

    LpHwa       = R_PMA_SM_GetHwa(hwaId);
    LenResult   = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

    if (R_PMA_OK == LenResult)
    {
        if (R_PMA_APPLIED_RESET != LpHwa->pCpgDomain->enRstStat)
        {
            cpgRet =R_PMA_CPG_SetResetStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[LpHwa->pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[LpHwa->pCpgDomain->id]), 1);
            if (R_CPG_ERR_SUCCESS != cpgRet)
            {
                LenResult = R_PMA_NG;
            }
            if (R_PMA_OK == LenResult)
            {
                LpHwa->pCpgDomain->enRstStat = R_PMA_APPLIED_RESET;
            }
        }
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ApplyResetHWA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ReleaseResetHWA()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_ReleaseResetHWA(st_pma_handle_t handle,
                                           e_pma_hwa_id_t  hwaId)
{
    e_result_t     LenResult = R_PMA_OK;
    const st_hwa_t * LpHwa;
    r_cpg_Error_t   cpgRet;

    LpHwa       = R_PMA_SM_GetHwa(hwaId);
    LenResult   = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

    if (R_PMA_OK == LenResult)
    {
        if (R_PMA_RELEASED_RESET != LpHwa->pCpgDomain->enRstStat)
        {
            cpgRet =R_PMA_CPG_SetResetStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[LpHwa->pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[LpHwa->pCpgDomain->id]), 0);
            if (R_CPG_ERR_SUCCESS != cpgRet)
            {
                LenResult = R_PMA_NG;
            }
            if (R_PMA_OK == LenResult)
            {
                LpHwa->pCpgDomain->enRstStat = R_PMA_RELEASED_RESET;
            }
        }
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ReleaseResetHWA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_GetLowestPowerState()
***********************************************************************************************************************/
/*PRQA S 3673 1 # Tool error, handle is a pointer to const */
e_result_t R_PMA_SM_GetLowestPowerState(st_pma_handle_t handle,           
                                        e_pma_hwa_id_t hwaId,
                                        e_pma_power_state_t *pLowestPower
)
{
    e_result_t LenResult;
    st_hwa_t  const * LpHwa;


    if (handle == NULL)
    {
        LenResult = R_PMA_NG;
    }
    else
    {
        LenResult = R_PMA_OK;
    }

    if ((hwa_idmax > hwaId) && (NULL != pLowestPower))
    {
        LpHwa     = R_PMA_SM_GetHwa(hwaId);
        
        if (NULL != LpHwa)
        {
            *pLowestPower = LpHwa->enLowestPwrState;
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_INVALID_ARGUMENT;
    }

    return LenResult;

}
/***********************************************************************************************************************
* End of function R_PMA_SM_GetLowestPowerState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_SetPostClock()
***********************************************************************************************************************/
e_result_t R_PMA_SM_SetPostClock(e_pma_clk_req_t clockEnable,
                                 uint32_t        chanIdx,
                                 uint32_t const * pClockDivider
)
{
    e_result_t      LenResult   = R_PMA_OK;
    return LenResult;

}
/***********************************************************************************************************************
* End of function R_PMA_SM_SetPostClock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_ChangePolicyHWA()
***********************************************************************************************************************/
/* PRQA S 1505 1 # Designed as an internal common function, not a static function in anticipation of future expansion */
e_result_t R_PMA_SM_ChangePolicyHWA(st_pma_handle_t handle,
                                    e_pma_hwa_id_t  hwaId,
                                    e_pma_power_policy_t policy)
{
    e_result_t     LenResult;
    const st_hwa_t * LpHwa;
    /* if V3U there is grand parents domain */

    LpHwa       = R_PMA_SM_GetHwa(hwaId);
    LenResult   = R_PMA_SM_CheckUserMatchHwa(handle, LpHwa);

    if (R_PMA_OK == LenResult)
    {
        if (NULL == LpHwa->pSyscDomain)
        {
            if (R_PMA_HWA_REQUESTED != LpHwa->enReqStat)
            {
                LenResult = R_PMA_SM_PolicyToStatusWithoutPower(hwaId, LpHwa, policy);

                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_UpdateLowestPowerState(handle, hwaId);
                }
            }
        }
        else
        {
            if (R_PMA_HWA_REQUESTED != LpHwa->enReqStat)
            {
                LenResult = R_PMA_SM_PolicyToStatus(hwaId, LpHwa, policy);
                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_CheckDepState(handle, sysc_mask_tbl[LpHwa->pSyscDomain->id]);
                }
            }
        }
    }

    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_SetPowerPolicy(handle, hwaId, policy);
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_ChangePolicyHWA()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_PolicyToStatus()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_PolicyToStatus(e_pma_hwa_id_t  hwaId,
                                          const st_hwa_t * pHwa,
                                          e_pma_power_policy_t policy)
{
    e_result_t LenResult = R_PMA_OK;
    uint32_t LuiholdCnt;

    switch(policy)/** new policy */
    {
    /** R_PMA_PG_MODE */
    case R_PMA_PG_MODE:
        if (R_PMA_CG_MODE == pHwa->enPwrPolicy) /** old policy */
        {
            /**power off */
            LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_OFF);
            if ((R_PMA_OK == LenResult) && (NULL != pHwa->pSyscDomain->pDepDomain))
            {
                LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain->pDepDomain, R_PMA_SYSC_POWER_OFF);
            }
            break;
        }
        else if (R_PMA_HP_MODE == pHwa->enPwrPolicy)
        {
            /**R_PMA_SM_ReleaseHWA */
            LenResult = R_PMA_SM_GstRegStatChangeFuncTbl[hwaId].funcToPG(pHwa);
            break;
        }
        else /** no change  */
        {
            /** do nothing */
        }
        break;
    /** R_PMA_CG_MODE */
    case R_PMA_CG_MODE:
        if (R_PMA_HP_MODE == pHwa->enPwrPolicy)
        {
            /** R_PMA_SM_ChangeToCG */
            LenResult = R_PMA_SM_ChangeToCG(pHwa);
            break;
        }
        else if (R_PMA_PG_MODE == pHwa->enPwrPolicy)
        {
            if (0U < pHwa->pSyscDomain->refCnt)
            {
                pHwa->pSyscDomain->refCnt++;
            }
            else
            {
                LuiholdCnt = pHwa->pCpgDomain->refCnt;
                /**power on */
                if (NULL != pHwa->pSyscDomain->pDepDomain)
                {
                    LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain->pDepDomain, R_PMA_SYSC_POWER_ON);
                }
                if (R_PMA_OK == LenResult)
                {
                    /** Clock Off */
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
                }
                if (R_PMA_OK == LenResult)
                {
                    /** Clock On */
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_ON);
                }

                if (R_PMA_OK == LenResult)
                {
                    LenResult = R_PMA_SM_SetPowerStat(pHwa->pSyscDomain, R_PMA_SYSC_POWER_ON);
                }
                if (R_PMA_OK == LenResult)
                {
                    /** Clock Off */
                    LenResult = R_PMA_SM_SetClockStat(pHwa->pCpgDomain, R_PMA_CPG_CLOCK_OFF);
                }
                if (R_PMA_OK == LenResult)
                {
                   pHwa->pCpgDomain->refCnt = LuiholdCnt;
                }
            }
            break;
        }
        else /** no change  */
        {
            /** do nothing */
        }
        break;
    /** R_PMA_HP_MODE */
    case R_PMA_HP_MODE:
        /** R_PMA_SM_RequestHWA */
        LenResult = R_PMA_SM_GstRegStatChangeFuncTbl[hwaId].funcToReady(pHwa);
        break;
    default:
        LenResult = R_PMA_NG;
        break;
    }
    return LenResult;
}/* PRQA S 9110 # It is a series of processes and is necessary so as not to reduce the readability based on design */
/***********************************************************************************************************************
* End of function R_PMA_SM_PolicyToStatus()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_PolicyToStatusWithoutPower()
***********************************************************************************************************************/
static e_result_t R_PMA_SM_PolicyToStatusWithoutPower(e_pma_hwa_id_t hwaId,
                                                      const st_hwa_t * pHwa,
                                                      e_pma_power_policy_t policy)
{
    e_result_t LenResult = R_PMA_OK;

    switch(policy)/** new policy */
    {
    /** R_PMA_PG_MODE */
    case R_PMA_PG_MODE:
    /** R_PMA_CG_MODE */
    case R_PMA_CG_MODE:
        if (R_PMA_HP_MODE == pHwa->enPwrPolicy)
        {
            /** R_PMA_SM_ChangeToCG */
            LenResult = R_PMA_SM_ChangeToCG(pHwa);
        }
        else
        {
            /** no change  */
            /** do nothing */
        }
        break;
    /** R_PMA_HP_MODE */
    case R_PMA_HP_MODE:
        /** R_PMA_SM_RequestHWA */
        LenResult = R_PMA_SM_GstRegStatChangeFuncTbl[hwaId].funcToReady(pHwa);
        break;
    default:
        LenResult = R_PMA_NG;
        break;
    }

    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_SM_PolicyToStatusWithoutPower()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_HwStateOff()
***********************************************************************************************************************/
e_result_t R_PMA_SM_HwStateOff(void)
{
    e_result_t LenResult = R_PMA_OK;
    int32_t id;
    r_cpg_Error_t   cpgRet;
    r_sysc_Error_t  pwrRet;
    bool             clkStat;
    uint32_t         pwrStat;

    for (id = (int32_t)cpg_idmin; (int32_t)cpg_idmax > id; id++)
    {
        if (id != (int32_t)cpg_idimpr) /* if A3IR skip */
        {
            cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[id]), false);
        }
        else
        {
            cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[id]), true);
        }
        if (R_CPG_ERR_SUCCESS != cpgRet)
        {
            LenResult = R_PMA_NG;
            break;
        }
    }

    if (R_CPG_ERR_SUCCESS == cpgRet)
    {
        pwrRet = R_SYSC_ERR_FAILED;
        for (id = (int32_t)sysc_idmin; (int32_t)sysc_idmax > id; id++)
        {
            pwrRet = R_PMA_SYSC_GetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[id], false, &pwrStat);

            if ((0 == pwrStat) && (R_SYSC_ERR_SUCCESS == pwrRet))
            {
                pwrRet = R_PMA_SYSC_SetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[id], false);
            }
            else
            {
                /* error */
            }
#ifdef INTEGRITY_GUEST
            /* FIXME: This is a workaround for INTEGRITY-OS */
            pwrRet = R_SYSC_ERR_SUCCESS;
#endif
            if (R_SYSC_ERR_SUCCESS != pwrRet)
            {
                LenResult = R_PMA_NG;
                break;
            }
        }
    }
    else
    {
        LenResult = R_PMA_NG;
        pwrRet = R_SYSC_ERR_FAILED;
    }

    if (R_SYSC_ERR_SUCCESS == pwrRet)
    {
        for (id = (int32_t)cpg_idmin; (int32_t)cpg_idmax > id; id++)
        {
            cpgRet = R_PMA_CPG_GetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[id]), &clkStat);

            if ((false == clkStat) && (R_CPG_ERR_SUCCESS == cpgRet))
            {
                cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[id]), true);
            }

            if (R_CPG_ERR_SUCCESS != cpgRet)
            {
                LenResult = R_PMA_NG;
                break;
            }
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    return LenResult;
/* PRQA S 9104 3 # The HW initialization function has already multiple sub-functions to reduce complexity.
                   We have decided that further division is not desirable from perspective of maintainability and readability.
*/
}
/***********************************************************************************************************************
* End of function R_PMA_SM_HwStateOff()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SM_PowerONA3IR()
***********************************************************************************************************************/
e_result_t R_PMA_SM_PowerONA3IR(void)
{
    e_result_t  LenResult = R_PMA_OK;
    const st_hwa_t    *pHwaIMPRAM;
    r_cpg_Error_t   cpgRet;
    r_sysc_Error_t  pwrRet;
    bool             clkStat;

    pHwaIMPRAM = R_PMA_SM_GetHwa((e_pma_hwa_id_t)hwa_idimpr);

    if (NULL != pHwaIMPRAM)
    {

        cpgRet = R_PMA_CPG_GetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[pHwaIMPRAM->pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[pHwaIMPRAM->pCpgDomain->id]), &clkStat);
        if ((true == clkStat) && (R_CPG_ERR_SUCCESS== cpgRet))
        {
            /** Clock ON */
            cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[pHwaIMPRAM->pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[pHwaIMPRAM->pCpgDomain->id]), false);
        }
        if (R_CPG_ERR_SUCCESS == cpgRet)
        {
            pwrRet = R_PMA_SYSC_SetPowerStat(R_PMA_SYSC_PDR_ID_TABLE[pHwaIMPRAM->pSyscDomain->id], true);
        }
        else
        {
            pwrRet = R_SYSC_ERR_FAILED;
            LenResult = R_PMA_NG;
        }

        if (R_SYSC_ERR_SUCCESS == pwrRet)
        {
            pHwaIMPRAM->pSyscDomain->refCnt++;

            {
                cpgRet = R_PMA_CPG_SetModuleStopStat(CPG_MSTP_ID_TO_REG_OFFSET(R_PMA_CPG_MOD_ID_TABLE[pHwaIMPRAM->pCpgDomain->id]), CPG_MSTP_ID_TO_BIT(R_PMA_CPG_MOD_ID_TABLE[pHwaIMPRAM->pCpgDomain->id]), true);
            }
        }
        if ((cpgRet != R_CPG_ERR_SUCCESS) || (pwrRet != R_SYSC_ERR_FAILED))
        {
            LenResult = R_PMA_NG;
        }
    }
    else
    {
        LenResult = R_PMA_NG;
    }
    return LenResult;
}

/***********************************************************************************************************************
* End of function R_PMA_SM_PowerONA3IR()
***********************************************************************************************************************/
