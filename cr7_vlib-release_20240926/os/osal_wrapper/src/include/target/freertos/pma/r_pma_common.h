/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022]  Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : Common definition of PM API.
***********************************************************************************************************************/

#ifndef R_PMA_COMMON_H
/** Multiple inclusion protection macro */
#define R_PMA_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#include "rcar-xos/osal/r_osal_types.h"

#include "r_pma_hwdepend.h"

typedef enum 
{
    R_PMA_RESULT_OK,              /*!< Success */
    R_PMA_ERR_RESULT_NG,          /*!< Internal error */
    R_PMA_ERR_ALREADY_IN_USE,     /*!< HWA is already occupied. */
    R_PMA_ERR_INVALID_HANDLE,     /*!< Handle value is Incorrect. */
    R_PMA_ERR_NO_VACANT_HANDLE,   /*!< Number of users exceeds the limit. */
    R_PMA_ERR_INVALID_ARGUMENT,   /*!< Parameter value is Incorrect. */
    R_PMA_ERR_DEVICE,             /*!< Failure on the device side. (HW failure)   */
    R_PMA_ERR_TIMEOUT,            /*!< Timeout situation. */
    R_PMA_ERR_IO,                  /*!< Failure related to input/output operation. */
    R_PMA_ERR_BUS_IF_CHK_NG, //Temporarily added
    R_PMA_ERR_RESET_CHK_NG, //Temporarily added
    R_PMA_ERR_RESET_TOGGLE_CHK_NG //Temporarily added
} e_pma_return_t;

typedef enum 
{
    R_PMA_STATE_OFF,              /*!< Power or clock domain status is OFF. */
    R_PMA_STATE_ON,               /*!< Power or clock domain status is ON. */
    R_PMA_UNDEFINED_STATE         /*!< Power or clock domain status is not defined */
} e_pma_onoff_t;

typedef enum 
{
    R_PMA_HWA_RELEASED,           /*!< HWA is not ready. */
    R_PMA_HWA_REQUESTED           /*!< HWA is ready. */
} e_pma_hwa_req_t;

typedef enum 
{
    R_PMA_PG_MODE,                /*!< Power Policy is "PG-Mode". */
    R_PMA_CG_MODE,                /*!< Power Policy is "CG-Mode". */
    R_PMA_HP_MODE                 /*!< Power Policy is "HP-Mode". */
} e_pma_power_policy_t;

typedef enum 
{
    R_PMA_RELEASED_RESET,         /*!< Software Reset is released. */
    R_PMA_APPLIED_RESET,          /*!< Software Reset is applied. */
    R_PMA_UNDEFINED_RESET         /*!< Software Reset is not defined. */
} e_pma_reset_req_t;
typedef enum 
{
    R_PMA_STATE_PG,               /*!< Power State is Power Gated. */
    R_PMA_STATE_CG,               /*!< Power State is Clock Gated. */
    R_PMA_STATE_ENABLED           /*!< Power State is Enabled. */
} e_pma_power_state_t;

typedef enum 
{
    R_PMA_CLK_DISABLE,            /*!< Disable the POST clock. */
    R_PMA_CLK_ENABLE,             /*!< Enable the POST clock. */
} e_pma_clk_req_t;
    
typedef enum
{
    R_PMA_CPG_CLOCK_OFF,          /*!< Clock Off */
    R_PMA_CPG_CLOCK_ON            /*!< Clock On */
} e_pma_clock_onoff_t;
    
typedef enum
{
    R_PMA_SYSC_POWER_OFF,        /*!< Power Off */
    R_PMA_SYSC_POWER_ON          /*!< Power On */
} e_pma_power_onoff_t;
    

/** structure */
/* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct
{
    e_pma_hwa_req_t enReqState;         /*!< User's requirement of HWA. */
    e_pma_onoff_t   enActualPwrStat;    /*!< Actual status of SYSC register. */
    e_pma_onoff_t   enActualClkStat;    /*!< Actual status of CPG register. */
} st_pma_hwa_state_t;

/* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct
{
    uint32_t task_priority;             /*!< Priority of OSAL PM API task */
    uint32_t timeout_ms;                /*!< Wait time[ms] for return of OS API */
    uint32_t polling_max_num;           /*!< The number of times to check register */
    uint32_t loop_num_for_rclk_cycle;   /*!< The number of busy loops to wait for 1 cycle by RCLK */
} st_pma_config_t;

/* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct
{
    uint32_t    task_priority;            /*!< Priority of PMA task */
    uint32_t    timeout_ticks;            /*!< wait ticks for return of OS API */
    uint32_t    polling_max_num;          /*!< the number of times to check register */
    uint32_t    loop_num_for_rclk_cycle;  /*!< the number of busy loop to wait for RCLK 1cycle
                                               until a software reset completion */
} st_pma_param_t;

/* PRQA S 3630 1 # This definition must be published because the function references the member after calling */
typedef struct
{
    bool isUsed;    /** flag to indicate that the handle is used */
} st_pma_user_info_t;

/** typedef */
typedef st_pma_user_info_t * st_pma_handle_t;  /*!< Type of OSAL PM API handle */

#ifdef __cplusplus
}
#endif
#endif /** R_PMA_COMMON_H */
