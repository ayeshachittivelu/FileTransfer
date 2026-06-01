/*************************************************************************************************************
* OSAL Resource Configurator
* Copyright (c) [2023] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_resource_info.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL resource configuration interface
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "config/freertos/r_osal_resource_info_e3.h"
#include "config/freertos/r_osal_resource_info_h3.h"
#include "config/freertos/r_osal_resource_info_m3.h"
#include "config/freertos/r_osal_resource_info_m3n.h"
#include "r_prr_api.h"

/***********************************************************************************************************************
* Start of function osal_interrupt_thread_config()
***********************************************************************************************************************/
st_osal_interrupt_config_t* osal_interrupt_thread_config(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    static r_prr_DevName_t soc = R_PRR_RCARLAST;

    if (soc == R_PRR_RCARLAST)
    {
        soc = R_PRR_GetDevice();
    }

    switch (soc) {
        case R_PRR_RCARE3:
            return gs_osal_interrupt_thread_config_e3;
        case R_PRR_RCARH3:
            return gs_osal_interrupt_thread_config_h3;
        case R_PRR_RCARM3W:
            return gs_osal_interrupt_thread_config_m3;
        case R_PRR_RCARM3N:
            return gs_osal_interrupt_thread_config_m3n;
    }

    return NULL;
}
/***********************************************************************************************************************
* End of function osal_interrupt_thread_config()
***********************************************************************************************************************/
