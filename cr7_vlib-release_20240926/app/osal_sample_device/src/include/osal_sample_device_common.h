/*************************************************************************************************************
* OSAL Sample App
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    osal_sample_device_common.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  Header file of Sample application to use OSAL IO/Interrupt/Power API
***********************************************************************************************************************/

#ifndef OSAL_SAMPLE_DEVICE_COMMON_H_
#define OSAL_SAMPLE_DEVICE_COMMON_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rcar-xos/osal/r_osal.h"

#define OSAL_SAMPLE_INFO(...) {printf("%s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}
#define OSAL_SAMPLE_ERR(...)  {printf("error: %s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}

/* Device type */
#define ims "imr"

/* IRQ channel */
#define IRQ_CHANNEL_0 0

/* Timeout */
#define TIMEOUT 1000 /* 1000ms */

#endif /* End of File */
