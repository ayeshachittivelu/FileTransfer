/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_version.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper version function for FreeRTOS
***********************************************************************************************************************/

/**
 * @file r_osal_version.c
 */

#include "target/freertos/r_osal_common.h"

#define OSAL_WRAPPER_VERSION_MAJOR   (1)
#define OSAL_WRAPPER_VERSION_MINOR   (0)
#define OSAL_WRAPPER_VERSION_PATCH   (0)

/***********************************************************************************************************************
* Start of function R_OSAL_GetVersion()
***********************************************************************************************************************/
const st_osal_version_t* R_OSAL_GetVersion(void)
{
    static const st_osal_version_t s_version =
    {
        {
            OSAL_VERSION_MAJOR,
            OSAL_VERSION_MINOR,
            OSAL_VERSION_PATCH
        },
        {
            OSAL_WRAPPER_VERSION_MAJOR,
            OSAL_WRAPPER_VERSION_MINOR,
            OSAL_WRAPPER_VERSION_PATCH
        }
    };

    return &s_version;
}
/***********************************************************************************************************************
* End of function R_OSAL_GetVersion()
***********************************************************************************************************************/
