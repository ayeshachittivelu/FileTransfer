/*************************************************************************************************************
* cpg_api_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_CPG_API_H_
#define R_CPG_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup     CPGgroup CPG driver
 * @ingroup drivers
 * @brief   CPG API (Clock Pulse Generator)
 * @{
 * @}
 */

/**
 * \defgroup CPGtypes   Data Types
 * @ingroup CPGgroup
 * @{
 */

/**
 * @enum    r_cpg_RegIdx_t
 * @brief   List of register index used in RMSTPCRn/MSTPSRn/SRCRn/SRSTCLRn
 */
typedef enum {
    R_CPG_REGIDX_0,
    R_CPG_REGIDX_1,
    R_CPG_REGIDX_2,
    R_CPG_REGIDX_3,
    R_CPG_REGIDX_4,
    R_CPG_REGIDX_5,
    R_CPG_REGIDX_6,
    R_CPG_REGIDX_7,
    R_CPG_REGIDX_8,
    R_CPG_REGIDX_9,
    R_CPG_REGIDX_10,
    R_CPG_REGIDX_11,
    R_CPG_REGIDX_12,
    R_CPG_REGIDX_13,
    R_CPG_REGIDX_14,
    R_CPG_REGIDX_15,
    R_CPG_REGIDX_16,
    R_CPG_REGIDX_17,
    R_CPG_REGIDX_18,
    R_CPG_REGIDX_19,
    R_CPG_REGIDX_20,
} r_cpg_RegIdx_t;

/**
 * @enum    r_cpg_BitPos_t
 * @brief   List of bit number
 */
typedef enum {
    R_CPG_BIT0,
    R_CPG_BIT1,
    R_CPG_BIT2,
    R_CPG_BIT3,
    R_CPG_BIT4,
    R_CPG_BIT5,
    R_CPG_BIT6,
    R_CPG_BIT7,
    R_CPG_BIT8,
    R_CPG_BIT9,
    R_CPG_BIT10,
    R_CPG_BIT11,
    R_CPG_BIT12,
    R_CPG_BIT13,
    R_CPG_BIT14,
    R_CPG_BIT15,
    R_CPG_BIT16,
    R_CPG_BIT17,
    R_CPG_BIT18,
    R_CPG_BIT19,
    R_CPG_BIT20,
    R_CPG_BIT21,
    R_CPG_BIT22,
    R_CPG_BIT23,
    R_CPG_BIT24,
    R_CPG_BIT25,
    R_CPG_BIT26,
    R_CPG_BIT27,
    R_CPG_BIT28,
    R_CPG_BIT29,
    R_CPG_BIT30,
    R_CPG_BIT31,
} r_cpg_BitPos_t;

/**
 * @enum    r_cpg_Error_t
 * @brief   list of errors CPG API can return
 */
typedef enum r_cpg_Error_st {
    R_CPG_ERR_SUCCESS,             /**< success */
    R_CPG_ERR_INVALID_PARAMETER,   /**< invalid parameter */
    R_CPG_ERR_FAILED,              /**< failed */
} r_cpg_Error_t;

/**
 * @enum    r_cpg_ClockName_t
 * @brief   List of clock name
 */
