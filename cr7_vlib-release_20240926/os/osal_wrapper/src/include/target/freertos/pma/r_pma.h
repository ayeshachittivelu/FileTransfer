/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : interface definition for OSAL.
***********************************************************************************************************************/

#ifndef R_PMA_H
/** Multiple inclusion protection macro */
#define R_PMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "r_pma_common.h"

/** Function definition */

e_pma_return_t R_PMA_Init(st_pma_handle_t *pHandle, const st_pma_config_t *pConfig);
e_pma_return_t R_PMA_Quit(st_pma_handle_t handle);
e_pma_return_t R_PMA_GetLockHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_pma_return_t R_PMA_UnlockHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_pma_return_t R_PMA_RequestHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_pma_return_t R_PMA_ReleaseHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_pma_return_t R_PMA_GetHwaState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, st_pma_hwa_state_t *pHwaState);
e_pma_return_t R_PMA_SetPowerPolicy(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_power_policy_t policy);
e_pma_return_t R_PMA_GetPowerPolicy(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_power_policy_t *pPolicy);
e_pma_return_t R_PMA_ApplyResetHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_pma_return_t R_PMA_ReleaseResetHwa(st_pma_handle_t handle, e_pma_hwa_id_t hwaId);
e_pma_return_t R_PMA_GetReset(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_reset_req_t *pRstStat);
e_pma_return_t R_PMA_GetLowestPowerState(st_pma_handle_t handle, e_pma_hwa_id_t hwaId, e_pma_power_state_t *pLowestPower);
e_pma_return_t R_PMA_WaitForState(st_pma_handle_t const handle, e_pma_hwa_id_t hwaId, st_pma_hwa_state_t const * pHwaState, e_pma_reset_req_t rstStat, osal_milli_sec_t sleepTimePeriod, osal_milli_sec_t timeoutMsPeriod, osal_nano_sec_t timeoutNsPeriod);
e_pma_return_t R_PMA_SetPostClock(st_pma_handle_t const handle, e_pma_clk_req_t  clockEnable, uint32_t chanIdx, uint32_t const * pClockDivider);

#ifdef __cplusplus
}
#endif
#endif /** R_PMA_H */
