/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022]  Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : Internal function definition for PM API
***********************************************************************************************************************/

#ifndef R_PMA_STATEMACHINE_H
/** Multiple inclusion protection macro */
#define R_PMA_STATEMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "r_pma_common.h"

#include "target/freertos/r_osal_common.h"

#define R_PMA_COMMON_CLK_POSTCKCR   (4u)
#define R_PMA_COMMON_CLK_DIVIDER    (64u)

/** for osal_wrapper*/
#define R_PMA_QUE_HANDLE    osal_inner_mq_handle_t
#define R_PMA_SEM_HANDLE    osal_inner_mutex_handle_t
#define R_PMA_TSK_HANDLE    osal_inner_thread_handle_t
#define R_PMA_PRIORITY      e_osal_thread_priority_t
#define R_PMA_TIMEOUT_T     osal_milli_sec_t

typedef enum
{
    R_PMA_OK,                     /*!< OK */
    R_PMA_NG,                     /*!< Fatal error */
    R_PMA_ALREADY_IN_USE,         /*!< Another user occupy HWA */
    R_PMA_INVALID_ARGUMENT,       /*!< Argument is invalid */
    R_PMA_INVALID_HANDLE,         /*!< Handle is invalid */
    R_PMA_TIMEOUT,                /*!< Timeout of OS API or polling register */
    R_PMA_BSP_EINVAL,             /*!< Invalid situation of BSP */
    R_PMA_BSP_ETIMEDOUT,          /*!< Timeout situation of BSP */
    R_PMA_BSP_EIO,                /*!< Failure related to input/output operation of BSP */
    R_PMA_RESULT_TYPE_NUM         /*!< The number of result code */
} e_result_t;

typedef enum
{
    R_PMA_PROCESS_ID_REQUEST_HWA,    /*!< Resume HWA */
    R_PMA_PROCESS_ID_RELEASE_HWA,    /*!< Suspend HWA */
    R_PMA_PROCESS_ID_APPLY_RESET,    /*!< Apply software reset */
    R_PMA_PROCESS_ID_RELEASE_RESET,  /*!< Clear software reset */
    R_PMA_PROCESS_ID_CHANGE_POLICY,  /*!< Change power policy */
    R_PMA_PROCESS_ID_CHANGE_POLICY_IMMEDIATE,  /*!< Change power policy */
    R_PMA_PROCESS_ID_EXIT_THREAD,    /*!< Terminate thread */
    R_PMA_PROCESS_ID_NUM             /*!< The number of process */
} e_task_proc_id_t;

typedef struct
{
    e_task_proc_id_t enProcId;        /** Process ID of Statemachine */
    e_pma_hwa_id_t   enHwaId;         /** HWA ID */
    e_pma_power_policy_t enPolicy;    /** Power Policy */
    st_pma_handle_t  stPmaHandle;     /** Handle of PMA */
    xTaskHandle xUserTaskHandle; /** User task handle */
} st_msg_to_task_t;

/* PRQA S 3630 1 # This definition must be published because the function references the member after calling */
typedef struct st_sysc_domain_t_
{
    e_sysc_domain_id_t         id;         /** SYSC domain ID */
    uint32_t                   refCnt;     /** Reference counter */
    struct st_sysc_domain_t_ * pDepDomain; /** Parent SYSC domain */
} st_sysc_domain_t;

/* PRQA S 3630 1 # This definition must be published because the function references the member after calling */
typedef struct st_cpg_domain_t_
{
    e_cpg_domain_id_t         id;         /** CPG domain ID */
    uint32_t                  refCnt;     /** Reference counter */
    e_pma_reset_req_t         enRstStat;  /** Reset status */
    struct st_cpg_domain_t_ * pDepDomain; /** Parent CPG domain */
} st_cpg_domain_t;

