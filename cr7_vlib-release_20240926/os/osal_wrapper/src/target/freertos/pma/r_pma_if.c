/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022]  Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : interface for OSAL.
***********************************************************************************************************************/

/*======================================================================================================================
Includes <System Includes> , "Project Includes"
======================================================================================================================*/
#include <string.h>
#include "target/freertos/pma/r_pma_common.h"
#include "target/freertos/pma/r_pma.h"
#include "target/freertos/pma/r_pma_statemachine.h"

#include "target/freertos/r_osal_common.h"
#include <r_cpg_api.h>
#include <r_sysc_api.h>

/*======================================================================================================================
Private macro definitions
======================================================================================================================*/
#define R_PMA_IF_USER_MAX                        (16u)

#define R_PMA_IF_TASK_PRIORITY_MIN_TH            (1u)
#define R_PMA_IF_TASK_PRIORITY_MAX_TH            (63u)
#define R_PMA_IF_TIMEOUT_MS_MIN_TH               (1u)
#define R_PMA_IF_POLLING_NUM_MIN_TH              (1u)
#define R_PMA_IF_LOOP_NUM_FOR_RCLK_CYCLE_MIN_TH  (1u)
#define R_PMA_MAX_TIMEOUT_MS                     ((osal_milli_sec_t)9223372036854)

/*======================================================================================================================
Private data types
======================================================================================================================*/

/*======================================================================================================================
Private function prototypes
======================================================================================================================*/
static e_pma_return_t R_PMA_IF_CheckConfigParam(const st_pma_config_t *pConfig, st_pma_param_t *pParam);
static e_pma_return_t R_PMA_IF_DefaultInit(const st_pma_param_t *pParam);
static e_pma_return_t R_PMA_IF_CheckUser(const st_pma_handle_t pUserInfo);
static st_pma_user_info_t *R_PMA_IF_GetUserInfo(uint32_t userNo);
static e_result_t R_PMA_IF_SendMessageToTask(st_msg_to_task_t *pMsg);
static e_result_t R_PMA_IF_GetCurrentState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, st_pma_hwa_state_t const * pHwaState, st_pma_hwa_state_t *pCurHwaState, e_pma_reset_req_t *pCurRstStat);
static e_osal_return_t R_PMA_IF_GetTimeDiffAndSleep(st_osal_time_t* LstStartTime, osal_nano_sec_t * pDiffTime, osal_milli_sec_t sleepTimePeriod);

static e_pma_return_t R_PMA_IF_Exchange_ErrCode(e_result_t innerErr);
/*======================================================================================================================
Private global variables
======================================================================================================================*/

static R_PMA_TIMEOUT_T R_PMA_IF_GxWaitTick;

/* PRQA S 3218 1 # This definition must be published because the user references the member after calling */
static R_PMA_QUE_HANDLE R_PMA_IF_GxQueueSend;

static bool LbIsFirstInit = true;