typedef enum {
    R_CPG_ZB3_CLOCK,      /* ZB3 Clock   */
    R_CPG_SD0H_CLOCK,     /* SD0H Clock   */
    R_CPG_SD0_CLOCK,      /* SD0 Clock    */
    R_CPG_SD1H_CLOCK,     /* SD1H Clock   */
    R_CPG_SD1_CLOCK,      /* SD1 Clock    */
    R_CPG_SD2H_CLOCK,     /* SD2H Clock   */
    R_CPG_SD2_CLOCK,      /* SD2 Clock    */
    R_CPG_SD3H_CLOCK,     /* SD3H Clock   */
    R_CPG_SD3_CLOCK,      /* SD3 Clock    */
    R_CPG_SSPSRC_CLOCK,   /* SSPSRC Clock */
    R_CPG_SSPRS_CLOCK,    /* SSPRS Clock  */
    R_CPG_RPC_CLOCK,      /* RPC Clock    */
    R_CPG_RPCD2_CLOCK,    /* RPCD2 Clock  */
    R_CPG_MSO_CLOCK,      /* MSO Clock    */
    R_CPG_CANFD_CLOCK,    /* CANFD Clock  */
    R_CPG_HDMI_CLOCK,     /* HDMI Clock   */
    R_CPG_CSI0_CLOCK,     /* CSI0 Clock   */
    R_CPG_DCU_CLOCK,      /* DCU Clock    */
    R_CPG_ADG_CLOCK,      /* Audio ADG Clock    */
    R_CPG_SSI0_CLOCK,      /* SSI0 Clock    */
    R_CPG_SSI1_CLOCK,      /* SSI1 Clock    */
    R_CPG_SSI2_CLOCK,      /* SSI2 Clock    */
    R_CPG_SSI3_CLOCK,      /* SSI3 Clock    */
    R_CPG_SSI4_CLOCK,      /* SSI4 Clock    */
    R_CPG_SSI5_CLOCK,      /* SSI5 Clock    */
    R_CPG_SSI6_CLOCK,      /* SSI6 Clock    */
    R_CPG_SSI7_CLOCK,      /* SSI7 Clock    */
    R_CPG_SSI8_CLOCK,      /* SSI8 Clock    */
    R_CPG_SSI9_CLOCK,      /* SSI9 Clock    */
    R_CPG_SSI_ALL_CLOCK,   /* SSI ALL Clock */
    R_CPG_SDIF0_CLOCK,     /* SDIF0 Clock   */
    R_CPG_SDIF1_CLOCK,     /* SDIF1 Clock   */
    R_CPG_SDIF2_CLOCK,     /* SDIF2 Clock   */
    R_CPG_SDIF3_CLOCK,     /* SDIF3 Clock   */
    R_CPG_CLOCK_LAST
} r_cpg_ClockName_t;

/** @} data types */
/**
 * \defgroup CPGapi   API
 * @ingroup CPGgroup
 * @{
 */

/**

 * Function R_PMA_CPG_Init
 * @brief Initialise of CPG
 */
void R_PMA_CPG_Init(void);

/**
 * Function R_PMA_CPG_Quit
 * @brief DeInitialise of CPG
 */
void R_PMA_CPG_Quit(void);

/**
 * Function R_PMA_CPG_SetModuleStopStat
 * @brief Set a state of the module stop for specified module
 *
 * @param[in]   RegIdx - RMSTPCR register index, range R_CPG_REGIDX_0 - R_CPG_REGIDX_11.
 *                       See @ref r_cpg_RegIdx_t
 * @param[in]   BitPos - RMSTPCR register bit number, range R_CPG_BIT0 - R_CPG_BIT31.
 *                       See @ref r_cpg_BitPos_t
 * @param[in]   enable - true: Enable Module Stop, false: Disable Module Stop
 * @retval      See  @ref r_cpg_Error_t
 *
 * @note In this API, it is possible to specify the Bit of the Module described
 * in Table 8A.15 to Table 8A.26 of HWM.
 * In addition, even if you specify a Bit that does not have Module described
 * in the table, no error will occur.
 */
r_cpg_Error_t R_PMA_CPG_SetModuleStopStat(r_cpg_RegIdx_t RegIdx,
                                           r_cpg_BitPos_t BitPos,
                                           bool enable);

/**
 * Function R_PMA_CPG_GetModuleStopStat
 * @brief Get a state of the module stop for specified module
 *
 * @param[in]   RegIdx - RMSTPCR register index, range R_CPG_REGIDX_0 - R_CPG_REGIDX_11.
 *                       See @ref r_cpg_RegIdx_t
 * @param[in]   BitPos - RMSTPCR register bit number, range R_CPG_BIT0 - R_CPG_BIT31.
 *                       See @ref r_cpg_BitPos_t
 * @param[out]  enable - true: Module Stop is enabled, false: Module Stop is disabled
 * @retval      See  @ref r_cpg_Error_t
 */