/* PRQA S 3630 1 # This definition must be published because the function references the member after calling */
typedef struct
{
    st_pma_handle_t      handle;        /** User of HWA */
    e_pma_hwa_req_t      enReqStat;     /** User's Requirement status to HWA */
    uint64_t             refCnt;        /** Reference counter */
    e_pma_power_policy_t enPwrPolicy;   /** Current Power Policy */
    e_pma_power_policy_t enPrevStbMode; /** Previous stanby mode */
    e_pma_power_policy_t enApparentMode;/** Apparent mode */
    st_sysc_domain_t   * pSyscDomain;   /** Dependent SYSC domain */
    st_cpg_domain_t    * pCpgDomain;    /** Dependent CPG domain */
    e_pma_power_state_t  enLowestPwrState;
} st_hwa_t;

typedef struct
{
    e_sysc_domain_id_t enDepSyscId; /** Depedent SYSC */
    e_cpg_domain_id_t  enDepCpgId;  /** Depedent CPG */
} st_hwa_dep_info_t;

typedef e_result_t (*p_chg_hwa_stat_func_t)(const st_hwa_t *pHwa);

typedef struct
{
    p_chg_hwa_stat_func_t funcToPG;
    p_chg_hwa_stat_func_t funcToReady;
} st_hwa_func_tbl_t;

extern R_PMA_QUE_HANDLE R_PMA_SM_GxTaskQueue;
extern R_PMA_TSK_HANDLE R_PMA_SM_GxThread;
extern R_PMA_SEM_HANDLE R_PMA_SM_GxMutex;
extern uint64_t pma_soc_type;
extern e_pma_hwa_id_t hwa_idmax;
extern e_pma_hwa_id_t hwa_idmin;
extern e_cpg_domain_id_t cpg_idmax;
extern e_cpg_domain_id_t cpg_idmin;
extern e_sysc_domain_id_t sysc_idmax;
extern e_sysc_domain_id_t sysc_idmin;

extern e_result_t R_PMA_SM_Init(R_PMA_QUE_HANDLE *pTaskQueue, const st_pma_param_t *pParam);
extern e_result_t R_PMA_SM_GetLockHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
extern e_result_t R_PMA_SM_UnlockHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
extern e_result_t R_PMA_SM_GetHwaState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, st_pma_hwa_state_t *pHwaState);
extern e_result_t R_PMA_SM_GetResetState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_reset_req_t *pRstState);
extern e_result_t R_PMA_SM_SetPowerPolicy(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_power_policy_t policy);
extern e_result_t R_PMA_SM_GetPowerPolicy(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_power_policy_t *pPolicy);
st_sysc_domain_t *R_PMA_SM_GetSyscDomain(e_sysc_domain_id_t syscId);
st_cpg_domain_t *R_PMA_SM_GetCpgDomain(e_cpg_domain_id_t cpgId);
st_hwa_t *R_PMA_SM_GetHwa(e_pma_hwa_id_t hwaId);
e_result_t R_PMA_SM_SetClockStat(st_cpg_domain_t * pCpgDomain, bool bStat);
e_result_t R_PMA_SM_SetPowerStat(st_sysc_domain_t * pSyscDomain, bool bStat);
extern e_result_t R_PMA_SM_GetLowestPowerState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_power_state_t *pLowestPower);
extern e_result_t R_PMA_SM_SetPostClock(e_pma_clk_req_t clockEnable, uint32_t chanIdx, uint32_t const * pClockDivider);

extern e_result_t R_PMA_SM_InitSyscDomain(void);
extern e_result_t R_PMA_SM_InitCpgDomain(void);
extern e_result_t R_PMA_SM_InitHwa(void);
extern e_result_t R_PMA_SM_ChangeToCG(const st_hwa_t *pHwa);
extern e_result_t R_PMA_SM_PowerONA3IR(void);
extern e_result_t R_PMA_SM_HwStateOff(void);

#ifdef __cplusplus
}
#endif
#endif /** R_PMA_STATEMACHINE_H */