/*======================================================================================================================
Public function definitions
======================================================================================================================*/
/***********************************************************************************************************************
* Start of function R_PMA_Init()
***********************************************************************************************************************/
e_pma_return_t R_PMA_Init(st_pma_handle_t       *pHandle,
                          const st_pma_config_t *pConfig)
{
    e_pma_return_t       LenRetCode;
    st_pma_user_info_t * LpUserInfo;
    st_pma_param_t       LstParam;


    if (NULL == pHandle)
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }
    else
    {
        LenRetCode = R_PMA_IF_CheckConfigParam(pConfig, &LstParam);
    }
    if (R_PMA_RESULT_OK == LenRetCode)
    {
        /** first time initialization */
        if (LbIsFirstInit)
        {
            LbIsFirstInit = false;
            LenRetCode = R_PMA_IF_DefaultInit(&LstParam);
        }

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenRetCode = R_PMA_ERR_NO_VACANT_HANDLE;
            for (uint32_t num = 0U; num < R_PMA_IF_USER_MAX; num++)
            {
                LpUserInfo = R_PMA_IF_GetUserInfo(num);

                if (false == LpUserInfo->isUsed)
                {
                    LpUserInfo->isUsed = true;
                    /* PRQA S 0310,0308 2 # unavoidable in the casts of the structure */
                    /* PRQA S 2916 1 # There is no problem because it uses a persistent address */
                    *pHandle = (st_pma_handle_t)LpUserInfo;
                    LenRetCode = R_PMA_RESULT_OK;
                    break;
                }
            }
        }
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_Init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_Quit()
***********************************************************************************************************************/
e_pma_return_t R_PMA_Quit(st_pma_handle_t handle)
{
    e_pma_return_t          LenRetCode;
    e_result_t              LenResult = R_PMA_OK;
    st_pma_user_info_t *    LpUserInfo;
    static st_msg_to_task_t LstMsgToTask;
    int32_t                 LuiTaskRet  = 0;
    e_osal_return_t         LenOsalRet;

    LenRetCode = R_PMA_IF_CheckUser(handle);

    if (R_PMA_RESULT_OK == LenRetCode)
    {
        LstMsgToTask.enProcId    = R_PMA_PROCESS_ID_EXIT_THREAD;
        LstMsgToTask.stPmaHandle = handle;
        LenResult = R_PMA_IF_SendMessageToTask(&LstMsgToTask);
    }

    if (R_PMA_OK != LenResult) 
    {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }

    if (R_PMA_RESULT_OK == LenRetCode)
    {
        LenOsalRet = R_OSAL_Internal_ThreadTimedJoin(R_PMA_SM_GxThread, &LuiTaskRet, R_PMA_IF_GxWaitTick);
        if (LenOsalRet != OSAL_RETURN_OK) {
            LenRetCode = R_PMA_ERR_RESULT_NG;
        }
    }

    if (R_PMA_RESULT_OK == LenRetCode)
    {
        /* PRQA S 4424 1 # No problem because it is an enum and an integer cast */
        for (uint32_t id = hwa_idmin; id < (uint32_t)hwa_idmax; id++)
        {
            /* PRQA S 4342 1 # No problem because it is an enum and an integer cast */
            (void)R_PMA_SM_UnlockHwa(handle, (e_pma_hwa_id_t)id);
        }

        /* PRQA S 0310,0308 1 # unavoidable in the casts of the structure */
        LpUserInfo = (st_pma_user_info_t*)handle;
        LpUserInfo->isUsed = false;
    }
    LenOsalRet = R_OSAL_Internal_MqDelete(R_PMA_SM_GxTaskQueue);
    if (LenOsalRet != OSAL_RETURN_OK) {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }

    LenOsalRet = R_OSAL_Internal_MutexDestroy(R_PMA_SM_GxMutex);
    if (LenOsalRet != OSAL_RETURN_OK) {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }
    if (R_PMA_OK == LenResult)
    {
        LenResult = R_PMA_SM_HwStateOff();
        if (R_PMA_RESULT_OK == LenRetCode)
        {
            R_PMA_CPG_Quit();
            R_PMA_SYSC_Quit();
        }

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
        }
    }
    LbIsFirstInit = true;
    return LenRetCode;
