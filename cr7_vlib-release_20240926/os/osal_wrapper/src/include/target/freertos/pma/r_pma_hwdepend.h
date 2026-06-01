/*************************************************************************************************************
* Power Management Agent 
* Copyright (c) [2021-2022]  Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* Version      : 1.0.0
* Description  : Hardware dependant function for PM API
***********************************************************************************************************************/

#ifndef R_PMA_HWDEPEND_H
/** Multiple inclusion protection macro */
#define R_PMA_HWDEPEND_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************************************************//**
 * @enum e_pma_hwa_id_t
 * Hardware accelerator id.
***********************************************************************************************************************/
typedef enum
{
    R_PMA_HWA_ID_IVDP1C,
    R_PMA_HWA_ID_FCPC,
    R_PMA_HWA_ID_IMR0,
    R_PMA_HWA_ID_IMR1,
    R_PMA_HWA_ID_IMR2,
    R_PMA_HWA_ID_IMR3,
    R_PMA_HWA_ID_DHD,
    R_PMA_HWA_ID_DU0,
    R_PMA_HWA_ID_DU1,
    R_PMA_HWA_ID_DU2,
    R_PMA_HWA_ID_DU3,
    R_PMA_HWA_ID_VSPD0,
    R_PMA_HWA_ID_VSPD1,
    R_PMA_HWA_ID_VSPD2,
    R_PMA_HWA_ID_VIN0,
    R_PMA_HWA_ID_VIN1,
    R_PMA_HWA_ID_VIN2,
    R_PMA_HWA_ID_VIN3,
    R_PMA_HWA_ID_VIN4,
    R_PMA_HWA_ID_VIN5,
    R_PMA_HWA_ID_VIN6,
    R_PMA_HWA_ID_VIN7,
    R_PMA_HWA_ID_CSI20,
    R_PMA_HWA_ID_CSI40,
    R_PMA_HWA_ID_CSI41,
    R_PMA_HWA_ID_DOC0,
    R_PMA_HWA_ID_DOC1,
    R_PMA_HWA_ID_HDMI0,
    R_PMA_HWA_ID_HDMI1,
    R_PMA_HWA_ID_LVDS,
    R_PMA_HWA_ID_CANFD,
    R_PMA_HWA_ID_ETHERNET,
    R_PMA_HWA_ID_RTDMAC0,
    R_PMA_HWA_ID_RTDMAC1,
    R_PMA_HWA_ID_AUDMAC0,
    R_PMA_HWA_ID_AUDMAC1,
    R_PMA_HWA_ID_NUM,
    R_PMA_HWA_ID_INVALID = 0xFFFFU
} e_pma_hwa_id_t;

/*******************************************************************************************************************//**
 * @enum e_pma_hwa_req_t
 * Requirement of Hardware accelerator.
***********************************************************************************************************************/
/***********************************************************************************************************************
 * @enum e_pma_hwa_req_t
***********************************************************************************************************************/
typedef enum
{
    R_PMA_CPG_DOMAIN_ID_IVDP1C = 0,
    R_PMA_CPG_DOMAIN_ID_FCPC,
    R_PMA_CPG_DOMAIN_ID_IMR0,
    R_PMA_CPG_DOMAIN_ID_IMR1,
    R_PMA_CPG_DOMAIN_ID_IMR2,
    R_PMA_CPG_DOMAIN_ID_IMR3,
    R_PMA_CPG_DOMAIN_ID_DHD,
    R_PMA_CPG_DOMAIN_ID_DU0,
    R_PMA_CPG_DOMAIN_ID_DU1,
    R_PMA_CPG_DOMAIN_ID_DU2,
    R_PMA_CPG_DOMAIN_ID_DU3,
    R_PMA_CPG_DOMAIN_ID_VSPD0,
    R_PMA_CPG_DOMAIN_ID_VSPD1,
    R_PMA_CPG_DOMAIN_ID_VSPD2,
    R_PMA_CPG_DOMAIN_ID_VIN0,
    R_PMA_CPG_DOMAIN_ID_VIN1,
    R_PMA_CPG_DOMAIN_ID_VIN2,
    R_PMA_CPG_DOMAIN_ID_VIN3,
    R_PMA_CPG_DOMAIN_ID_VIN4,
    R_PMA_CPG_DOMAIN_ID_VIN5,
    R_PMA_CPG_DOMAIN_ID_VIN6,
    R_PMA_CPG_DOMAIN_ID_VIN7,
    R_PMA_CPG_DOMAIN_ID_CSI20,
    R_PMA_CPG_DOMAIN_ID_CSI40,
    R_PMA_CPG_DOMAIN_ID_CSI41,
    R_PMA_CPG_DOMAIN_ID_DOC0,
    R_PMA_CPG_DOMAIN_ID_DOC1,
    R_PMA_CPG_DOMAIN_ID_HDMI0,
    R_PMA_CPG_DOMAIN_ID_HDMI1,
    R_PMA_CPG_DOMAIN_ID_LVDS,
    R_PMA_CPG_DOMAIN_ID_CANFD,
    R_PMA_CPG_DOMAIN_ID_ETHERNET,
    R_PMA_CPG_DOMAIN_ID_RTDMAC0,
    R_PMA_CPG_DOMAIN_ID_RTDMAC1,
    R_PMA_CPG_DOMAIN_ID_AUDMAC0,
    R_PMA_CPG_DOMAIN_ID_AUDMAC1,
    R_PMA_CPG_DOMAIN_ID_NUM,                    /** the number of CPG domain */
    R_PMA_CPG_DOMAIN_ID_NONE,                   /** No depedent CPG domain */
    R_PMA_CPG_DOMAIN_ID_INVALID
} e_cpg_domain_id_t;
/***********************************************************************************************************************
 * @enum e_sysc_domain_id_t
***********************************************************************************************************************/
typedef enum
{
    R_PMA_SYSC_DOMAIN_ID_A3IR,
    R_PMA_SYSC_DOMAIN_ID_A3VP,
    R_PMA_SYSC_DOMAIN_ID_A3VC,
    R_PMA_SYSC_DOMAIN_ID_A2VC1,
    R_PMA_SYSC_DOMAIN_ID_NUM,                   /** the number of SYSC domain */
    R_PMA_SYSC_DOMAIN_ID_NONE                   /** No depedent SYSC domain */
} e_sysc_domain_id_t;

#define CPG_MSTP_REG_BIT_TO_ID(_reg, _bit)   ( (((_reg) & 0xFF) << 8) | ((_bit) & 0xFF) )
#define CPG_MSTP_ID_TO_REG_OFFSET(_id)       ( ((_id) >> 8) & 0xFF )
#define CPG_MSTP_ID_TO_BIT(_id)              ( (_id) & 0xFF )

#ifdef __cplusplus
}
#endif
#endif /** R_PMA_HWDEPEND_H */
