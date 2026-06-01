/*************************************************************************************************************
* OSAL Device Configurator
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_dev_cfg_tbl.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL device configuration file
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_info.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_control.h"

#include "config/r_osal_dev_cfg_tbl_e3.h"
#include "config/r_osal_dev_cfg_tbl_h3.h"
#include "config/r_osal_dev_cfg_tbl_m3.h"
#include "config/r_osal_dev_cfg_tbl_m3n.h"

/***********************************************************************************************************************
* Start of function osal_device_info()
***********************************************************************************************************************/
st_osal_device_info_t* osal_device_info(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    const uint64_t soc = R_OSAL_DCFG_GetSocType();

    switch (soc) {
        case OSAL_DEVICE_SOC_TYPE_E3:
            return gs_osal_device_info_e3;
        case OSAL_DEVICE_SOC_TYPE_H3:
            return gs_osal_device_info_h3;
        case OSAL_DEVICE_SOC_TYPE_M3:
            return gs_osal_device_info_m3;
        case OSAL_DEVICE_SOC_TYPE_M3N:
            return gs_osal_device_info_m3n;
    }

    return NULL;
}
/***********************************************************************************************************************
* End of function osal_device_info()
***********************************************************************************************************************/