/* PRQA S 9104,9107 3 # The termination function has already multiple sub-functions to reduce complexity.
                             We have decided that further division is not desirable from perspective of maintainability and readability.
*/
}
/***********************************************************************************************************************
* End of function R_PMA_Quit()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetLockHwa()
***********************************************************************************************************************/
e_pma_return_t R_PMA_GetLockHwa(st_pma_handle_t handle,
                                e_pma_hwa_id_t  hwaId)
{
    e_pma_return_t LenRetCode;
    e_result_t     LenResult;

    if (hwa_idmax > hwaId)
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenResult = R_PMA_SM_GetLockHwa(handle, hwaId);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_GetLockHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_UnlockHwa()
***********************************************************************************************************************/
/* PRQA S 1505 1 # This api is called from user App. */
e_pma_return_t R_PMA_UnlockHwa(st_pma_handle_t handle,
                               e_pma_hwa_id_t  hwaId)
{
    e_pma_return_t LenRetCode;
    e_result_t     LenResult;

    LenRetCode = R_PMA_ReleaseHwa(handle, hwaId);

    if (R_PMA_RESULT_OK == LenRetCode)
    {
        LenResult = R_PMA_SM_UnlockHwa(handle, hwaId);
        LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);

    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_UnlockHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_RequestHwa()
***********************************************************************************************************************/
e_pma_return_t R_PMA_RequestHwa(st_pma_handle_t handle,
                                e_pma_hwa_id_t  hwaId)
{
    e_pma_return_t   LenRetCode;
    e_result_t       LenResult;
    st_msg_to_task_t LstMsgToTask;

    if (hwa_idmax > hwaId)
    {
        LstMsgToTask.enProcId    = R_PMA_PROCESS_ID_REQUEST_HWA;
        LstMsgToTask.stPmaHandle = handle;
        LstMsgToTask.enHwaId     = hwaId;

        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenResult = R_PMA_IF_SendMessageToTask(&LstMsgToTask);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);

        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }
    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_RequestHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_ReleaseHwa()
***********************************************************************************************************************/
/* PRQA S 1505 1 # This api is called from user App. */
e_pma_return_t R_PMA_ReleaseHwa(st_pma_handle_t handle,
                                e_pma_hwa_id_t  hwaId)
{
    e_pma_return_t   LenRetCode;
    e_result_t       LenResult;
    st_msg_to_task_t LstMsgToTask;

    if (hwa_idmax > hwaId)
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LstMsgToTask.enProcId    = R_PMA_PROCESS_ID_RELEASE_HWA;
            LstMsgToTask.stPmaHandle = handle;
            LstMsgToTask.enHwaId     = hwaId;

            LenResult = R_PMA_IF_SendMessageToTask(&LstMsgToTask);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);

        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }
    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_ReleaseHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetHwaState()
***********************************************************************************************************************/
e_pma_return_t R_PMA_GetHwaState(st_pma_handle_t     handle,
                                 e_pma_hwa_id_t      hwaId,
                                 st_pma_hwa_state_t *pHwaState)
{
    e_pma_return_t  LenRetCode;
    e_result_t      LenResult;
    e_osal_return_t LenOsalRet;

    LenOsalRet = R_OSAL_Internal_MutexTimedLock(R_PMA_SM_GxMutex, R_PMA_IF_GxWaitTick);
    
    if (OSAL_RETURN_OK == LenOsalRet)
    {
        if ((hwa_idmax > hwaId) && (NULL != pHwaState))
        {
            LenRetCode = R_PMA_IF_CheckUser(handle);

            if (R_PMA_RESULT_OK == LenRetCode)
            {
                LenResult = R_PMA_SM_GetHwaState(handle, hwaId, pHwaState);
                LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
            }
        }
        else
        {
            LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
        }

        LenOsalRet = R_OSAL_Internal_MutexUnlock(R_PMA_SM_GxMutex);
        if (OSAL_RETURN_OK == LenOsalRet)
        {
            /** do nothing */
        }
        else
        {
            LenRetCode = R_PMA_ERR_RESULT_NG;
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_GetHwaState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SetPowerPolicy()
***********************************************************************************************************************/
e_pma_return_t R_PMA_SetPowerPolicy(st_pma_handle_t      handle,
                                    e_pma_hwa_id_t       hwaId,
                                    e_pma_power_policy_t policy)
{
    e_pma_return_t LenRetCode;
    e_result_t     LenResult;
    st_msg_to_task_t LstMsgToTask;

    if ((hwa_idmax > hwaId)
        && ((R_PMA_PG_MODE == policy) || (R_PMA_CG_MODE == policy) || (R_PMA_HP_MODE == policy)))
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LstMsgToTask.enProcId    = R_PMA_PROCESS_ID_CHANGE_POLICY;
            LstMsgToTask.stPmaHandle = handle;
            LstMsgToTask.enHwaId     = hwaId;
            LstMsgToTask.enPolicy    = policy;  /* after policy (before policy >>> st_hwa_t.enPwrPolicy) */

            LenResult = R_PMA_IF_SendMessageToTask(&LstMsgToTask);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }
    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_SetPowerPolicy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetPowerPolicy()
***********************************************************************************************************************/
e_pma_return_t R_PMA_GetPowerPolicy(st_pma_handle_t       handle,
                                    e_pma_hwa_id_t        hwaId,
                                    e_pma_power_policy_t *pPolicy)
{
    e_pma_return_t LenRetCode;
    e_result_t     LenResult;

    if ((hwa_idmax > hwaId) && (NULL != pPolicy))
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenResult = R_PMA_SM_GetPowerPolicy(handle, hwaId, pPolicy);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);

        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_GetPowerPolicy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_ApplyResetHwa()
***********************************************************************************************************************/
e_pma_return_t R_PMA_ApplyResetHwa(st_pma_handle_t handle,
                                   e_pma_hwa_id_t  hwaId)
{
    e_pma_return_t   LenRetCode;
    e_result_t       LenResult;
    st_msg_to_task_t LstMsgToTask;

    if (hwa_idmax > hwaId)
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LstMsgToTask.enProcId    = R_PMA_PROCESS_ID_APPLY_RESET;
            LstMsgToTask.stPmaHandle = handle;
            LstMsgToTask.enHwaId     = hwaId;

            LenResult = R_PMA_IF_SendMessageToTask(&LstMsgToTask);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);

        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_ApplyResetHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_ReleaseResetHwa()
***********************************************************************************************************************/
e_pma_return_t R_PMA_ReleaseResetHwa(st_pma_handle_t handle,
                                     e_pma_hwa_id_t  hwaId)
{
    e_pma_return_t   LenRetCode;
    e_result_t       LenResult;
    st_msg_to_task_t LstMsgToTask;

    if (hwa_idmax > hwaId)
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);

        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LstMsgToTask.enProcId    = R_PMA_PROCESS_ID_RELEASE_RESET;
            LstMsgToTask.stPmaHandle = handle;
            LstMsgToTask.enHwaId     = hwaId;

            LenResult = R_PMA_IF_SendMessageToTask(&LstMsgToTask);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);

        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_ReleaseResetHwa()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetReset()
***********************************************************************************************************************/
e_pma_return_t R_PMA_GetReset(st_pma_handle_t    handle,
                              e_pma_hwa_id_t     hwaId,
                              e_pma_reset_req_t *pRstStat)
{
    e_pma_return_t  LenRetCode;
    e_result_t      LenResult;
    e_osal_return_t LenOsalRet;

    LenOsalRet = R_OSAL_Internal_MutexTimedLock(R_PMA_SM_GxMutex, R_PMA_IF_GxWaitTick);

    if (OSAL_RETURN_OK == LenOsalRet)
    {
        if ((hwa_idmax > hwaId)
             && (NULL != pRstStat))
        {
            LenRetCode = R_PMA_IF_CheckUser(handle);

            if (R_PMA_RESULT_OK == LenRetCode)
            {
                LenResult = R_PMA_SM_GetResetState(handle, hwaId, pRstStat);
                LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
            }
        }
        else
        {
            LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
        }

        LenOsalRet = R_OSAL_Internal_MutexUnlock(R_PMA_SM_GxMutex);

        if (OSAL_RETURN_OK == LenOsalRet)
        {
            /** do nothing */
        }
        else
        {
            LenRetCode = R_PMA_ERR_RESULT_NG;
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_GetReset()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetLowestPowerState() API
***********************************************************************************************************************/
e_pma_return_t R_PMA_GetLowestPowerState(st_pma_handle_t handle,
                                                e_pma_hwa_id_t hwaId,
                                                e_pma_power_state_t *pLowestPower)
{
    e_osal_return_t LenOsalRet;
    e_pma_return_t LenRetCode;
    e_result_t     LenResult;

    LenOsalRet = R_OSAL_Internal_MutexTimedLock(R_PMA_SM_GxMutex, R_PMA_IF_GxWaitTick);

    if (OSAL_RETURN_OK == LenOsalRet)
    {
        if ((hwa_idmax > hwaId)
             && (NULL != pLowestPower))
        {
            LenRetCode = R_PMA_IF_CheckUser(handle);

            if (R_PMA_RESULT_OK == LenRetCode)
            {
                LenResult = R_PMA_SM_GetLowestPowerState(handle, hwaId, pLowestPower);
                LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
            }
        }
        else
        {
            LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
        }
        
        LenOsalRet = R_OSAL_Internal_MutexUnlock(R_PMA_SM_GxMutex);

        if (OSAL_RETURN_OK == LenOsalRet)
        {
            /** do nothing */
        }
        else
        {
            LenRetCode = R_PMA_ERR_RESULT_NG;
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }

    return LenRetCode;

}
/***********************************************************************************************************************
* End of function R_PMA_GetLowestPowerState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_SetPostClock()
***********************************************************************************************************************/
e_pma_return_t R_PMA_SetPostClock(st_pma_handle_t const handle,     /*PRQA S 3673 # Tool error, handle is a pointer to const */
                                  e_pma_clk_req_t clockEnable,
                                  uint32_t        chanIdx,
                                  uint32_t const * pClockDivider)
{
    e_pma_return_t LenRetCode;
    e_result_t     LenResult;

    if (((R_PMA_CLK_ENABLE == clockEnable) || (R_PMA_CLK_DISABLE == clockEnable))
          && (R_PMA_COMMON_CLK_POSTCKCR >= chanIdx)
          && (R_PMA_COMMON_CLK_DIVIDER > *pClockDivider))
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);
       
        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenResult = R_PMA_SM_SetPostClock(clockEnable, chanIdx, pClockDivider);
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }
    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_SetPostClock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_WaitForState()
***********************************************************************************************************************/
 e_pma_return_t R_PMA_WaitForState(st_pma_handle_t     const handle,
                                   e_pma_hwa_id_t      hwaId,
                                   st_pma_hwa_state_t  const * pHwaState,
                                   e_pma_reset_req_t   rstStat,
                                   osal_milli_sec_t    sleepTimePeriod,
                                   osal_milli_sec_t    timeoutMsPeriod,
                                   osal_nano_sec_t     timeoutNsPeriod)
{
    e_pma_return_t      LenRetCode;
    e_result_t          LenResult;
    e_osal_return_t     LenOsalRet = OSAL_RETURN_OK;
    osal_nano_sec_t     LDiffTime = (osal_nano_sec_t)0;
    e_pma_reset_req_t   LenCurRstStat;
    st_osal_time_t      LstStartTime;
    st_pma_hwa_state_t  LstCurHwaState;

    /* PRQA S 0314 2 # The 1st parameter must be (void *) and there is no side side-effect in this case. */
    (void)memset((void*)&LstStartTime,   0, sizeof(st_osal_time_t));
    (void)memset((void*)&LstCurHwaState, 0, sizeof(st_pma_hwa_state_t));
    if ((hwa_idmax > hwaId)
        && ((osal_nano_sec_t)0 <= timeoutNsPeriod)
        && ((osal_milli_sec_t)0 <= timeoutMsPeriod)
        && ((osal_milli_sec_t)0 <= sleepTimePeriod)
            && (timeoutNsPeriod < (osal_nano_sec_t)1000000)
            && (timeoutMsPeriod < (osal_milli_sec_t)R_PMA_MAX_TIMEOUT_MS) 
            && (sleepTimePeriod <= timeoutMsPeriod))
    {
        LenRetCode = R_PMA_IF_CheckUser(handle);
        
        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenOsalRet = R_OSAL_Internal_GetHighResoTimeStamp(&LstStartTime);
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }
    
    if ((R_PMA_RESULT_OK == LenRetCode)
        && (OSAL_RETURN_OK == LenOsalRet))
    {
        do
        {
            LenResult = R_PMA_IF_GetCurrentState(handle, hwaId, pHwaState, &LstCurHwaState, &LenCurRstStat);
            if (R_PMA_OK == LenResult)
            {
                if ((pHwaState->enReqState == LstCurHwaState.enReqState)
                    && ((pHwaState->enActualPwrStat == LstCurHwaState.enActualPwrStat)
                        || (R_PMA_UNDEFINED_STATE == pHwaState->enActualPwrStat))
                    && ((pHwaState->enActualClkStat == LstCurHwaState.enActualClkStat)
                        || (R_PMA_UNDEFINED_STATE == pHwaState->enActualClkStat))
                    && ((R_PMA_HWA_RELEASED == pHwaState->enReqState)
                        || ((R_PMA_HWA_REQUESTED == pHwaState->enReqState)
                            && ((rstStat == LenCurRstStat) || (R_PMA_UNDEFINED_RESET == rstStat)))))
                {
                    /** Break out of the loop  (As expected) */
                    break;
                }

                LenOsalRet = R_PMA_IF_GetTimeDiffAndSleep(&LstStartTime, &LDiffTime, sleepTimePeriod);
            }
        } while ((((osal_nano_sec_t)(timeoutMsPeriod*(osal_milli_sec_t)1000000) + timeoutNsPeriod) >= LDiffTime)
                    && (OSAL_RETURN_OK == LenOsalRet)
                    && (R_PMA_OK == LenResult));
    }
    
    if (OSAL_RETURN_OK == LenOsalRet)
    {
        if (R_PMA_RESULT_OK == LenRetCode)
        {
            LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
            if (R_PMA_RESULT_OK == LenRetCode)
            {
                if (((osal_nano_sec_t)(timeoutMsPeriod*(osal_milli_sec_t)1000000) + timeoutNsPeriod) >= LDiffTime)
                {
                    LenRetCode = R_PMA_RESULT_OK;
                }
                else
                {
                    LenRetCode = R_PMA_ERR_TIMEOUT;
                }
            }
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }
    return LenRetCode;
} /* PRQA S 9108 # Because it is a policy not to make the argument a structure */
/***********************************************************************************************************************
* End of function R_PMA_WaitForState()
***********************************************************************************************************************/

/*======================================================================================================================
Private function definitions
======================================================================================================================*/

/***********************************************************************************************************************
* Start of function R_PMA_IF_CheckConfigParam()
***********************************************************************************************************************/
static e_pma_return_t R_PMA_IF_CheckConfigParam(const st_pma_config_t *pConfig,
                                                st_pma_param_t        *pParam)
{
    e_pma_return_t LenRetCode = R_PMA_RESULT_OK;

    if (NULL != pConfig)
    {
        if (NULL != pParam)
        {
            /** Task priority */
            if ((R_PMA_IF_TASK_PRIORITY_MIN_TH <= pConfig->task_priority)
                && (R_PMA_IF_TASK_PRIORITY_MAX_TH >= pConfig->task_priority))
            {
                pParam->task_priority = pConfig->task_priority;
            }
            else
            {
                LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
            }

            /** Timeout */
            if (R_PMA_RESULT_OK == LenRetCode)
            {
                /* PRQA S 1842 2 # The comparison between constants and int is not a problem,
                   as QAC seems to be false positives */
                if ((int32_t)R_PMA_IF_TIMEOUT_MS_MIN_TH <= pConfig->timeout_ms)
                {
                    pParam->timeout_ticks = pConfig->timeout_ms;
                }
                else
                {
                    LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
                }
            }

            /** Polling times */
            if (R_PMA_RESULT_OK == LenRetCode)
            {
                if (R_PMA_IF_POLLING_NUM_MIN_TH <= pConfig->polling_max_num)
                {
                    pParam->polling_max_num = pConfig->polling_max_num;
                }
                else
                {
                    LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
                }
            }

            /** RCLK 1cycle wait */
            if (R_PMA_RESULT_OK == LenRetCode)
            {
                if (R_PMA_IF_LOOP_NUM_FOR_RCLK_CYCLE_MIN_TH <= pConfig->loop_num_for_rclk_cycle)
                {
                    pParam->loop_num_for_rclk_cycle = pConfig->loop_num_for_rclk_cycle;
                }
                else
                {
                    LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
                }
            }
        }
        else
        {
            LenRetCode = R_PMA_ERR_RESULT_NG;
        }
    }
    else
    {
        /* PRQA S 2053 4 # Advance start for future updates */
        /**pParam->task_priority           = R_PMA_IF_TASK_PRIORITY;            */
        /**pParam->timeout_ticks           = R_PMA_IF_WAIT_TICK;                */
        /**pParam->polling_max_num         = R_PMA_IF_MAX_POLLING;              */
        /**pParam->loop_num_for_rclk_cycle = R_PMA_IF_LOOP_NUM_FOR_RCLK_CYCLE;  */
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_IF_CheckConfigParam()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_IF_DefaultInit()
***********************************************************************************************************************/
static e_pma_return_t R_PMA_IF_DefaultInit(const st_pma_param_t *pParam)
{
    e_pma_return_t  LenRetCode;
    e_result_t      LenResult;
    e_osal_return_t LenOsalRet;
    osal_inner_mutexattr_t osal_inner_mutexattr;

    /* PRQA S 2016 1 # No problem to cast from uint32_t to int32_t. It is based on the design. */
    R_PMA_IF_GxWaitTick = (R_PMA_TIMEOUT_T)pParam->timeout_ticks;
    LenOsalRet          = R_OSAL_Internal_SetMutexAttr(&osal_inner_mutexattr);
    if (OSAL_RETURN_OK == LenOsalRet)
    {
        LenOsalRet = R_OSAL_Internal_MutexInit(&osal_inner_mutexattr, &R_PMA_SM_GxMutex);
    }

    if ((OSAL_RETURN_OK == LenOsalRet)
        && (NULL != R_PMA_SM_GxMutex))
    {
        LenResult = R_PMA_SM_Init(&R_PMA_IF_GxQueueSend, pParam);

        LenRetCode = R_PMA_IF_Exchange_ErrCode(LenResult);
    }
    else
    {
        LenRetCode = R_PMA_ERR_RESULT_NG;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_IF_DefaultInit()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_IF_CheckUser()
***********************************************************************************************************************/
/* PRQA S 3673 2 # Tool error, handle is a pointer to const */
/* PRQA S 9106 1 # Since it is created as a common function, calls from multiple functions are expected */
static e_pma_return_t R_PMA_IF_CheckUser(const st_pma_handle_t pUserInfo)   
{
    e_pma_return_t             LenRetCode;
    const st_pma_user_info_t * LpUserInfo;
    /* PRQA S 0310,0308 1 # unavoidable in the casts of the structure */
    const st_pma_user_info_t * LpUserInfoArg = (st_pma_user_info_t *)pUserInfo; 

    LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;

    if (NULL != LpUserInfoArg)
    {
        for (uint32_t num = 0U; num < R_PMA_IF_USER_MAX; num++)
        {
            LpUserInfo = R_PMA_IF_GetUserInfo(num);

            if (LpUserInfo == LpUserInfoArg)
            {
                if (true == LpUserInfoArg->isUsed)
                {
                    LenRetCode = R_PMA_RESULT_OK;
                }
                else
                {
                    LenRetCode = R_PMA_ERR_INVALID_HANDLE;
                }

                break;
            }
        }
    }
    else
    {
        LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
    }

    return LenRetCode;
}
/***********************************************************************************************************************
* End of function R_PMA_IF_CheckUser()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_IF_GetUserInfo()
***********************************************************************************************************************/
static st_pma_user_info_t *R_PMA_IF_GetUserInfo(uint32_t userNo)
{
    static st_pma_user_info_t LstUsers[R_PMA_IF_USER_MAX] =
    {
        /** isUsed */
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false },
        { false }
    };

    st_pma_user_info_t * LpUserInfo;

    if (R_PMA_IF_USER_MAX > userNo)
    {
        LpUserInfo = &LstUsers[userNo];
    }
    else
    {
        LpUserInfo = NULL;
    }

    return LpUserInfo;
}
/***********************************************************************************************************************
* End of function R_PMA_IF_GetUserInfo()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_IF_SendMessageToTask()
***********************************************************************************************************************/
/* PRQA S 3673 2 # Implemented in accordance with the 3rd parameter of R_OSAL_MqSendForTimePeriod(). */
/* PRQA S 9106 1 # Since it is created as a common function, calls from multiple functions are expected */
static e_result_t R_PMA_IF_SendMessageToTask(st_msg_to_task_t *pMsg)            
{
    e_result_t LenResult;
    e_osal_return_t LenOsalRet;

    BaseType_t LxReturned;

    uint32_t LulBitsToClearOnEntry;
    uint32_t LulBitsToClearOnExit;
    uint32_t LulNotificationValue;

    LulBitsToClearOnEntry = 0u;
    LulBitsToClearOnExit = 0u;
    LulNotificationValue = 0u;

    if (NULL != pMsg)
    {
        LenResult = R_PMA_OK;
    }
    else
    {
        LenResult = R_PMA_NG;
    }

    if (R_PMA_OK == LenResult)
    {
        pMsg->xUserTaskHandle = xTaskGetCurrentTaskHandle();
        LenOsalRet = R_OSAL_Internal_MqTimedSend(R_PMA_SM_GxTaskQueue, R_PMA_IF_GxWaitTick, (void*)pMsg, sizeof(st_msg_to_task_t));
        if (LenOsalRet == OSAL_RETURN_OK)
        {
            LxReturned = xTaskNotifyWait(LulBitsToClearOnEntry, LulBitsToClearOnExit, &LulNotificationValue, R_PMA_IF_GxWaitTick);
            if (pdTRUE == LxReturned)
            {
                LenResult = (e_result_t)LulNotificationValue;
            }
            else
            {
                LenResult = R_PMA_TIMEOUT;
            }
        }
        else if (LenOsalRet == OSAL_RETURN_TIME)
        {
            LenResult = R_PMA_TIMEOUT;
        }
        else
        {
            LenResult = R_PMA_NG;
        }
    }

    return LenResult;
}

/***********************************************************************************************************************
* End of function R_PMA_IF_SendMessageToTask()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetCurrentState()
***********************************************************************************************************************/
static e_result_t R_PMA_IF_GetCurrentState(st_pma_handle_t     handle,
                                           e_pma_hwa_id_t      hwaId,
                                           st_pma_hwa_state_t  const * pHwaState,
                                           st_pma_hwa_state_t  * pCurHwaState,
                                           e_pma_reset_req_t   * pCurRstStat)
{
    e_result_t          LenResult;
    e_osal_return_t     LenOsalRet;
    
    LenOsalRet = R_OSAL_Internal_MutexTimedLock(R_PMA_SM_GxMutex, R_PMA_IF_GxWaitTick);
        
    if (OSAL_RETURN_OK == LenOsalRet)
    {
        LenResult = R_PMA_SM_GetHwaState(handle, hwaId, pCurHwaState);
        LenOsalRet = R_OSAL_Internal_MutexUnlock(R_PMA_SM_GxMutex);
    
        if ((OSAL_RETURN_OK == LenOsalRet)
             && (R_PMA_OK == LenResult))
        {
            if (R_PMA_HWA_REQUESTED == pHwaState->enReqState)
            {
                LenResult = R_PMA_SM_GetResetState(handle, hwaId, pCurRstStat);
            }
        }
    }
    
    if (OSAL_RETURN_OK != LenOsalRet)
    {
        LenResult = R_PMA_NG;
    }
    
    return LenResult;
}
/***********************************************************************************************************************
* End of function R_PMA_GetCurrentState()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_PMA_GetTimeDiffAndSleep()
***********************************************************************************************************************/
/* PRQA S 3673 1 # Implemented in accordance with the 2nd parameter of R_OSAL_ClockTimeCalculateTimeDifference(). */
static e_osal_return_t R_PMA_IF_GetTimeDiffAndSleep(st_osal_time_t *LstStartTime,
                                                    osal_nano_sec_t   * pDiffTime,
                                                    osal_milli_sec_t  sleepTimePeriod)
{
    e_osal_return_t     LenOsalRet;
    st_osal_time_t      LstCurTime;

    /* PRQA S 0314 1 # The 1st parameter must be (void *) and there is no side side-effect in this case. */
    (void)memset((void*)&LstCurTime, 0, sizeof(st_osal_time_t));

    (void)R_OSAL_Internal_MsSleep(sleepTimePeriod);

    LenOsalRet = R_OSAL_Internal_GetHighResoTimeStamp(&LstCurTime);
    
    if (OSAL_RETURN_OK == LenOsalRet)
    {
        LenOsalRet = R_OSAL_Internal_CalcTimeDifference(&LstCurTime, LstStartTime, pDiffTime);
    }

    return LenOsalRet;
}
/***********************************************************************************************************************
* End of function R_PMA_GetTimeDiffAndSleep()
***********************************************************************************************************************/

/***********************************************************************************************************************
* End of function R_PMA_IF_Exchange_ErrCode()
***********************************************************************************************************************/
/* PRQA S 9106 1 # Since it is created as a common function, calls from multiple functions are expected */
static e_pma_return_t R_PMA_IF_Exchange_ErrCode(e_result_t innerErr)
{
    e_pma_return_t LenRetCode;

    switch(innerErr)
    {
        case R_PMA_OK:
            LenRetCode = R_PMA_RESULT_OK;
            break;

        case R_PMA_INVALID_ARGUMENT:
            LenRetCode = R_PMA_ERR_INVALID_ARGUMENT;
            break;

        case R_PMA_BSP_EINVAL:
        case R_PMA_BSP_ETIMEDOUT:
            LenRetCode = R_PMA_ERR_DEVICE;
            break;

        case R_PMA_BSP_EIO:
            LenRetCode = R_PMA_ERR_IO;
            break;

        case R_PMA_ALREADY_IN_USE:
            LenRetCode = R_PMA_ERR_ALREADY_IN_USE;
            break;

        case R_PMA_INVALID_HANDLE:
            LenRetCode = R_PMA_ERR_INVALID_HANDLE;
            break;

        case R_PMA_TIMEOUT:
            LenRetCode = R_PMA_ERR_TIMEOUT;
            break;

        case R_PMA_NG:
        default:
            LenRetCode = R_PMA_ERR_RESULT_NG;
            break;
    }
    return LenRetCode;
}

/***********************************************************************************************************************
* End of function R_PMA_IF_Exchange_ErrCode()
***********************************************************************************************************************/

/*======================================================================================================================
End of file
======================================================================================================================*/
