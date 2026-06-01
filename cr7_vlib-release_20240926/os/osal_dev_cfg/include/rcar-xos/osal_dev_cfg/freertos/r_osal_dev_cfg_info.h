/*************************************************************************************************************
* OSAL Device Configurator
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_dev_cfg_info.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL device configuration header file
***********************************************************************************************************************/

#ifndef OSAL_DEV_CFG_INFO_H
#define OSAL_DEV_CFG_INFO_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "pma/r_pma_hwdepend.h"

/*******************************************************************************************************************//**
 * @defgroup OSAL_WRAPPER_FREERTOS_DEV_CFG OSAL Wrapper Device Configurator For FREERTOS
 * @ingroup osal_wrapper
 * OSAL Wrapper Device Configurator for FREERTOS
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Defines_DevConfig_info Device Configurator Public macro definitions (info)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_INVALID_ID
 * Invalid Device ID.
***********************************************************************************************************************/
#define    OSAL_DEVICE_INVALID_ID    (NULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_V3M
 * SoC type is R-Car V3M 
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_V3M        (0x56334d0000000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_V3H1
 * SoC type is R-Car V3H 1.x
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_V3H1       (0x5633483100000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_V3H2
 * SoC type is R-Car V3H 2.x
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_V3H2       (0x5633483200000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_V3U
 * SoC type is R-Car V3U
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_V3U        (0x5633550000000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_H3
 * SoC type is R-Car V3U
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_H3         (0x4833000000000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_M3
 * SoC type is R-Car M3 (M3-W)
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_M3         (0x4d33000000000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_M3N
 * SoC type is R-Car M3N
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_M3N        (0x4d334e0000000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_E3
 * SoC type is R-Car E3
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_E3         (0x4533000000000000ULL)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_SOC_TYPE_V3U
 * SoC type is R-Car V3U
***********************************************************************************************************************/
#define OSAL_DEVICE_SOC_TYPE_INVALID    (0xFFFFFFFFFFFFFFFFULL)

/** @} */

/*******************************************************************************************************************//**
 * @struct st_osal_device_soc_type_t
 * structure to hold SoC type
***********************************************************************************************************************/
typedef struct
{
    uint64_t    device_soc_type;    /*!< Device SoC type */
} st_osal_device_soc_type_t;

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Enum_DevConfig_info Device Configurator Public Enumration (info)
 *
 * @{
 **********************************************************************************************************************/

/* PoC implementation */
/*******************************************************************************************************************//**
 * @enum e_osal_pm_id_t
 * PMA ID definitions.
***********************************************************************************************************************/
#if 0
typedef enum e_osal_pm_id
{
    R_PMA_HWA_ID_IMP_TOP,           /*!< IMP TOP */
    R_PMA_HWA_ID_IMP_MASTER,        /*!< IMP LDMAC-MASTER */
    R_PMA_HWA_ID_IMP_CHECKER,       /*!< IMP LDMAC-CHECKER */
    R_PMA_HWA_ID_IMP_SPMI0_00,      /*!< IMP SPMI0 */
    R_PMA_HWA_ID_IMP_SPMI1_01,      /*!< IMP SPMI1 */
    R_PMA_HWA_ID_IMP_SPMC0_00,      /*!< IMP SPMC0 */
    R_PMA_HWA_ID_IMP_SPMC1_01,      /*!< IMP SPMC1 */
    R_PMA_HWA_ID_IMP_SPMC2_02,      /*!< IMP SPMC2 */
    R_PMA_HWA_ID_IMP_CORE0,         /*!< IMP0 */
    R_PMA_HWA_ID_IMP_CORE1,         /*!< IMP1 */
    R_PMA_HWA_ID_IMP_CORE2,         /*!< IMP2 */
    R_PMA_HWA_ID_IMP_CORE3,         /*!< IMP3 */
    R_PMA_HWA_ID_IMP_OCV_CORE0,     /*!< IMP-OCV0 */
    R_PMA_HWA_ID_IMP_OCV_CORE1,     /*!< IMP-OCV1 */
    R_PMA_HWA_ID_IMP_OCV_CORE2,     /*!< IMP-OCV2 */
    R_PMA_HWA_ID_IMP_OCV_CORE3,     /*!< IMP-OCV3 */
    R_PMA_HWA_ID_IMP_OCV_CORE4,     /*!< IMP-OCV4 */
    R_PMA_HWA_ID_IMP_OCV_CORE5,     /*!< IMP-OCV5 */
    R_PMA_HWA_ID_IMP_OCV_CORE6,     /*!< IMP-OCV6 */
    R_PMA_HWA_ID_IMP_OCV_CORE7,     /*!< IMP-OCV7 */
    R_PMA_HWA_ID_IMP_DMAC0,         /*!< IMP DMA0 */
    R_PMA_HWA_ID_IMP_DMAC1,         /*!< IMP DMA1 */
    R_PMA_HWA_ID_IMP_DMAC_SLIM0,    /*!< IMP DMAC SLIM0 */
    R_PMA_HWA_ID_IMP_PSC0,          /*!< IMP PSC0 */
    R_PMA_HWA_ID_IMP_PSC1,          /*!< IMP PSC1 */
    R_PMA_HWA_ID_IMP_CNN0,          /*!< IMP CNN0 */
    R_PMA_HWA_ID_IMP_CNN1,          /*!< IMP CNN1 */
    R_PMA_HWA_ID_IMP_CNN2,          /*!< IMP CNN2 */
    R_PMA_HWA_ID_IMP_SLIM0,         /*!< IMP SLIM0 */
    R_PMA_HWA_ID_IMP_SPM,           /*!< IMP SPM */
    R_PMA_HWA_ID_IMP_SPMI0,         /*!< IMP RAM0 */
    R_PMA_HWA_ID_IMP_SPMI1,         /*!< IMP RAM1 */
    R_PMA_HWA_ID_IMP_SPMC0,         /*!< IMP CNN RAM0 */
    R_PMA_HWA_ID_IMP_SPMC1,         /*!< IMP CNN RAM1 */
    R_PMA_HWA_ID_IMP_SPMC2,         /*!< IMP CNN RAM2 */
    R_PMA_HWA_ID_IMP_DTA,           /*!< IMP DTA */
    R_PMA_HWA_ID_IMP_RADSP0,        /*!< IMP RADSP0 */
    R_PMA_HWA_ID_IMP_RADSP1,        /*!< IMP RADSP1 */
    R_PMA_HWA_ID_IMP_RADSP_DMAC0,   /*!< IMP RADSP DMAC0 */
    R_PMA_HWA_ID_IMP_RADSP_DMAC1,   /*!< IMP RADSP DMAC1 */
    R_PMA_HWA_ID_VIP_UMFL0,         /*!< UMFL0  (DOF) */
    R_PMA_HWA_ID_VIP_UMFL1,         /*!< UMFL1  (DOF) */
    R_PMA_HWA_ID_VIP_DISP0,         /*!< DISP0 (STV) */
    R_PMA_HWA_ID_VIP_DISP1,         /*!< DISP1 (STV) */
    R_PMA_HWA_ID_VIP_ACF0,          /*!< ACF0(CLE0) */
    R_PMA_HWA_ID_VIP_ACF1,          /*!< ACF1(CLE1) */
    R_PMA_HWA_ID_VIP_ACF2,          /*!< ACF2(CLE2) */
    R_PMA_HWA_ID_VIP_ACF3,          /*!< ACF3(CLE3) */
    R_PMA_HWA_ID_VIP_ACF4,          /*!< ACF3(CLE4) */
    R_PMA_HWA_ID_ICCOM,             /*!< ICCOM */
    R_PMA_HWA_ID_VCP_VCP4,          /*!< VCP4 */
    R_PMA_HWA_ID_VCP_FCPC,          /*!< FCPC */
    R_PMA_HWA_ID_VCP_IVCP1E,        /*!< iVCP1E */
    R_PMA_HWA_ID_ISP_CORE0,         /*!< ISP Channel 0 */
    R_PMA_HWA_ID_ISP_CORE1,         /*!< ISP Channel 1 */
    R_PMA_HWA_ID_ISP_CORE2,         /*!< ISP Channel 2 */
    R_PMA_HWA_ID_ISP_CORE3,         /*!< ISP Channel 3 */
    R_PMA_HWA_ID_ISP_TISP0,         /*!< ISP TISP0 */
    R_PMA_HWA_ID_ISP_TISP1,         /*!< ISP TISP1 */
    R_PMA_HWA_ID_ISP_TISP2,         /*!< ISP TISP2 */
    R_PMA_HWA_ID_ISP_TISP3,         /*!< ISP TISP3 */
    R_PMA_HWA_ID_ISP_VSPX0,         /*!< ISP VSPX0 */
    R_PMA_HWA_ID_ISP_VSPX1,         /*!< ISP VSPX1 */
    R_PMA_HWA_ID_ISP_VSPX2,         /*!< ISP VSPX2 */
    R_PMA_HWA_ID_ISP_VSPX3,         /*!< ISP VSPX3 */
    R_PMA_HWA_ID_IMR_LX4_CH0,       /*!< IMR-LX4 ch0 */
    R_PMA_HWA_ID_IMR_LX4_CH1,       /*!< IMR-LX4 ch1 */
    R_PMA_HWA_ID_IMR_LX4_CH2,       /*!< IMR-LX4 ch2 */
    R_PMA_HWA_ID_IMR_LX4_CH3,       /*!< IMR-LX4 ch3 */
    R_PMA_HWA_ID_IMR_LX4_CH4,       /*!< IMR-LX4 ch4 */
    R_PMA_HWA_ID_IMR_LX4_CH5,       /*!< IMR-LX4 ch5 */
    R_PMA_HWA_ID_FBC,               /*!< FBC */
    R_PMA_HWA_ID_FBA_IMS0,          /*!< FBA IMS0 */
    R_PMA_HWA_ID_FBA_IMS1,          /*!< FBA IMS1 */
    R_PMA_HWA_ID_FBA_IMS2,          /*!< FBA IMS1 */
    R_PMA_HWA_ID_FBA_IMS3,          /*!< FBA IMS1 */
    R_PMA_HWA_ID_FBA_IMR0,          /*!< FBA IMR0 */
    R_PMA_HWA_ID_FBA_IMR1,          /*!< FBA IMR1 */
    R_PMA_HWA_ID_FBA_IMR2,          /*!< FBA IMR2 */
    R_PMA_HWA_ID_FBA_IMR3,          /*!< FBA IMR3 */
    R_PMA_HWA_ID_FBA_IMP0,          /*!< FBA IMP0 */
    R_PMA_HWA_ID_FBA_IMP1,          /*!< FBA IMP1 */
    R_PMA_HWA_ID_FBA_IMP2,          /*!< FBA IMP2 */
    R_PMA_HWA_ID_FBA_IMP3,          /*!< FBA IMP3 */
    R_PMA_HWA_ID_FBA_IMP4,          /*!< FBA IMP3 */
    R_PMA_HWA_ID_FBA_OCV0,          /*!< FBA IMP OCV0 */
    R_PMA_HWA_ID_FBA_OCV1,          /*!< FBA IMP OCV1 */
    R_PMA_HWA_ID_FBA_OCV2,          /*!< FBA IMP OCV2 */
    R_PMA_HWA_ID_FBA_OCV3,          /*!< FBA IMP OCV3 */
    R_PMA_HWA_ID_FBA_OCV4,          /*!< FBA IMP OCV4 */
    R_PMA_HWA_ID_FBA_OCV5,          /*!< FBA IMP OCV5 */
    R_PMA_HWA_ID_FBA_OCV6,          /*!< FBA IMP OCV6 */
    R_PMA_HWA_ID_FBA_OCV7,          /*!< FBA IMP OCV7 */
    R_PMA_HWA_ID_FBA_DP0,           /*!< FBA IMP DP0 */
    R_PMA_HWA_ID_FBA_DP1,           /*!< FBA IMP DP1 */
    R_PMA_HWA_ID_FBA_CNN0,          /*!< FBA IMP CNN0 MAIN */
    R_PMA_HWA_ID_FBA_CNN0_SUB0,     /*!< FBA IMP CNN0 SUB0 */
    R_PMA_HWA_ID_FBA_CNN0_SUB1,     /*!< FBA IMP CNN0 SUB1 */
    R_PMA_HWA_ID_FBA_CNN0_SUB2,     /*!< FBA IMP CNN0 SUB2 */
    R_PMA_HWA_ID_FBA_CNN0_SUB3,     /*!< FBA IMP CNN0 SUB3 */
    R_PMA_HWA_ID_FBA_CNN1,          /*!< FBA IMP CNN1 MAIN */
    R_PMA_HWA_ID_FBA_CNN1_SUB0,     /*!< FBA IMP CNN1 SUB0 */
    R_PMA_HWA_ID_FBA_CNN1_SUB1,     /*!< FBA IMP CNN1 SUB1 */
    R_PMA_HWA_ID_FBA_CNN1_SUB2,     /*!< FBA IMP CNN1 SUB2 */
    R_PMA_HWA_ID_FBA_CNN1_SUB3,     /*!< FBA IMP CNN1 SUB3 */
    R_PMA_HWA_ID_FBA_CNN2,          /*!< FBA IMP CNN2 MAIN */
    R_PMA_HWA_ID_FBA_CNN2_SUB0,     /*!< FBA IMP CNN2 SUB0 */
    R_PMA_HWA_ID_FBA_CNN2_SUB1,     /*!< FBA IMP CNN2 SUB1 */
    R_PMA_HWA_ID_FBA_CNN2_SUB2,     /*!< FBA IMP CNN2 SUB2 */
    R_PMA_HWA_ID_FBA_CNN2_SUB3,     /*!< FBA IMP CNN2 SUB3 */
    R_PMA_HWA_ID_FBA_DSP0,          /*!< FBA IMP DSP0 */
    R_PMA_HWA_ID_FBA_DSP1,          /*!< FBA IMP DSP1 */
    R_PMA_HWA_ID_FBA_CNRAM0,        /*!< FBA IMP CNRAM0 */
    R_PMA_HWA_ID_FBA_CNRAM1,        /*!< FBA IMP CNRAM1 */
    R_PMA_HWA_ID_FBA_CNRAM2,        /*!< FBA IMP CNRAM2 */
    R_PMA_HWA_ID_FBA_SLIM0,         /*!< FBA SLIM0 */
    R_PMA_HWA_ID_FBA_DOF0,          /*!< FBA DOF0 */
    R_PMA_HWA_ID_FBA_DOF1,          /*!< FBA DOF1 */
    R_PMA_HWA_ID_FBA_STV0,          /*!< FBA STV0 */
    R_PMA_HWA_ID_FBA_STV1,          /*!< FBA STV1 */
    R_PMA_HWA_ID_FBA_ACF0,          /*!< FBA ACF0 */
    R_PMA_HWA_ID_FBA_ACF1,          /*!< FBA ACF1 */
    R_PMA_HWA_ID_FBA_SMPS0,         /*!< FBA SMPS0 */
    R_PMA_HWA_ID_FBA_SMPS1,         /*!< FBA SMPS1 */
    R_PMA_HWA_ID_FBA_SMES0,         /*!< FBA SMES0 */
    R_PMA_HWA_ID_FBA_SMES1,         /*!< FBA SMES1 */
    R_PMA_HWA_ID_FBA_SMPO0,         /*!< FBA SMPO0 */
    R_PMA_HWA_ID_FBA_SMPO1,         /*!< FBA SMPO1 */
    R_PMA_HWA_ID_FBA_ISP0,          /*!< FBA ISP0 */
    R_PMA_HWA_ID_FBA_ISP1,          /*!< FBA ISP1 */
    R_PMA_HWA_ID_FBA_ISP2,          /*!< FBA ISP2 */
    R_PMA_HWA_ID_FBA_ISP3,          /*!< FBA ISP3 */
    R_PMA_HWA_ID_FBA_CL0,           /*!< FBA CA76 CL0 */
    R_PMA_HWA_ID_FBA_CL0_CPU0,      /*!< FBA CA76 CL0 CPU0 */
    R_PMA_HWA_ID_FBA_CL0_CPU1,      /*!< FBA CA76 CL0 CPU1 */
    R_PMA_HWA_ID_FBA_CL1,           /*!< FBA CA76 CL1 */
    R_PMA_HWA_ID_FBA_CL1_CPU2,      /*!< FBA CA76 CL1 CPU2 */
    R_PMA_HWA_ID_FBA_CL1_CPU3,      /*!< FBA CA76 CL1 CPU3 */
    R_PMA_HWA_ID_FBA_CL2,           /*!< FBA CA76 CL2 */
    R_PMA_HWA_ID_FBA_CL2_CPU4,      /*!< FBA CA76 CL2 CPU4 */
    R_PMA_HWA_ID_FBA_CL2_CPU5,      /*!< FBA CA76 CL2 CPU5 */
    R_PMA_HWA_ID_FBA_CL3,           /*!< FBA CA76 CL3 */
    R_PMA_HWA_ID_FBA_CL3_CPU6,      /*!< FBA CA76 CL3 CPU6 */
    R_PMA_HWA_ID_FBA_CL3_CPU7,      /*!< FBA CA76 CL3 CPU7 */
    R_PMA_HWA_ID_RFSO0,             /*!< RFSO0 */
    R_PMA_HWA_ID_RFSO1,             /*!< RFSO1 */
    R_PMA_HWA_ID_RFSO2,             /*!< RFSO2 */
    R_PMA_HWA_ID_RFSO3,             /*!< RFSO3 */
    R_PMA_HWA_ID_RFSO4,             /*!< RFSO4 */
    R_PMA_HWA_ID_RFSO5,             /*!< RFSO5 */
    R_PMA_HWA_ID_RFSO6,             /*!< RFSO6 */
    R_PMA_HWA_ID_RFSO7,             /*!< RFSO7 */
    R_PMA_HWA_ID_RFSO8,             /*!< RFSO8 */
    R_PMA_HWA_ID_RFSO9,             /*!< RFSO9 */
    R_PMA_HWA_ID_RFSO10,            /*!< RFSO10 */

    R_PMA_HWA_ID_NUM
} e_osal_pm_id_t;
/* PoC implementation */
#endif
/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Types_DevConfig_info Device Configurator Public type definitions (info)
 *
 * @{
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct st_osal_interrupt_info_t
 * structure to hold interrupt information
***********************************************************************************************************************/
typedef struct st_osal_interrupt_info
{
    uint64_t    irq_number;     /*!< IRQ number */

}st_osal_interrupt_info_t;

/*******************************************************************************************************************//**
 * @struct st_osal_device_info_t
 * structure to hold device information
***********************************************************************************************************************/
typedef struct st_osal_device_info
{
    const char                  *id;            /*!< Device of unique ID */
    const char                  *type;          /*!< Device type */

    uintptr_t                   address;        /*!< Start address of register */
    size_t                      range;          /*!< Register address range */

    st_osal_interrupt_info_t    *interrupt;     /*!< Array of Interrupt information of the device */
    size_t                      irq_channels;   /*!< Number of interrupts */

    e_pma_hwa_id_t              pm_id;         /*!< ID that uniquely represents the Power Manager of the device */
    e_osal_pm_policy_t          initial_policy; /*!< Initial policy of Power Manager of the device */

    const char                  *axi_bus_name;  /*!< AXI bus name associated with the device */

}st_osal_device_info_t;

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Defines_Extern_Variables_DevConfig Device Configurator Public extern variables (info)
 *
 * @{
 **********************************************************************************************************************/
/**
 * @brief Device info for the current SoC.
 *
 * Return structure that holds device information for the current SoC.
 *
 * @return == pointer to the valid structure
 * @return == NULL
 */
st_osal_device_info_t* osal_device_info(void);

/** @} */

/** @} */

#endif /* OSAL_DEV_CFG_INFO_H */
