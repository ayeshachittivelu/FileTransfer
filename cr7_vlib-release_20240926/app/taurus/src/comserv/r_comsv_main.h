/*************************************************************************************************************
* taurus_comsv_main_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef R_COMSV_MAIN_H
#define R_COMSV_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function Name: R_COMSV_Init
 */
/**
 * @brief Init communication server
 *
 * @param[in] Guest       - Guest ID
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_COMSV_Init(int Guest);

/*******************************************************************************
 * Function Name: R_COMSV_DeInit
 */
/**
 * @brief Deinit communication server
 *
 * @param[in] Guest       - Guest ID
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
int R_COMSV_DeInit(int Guest);

#ifdef __cplusplus
}
#endif

#endif /* R_COMSV_MAIN_H */