r_cpg_Error_t R_PMA_CPG_GetModuleStopStat(r_cpg_RegIdx_t RegIdx,
                                           r_cpg_BitPos_t BitPos,
                                           bool* enable);

/**
 * Function R_PMA_CPG_SetResetStat
 * @brief Set a state of the CPG reset for specified module
 *
 * @param[in]   RegIdx - SRCR/SRSTCLR register index, range R_CPG_REGIDX_0 - R_CPG_REGIDX_11.
 *                       See @ref r_cpg_RegIdx_t
 * @param[in]   BitPos - SRCR/SRSTCLR register bit number, range R_CPG_BIT0 - R_CPG_BIT31.
 *                       See @ref r_cpg_BitPos_t
 * @param[in]   ResetStat - 1: Set SRCR target bit to 1.
 *     0: Set the SRSTCLR target bit to 1 and wait for the SRCR bit to become 0.
 *   It is necessary to wait for RCLK 1 cycle on the user side.
 * @retval      See  @ref r_cpg_Error_t
 *
 * @note In this API, it is possible to specify the Bit of the Module described
 * in Table 8A.27 to Table 8A.38 of HWM.
 * In addition, even if you specify a Bit that does not have Module described
 * in the table, no error will occur.
 */
r_cpg_Error_t R_PMA_CPG_SetResetStat(r_cpg_RegIdx_t RegIdx,
                                      r_cpg_BitPos_t BitPos,
                                      uint32_t ResetStat);

/**
 * Function R_PMA_CPG_GetResetStat
 * @brief Get a state of the CPG reset for specified module
 *
 * @param[in]   RegIdx    - SRCR register index, range R_CPG_REGIDX_0 - R_CPG_REGIDX_11.
 *                          See @ref r_cpg_RegIdx_t
 * @param[in]   BitPos    - SRCR register bit number, range R_CPG_BIT0 - R_CPG_BIT31.
 *                          See @ref r_cpg_BitPos_t
 * @param[out]  ResetStat - Reset status, 0: SRCR target bit is 0
 *                                        1: SRCR target bit is 1
 * @retval      See  @ref r_cpg_Error_t
 */
r_cpg_Error_t R_PMA_CPG_GetResetStat(r_cpg_RegIdx_t RegIdx,
                                      r_cpg_BitPos_t BitPos,
                                      uint32_t* ResetStat);

/**
 * Function R_PMA_CPG_SetClockStopStat
 * @brief Set a state of the clock stop for specified module
 *
 * @param[in]   ClockName - Clock name of CPG, range R_CPG_ZB3_CLOCK - R_CPG_CSI0_CLOCK.
 *                          See @ref r_cpg_ClockName_t
 * @param[in]   enable    - true: Enable Clock Stop, false: Disable Clock Stop
 * @retval      See  @ref r_cpg_Error_t
 */
r_cpg_Error_t R_PMA_CPG_SetClockStopStat(r_cpg_ClockName_t ClockName,
                                          bool enable);

/**
 * Function R_PMA_CPG_GetClockStopStat
 * @brief Get a state of the clock stop for specified module
 *
 * @param[in]   ClockName - Clock name of CPG, range R_CPG_ZB3_CLOCK - R_CPG_CSI0_CLOCK.
 *                          See @ref r_cpg_ClockName_t
 * @param[out]  enable    - true: Clock is stopped, false: Clock is running
 * @retval      See  @ref r_cpg_Error_t
 */
r_cpg_Error_t R_PMA_CPG_GetClockStopStat(r_cpg_ClockName_t ClockName,
                                          bool* enable);

/** @} api */

#ifdef __cplusplus
}
#endif

#endif /* R_CPG_API_H_ */
