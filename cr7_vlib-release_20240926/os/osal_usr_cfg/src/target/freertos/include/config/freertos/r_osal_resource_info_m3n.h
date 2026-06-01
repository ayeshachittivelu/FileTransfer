/*************************************************************************************************************
* OSAL Resource Configurator
* Copyright (c) [2023] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_resource_info_m3n.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL resource configuration file for M3N
***********************************************************************************************************************/
#ifndef OSAL_USR_INFO_M3N_H
#define OSAL_USR_INFO_M3N_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal_usr_cfg/freertos/r_osal_usr_cfg_info.h"

/*******************************************************************************************************************//**
 * @var gs_osal_interrupt_thread_config_m3n
 * osal user configuration : interrupt thread configuration table
***********************************************************************************************************************/
st_osal_interrupt_config_t  gs_osal_interrupt_thread_config_m3n[] =
{
    /* irq, device_id,              interrupt priority */
    {192, "imr_00",                 OSAL_INTERRUPT_PRIORITY_TYPE0},
    {193, "imr_01",                 OSAL_INTERRUPT_PRIORITY_TYPE0},
    {194, "imr_02",                 OSAL_INTERRUPT_PRIORITY_TYPE0},
    {195, "imr_03",                 OSAL_INTERRUPT_PRIORITY_TYPE0},

    {30,  "canfd",                  OSAL_INTERRUPT_PRIORITY_TYPE9},
    {29,  "canfd",                  OSAL_INTERRUPT_PRIORITY_TYPE10},

    {188, "vin_00",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {189, "vin_01",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {190, "vin_02",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {191, "vin_03",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {174, "vin_04",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {175, "vin_05",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {176, "vin_06",                 OSAL_INTERRUPT_PRIORITY_TYPE12},
    {171, "vin_07",                 OSAL_INTERRUPT_PRIORITY_TYPE12},

    {466, "vspd_00",                OSAL_INTERRUPT_PRIORITY_TYPE13},
    {467, "vspd_01",                OSAL_INTERRUPT_PRIORITY_TYPE13},
    {468, "vspd_02",                OSAL_INTERRUPT_PRIORITY_TYPE13},
    {162, "vspd_00",                OSAL_INTERRUPT_PRIORITY_TYPE1},
    {169, "vspd_01",                OSAL_INTERRUPT_PRIORITY_TYPE1},
    {170, "vspd_02",                OSAL_INTERRUPT_PRIORITY_TYPE1},

    {99,  "dhd_00",                 OSAL_INTERRUPT_PRIORITY_TYPE11},
    {98,  "dhd_00",                 OSAL_INTERRUPT_PRIORITY_TYPE11},
    {97,  "dhd_00",                 OSAL_INTERRUPT_PRIORITY_TYPE11},

    {380, "ivdp1c_00",              OSAL_INTERRUPT_PRIORITY_TYPE0},
    {381, "ivdp1c_00",              OSAL_INTERRUPT_PRIORITY_TYPE0},
    {223, "ivdp1c_00",              OSAL_INTERRUPT_PRIORITY_TYPE0},

    {432, "doc_00",                 OSAL_INTERRUPT_PRIORITY_TYPE1},
    {433, "doc_00",                 OSAL_INTERRUPT_PRIORITY_TYPE1},
    {434, "doc_01",                 OSAL_INTERRUPT_PRIORITY_TYPE1},
    {435, "doc_01",                 OSAL_INTERRUPT_PRIORITY_TYPE1},

    {449, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {450, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {451, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {452, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {453, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {454, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {455, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {456, "rtdmac_00",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {457, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {458, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {459, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {460, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {461, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {462, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {463, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},
    {464, "rtdmac_01",              OSAL_INTERRUPT_PRIORITY_TYPE1},

    {39,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {40,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {41,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {42,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {43,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {44,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {45,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {46,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {47,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {48,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {49,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {50,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {51,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {52,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {53,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {54,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {55,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {56,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {57,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {58,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {59,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {60,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {61,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {62,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {63,  "ethernet",               OSAL_INTERRUPT_PRIORITY_TYPE12},
    {224, "mfis_00",                OSAL_INTERRUPT_PRIORITY_TYPE12},
    {444, "vspi_00",                OSAL_INTERRUPT_PRIORITY_TYPE12},

    {.id = OSAL_INTERRUPT_INVALID_ID}
};

#endif /* OSAL_USR_INFO_M3N_H */
