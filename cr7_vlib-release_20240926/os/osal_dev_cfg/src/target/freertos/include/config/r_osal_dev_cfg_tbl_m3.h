/*************************************************************************************************************
* OSAL Device Configurator
* Copyright (c) [2023] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_dev_cfg_m3.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car M3
* Description :  OSAL device configuration file for R-Car M3 (M3-W)
***********************************************************************************************************************/
#ifndef OSAL_DEV_CFG_M3_H
#define OSAL_DEV_CFG_M3_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_info.h"
#include "config/r_osal_dev_cfg_tbl.h"

/*******************************************************************************************************************//**
 * @var gs_int_vspd_m3
 * osal device configuration : VSPD interrupt
***********************************************************************************************************************/
static st_osal_interrupt_info_t gs_int_vspd_m3[3][2] =
{
    {
        {466}, {162}
    },
    {
        {467}, {163}
    },
    {
        {468}, {169}
    }
};

/*******************************************************************************************************************//**
 * @var gs_osal_device_info_m3
 * osal device configuration : device information structure
***********************************************************************************************************************/
st_osal_device_info_t gs_osal_device_info_m3[] =
{
    /* device unique ID,      device type,    physical address,    range,     interrupt,    number of interrupts, power manager id,   initial policy,      AXI bus name,    assignment */
    /* DECODER */
    {"ivdp1c_00",             "ivdp1c",            0xFE8D0000U, 0x00001000U, gs_int_decoder,          3U, R_PMA_HWA_ID_IVDP1C,        OSAL_PM_POLICY_CG,       "invalid"},
    {"fcpc_00",               "fcpc",              0xFE90F000U, 0x00001000U, NULL,                    0U, R_PMA_HWA_ID_FCPC,          OSAL_PM_POLICY_CG,       "vc0"    },

    /* IMR */
    {"imr_00",                "imr",               0xFE860000U, 0x00002000U, gs_int_ims[0],           1U, R_PMA_HWA_ID_IMR0,          OSAL_PM_POLICY_CG,       "vc0"},
    {"imr_01",                "imr",               0xFE870000U, 0x00002000U, gs_int_ims[1],           1U, R_PMA_HWA_ID_IMR1,          OSAL_PM_POLICY_CG,       "vc0"},
    {"imr_02",                "imr",               0xFE880000U, 0x00002000U, gs_int_ims[2],           1U, R_PMA_HWA_ID_IMR2,          OSAL_PM_POLICY_CG,       "vc0"},
    {"imr_03",                "imr",               0xFE890000U, 0x00002000U, gs_int_ims[3],           1U, R_PMA_HWA_ID_IMR3,          OSAL_PM_POLICY_CG,       "vc0"},

    /* DHD */
    {"dhd_00",                "dhd",               0xE6720000U, 0x00001000U, gs_int_dhd,              3U, R_PMA_HWA_ID_DHD,           OSAL_PM_POLICY_CG,       "invalid"},

    /* VSPD */
    {"vspd_00",               "vspd",              0xFEA20000U, 0x00008000U, gs_int_vspd_m3[0],       2U, R_PMA_HWA_ID_VSPD0,         OSAL_PM_POLICY_CG,       "invalid"},
    {"vspd_01",               "vspd",              0xFEA28000U, 0x00008000U, gs_int_vspd_m3[1],       2U, R_PMA_HWA_ID_VSPD1,         OSAL_PM_POLICY_CG,       "invalid"},
    {"vspd_02",               "vspd",              0xFEA30000U, 0x00008000U, gs_int_vspd_m3[2],       2U, R_PMA_HWA_ID_VSPD2,         OSAL_PM_POLICY_CG,       "invalid"},

    /* VIN */
    {"vin_00",                "vin",               0xE6EF0000U, 0x00001000U, gs_int_vin[0],           1U, R_PMA_HWA_ID_VIN0,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_01",                "vin",               0xE6EF1000U, 0x00001000U, gs_int_vin[1],           1U, R_PMA_HWA_ID_VIN1,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_02",                "vin",               0xE6EF2000U, 0x00001000U, gs_int_vin[2],           1U, R_PMA_HWA_ID_VIN2,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_03",                "vin",               0xE6EF3000U, 0x00001000U, gs_int_vin[3],           1U, R_PMA_HWA_ID_VIN3,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_04",                "vin",               0xE6EF4000U, 0x00001000U, gs_int_vin[4],           1U, R_PMA_HWA_ID_VIN4,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_05",                "vin",               0xE6EF5000U, 0x00001000U, gs_int_vin[5],           1U, R_PMA_HWA_ID_VIN5,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_06",                "vin",               0xE6EF6000U, 0x00001000U, gs_int_vin[6],           1U, R_PMA_HWA_ID_VIN6,          OSAL_PM_POLICY_CG,       "invalid"},
    {"vin_07",                "vin",               0xE6EF7000U, 0x00001000U, gs_int_vin[7],           1U, R_PMA_HWA_ID_VIN7,          OSAL_PM_POLICY_CG,       "invalid"},

    /* MFIS */
    {"mfis_00",               "mfis",              0xE6260000U, 0x00001000U, gs_int_mfis[0],          1U, R_PMA_HWA_ID_INVALID,       OSAL_PM_POLICY_CG,       "invalid"},

    /* DOC */
    {"doc_00",                "doc",               0xFEBA0000U, 0x00018000U, gs_int_doc[0],           2U, R_PMA_HWA_ID_DOC0,          OSAL_PM_POLICY_CG,       "invalid"},
    {"doc_01",                "doc",               0xFEBB8000U, 0x00018000U, gs_int_doc[1],           2U, R_PMA_HWA_ID_DOC1,          OSAL_PM_POLICY_CG,       "invalid"},

    /* CAN-FD */
    {"canfd",                 "canfd",             0xE66C0000U, 0x00008000U, gs_int_canfd[0],         2U, R_PMA_HWA_ID_CANFD,         OSAL_PM_POLICY_CG,       "invalid"},

    /* ETHERNET */
    {"ethernet",              "ethernet",          0xE6800000U, 0x00000800U, gs_int_ethernet[0],     25U, R_PMA_HWA_ID_ETHERNET,      OSAL_PM_POLICY_CG,       "invalid"},

    /* DMAC */
    {"rtdmac_00",             "rtdmac",            0xFFC10000U, 0x00008400U, gs_int_dmac[0],          8U, R_PMA_HWA_ID_RTDMAC0,       OSAL_PM_POLICY_CG,       "invalid"},
    {"rtdmac_01",             "rtdmac",            0xFFC20000U, 0x00008400U, gs_int_dmac[1],          8U, R_PMA_HWA_ID_RTDMAC1,       OSAL_PM_POLICY_CG,       "invalid"},

    /* Audio-DMAC */
    {"audmac_00",             "audmac",            0xEC700000U, 0x00008400U, gs_int_audmac[0],       16U, R_PMA_HWA_ID_AUDMAC0,       OSAL_PM_POLICY_CG,       "invalid"},
    {"audmac_01",             "audmac",            0xEC720000U, 0x00008400U, gs_int_audmac[1],       16U, R_PMA_HWA_ID_AUDMAC1,       OSAL_PM_POLICY_CG,       "invalid"},

    /* VSPI */
    {"vspi_00",               "vspi",              0xFE9A0000U, 0x00008000U, gs_int_vspi[0],          1U, R_PMA_HWA_ID_INVALID,       OSAL_PM_POLICY_CG,       "invalid"},

    {.id = OSAL_DEVICE_INVALID_ID}
};

/** @} */

#endif /* OSAL_DEV_CFG_M3_H */
