/*************************************************************************************************************
* OSAL Sample App
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    osal_sample_memory_common.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  Header file of Sample application to use OSAL Memory API
***********************************************************************************************************************/

#ifndef OSAL_SAMPLE_DEVICE_COMMON_H_
#define OSAL_SAMPLE_DEVICE_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal/r_osal_memory_impl.h"

#define OSAL_RESOURCE_ID        0xf000U            /* check, if static env */
#define INITIAL_TASK_ID         OSAL_RESOURCE_ID + 6

#define OSAL_SAMPLE_INFO(...) {printf("%s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}
#define OSAL_SAMPLE_ERR(...)  {printf("error: %s(%d):", __func__, __LINE__); printf(__VA_ARGS__);}

/* Alloc Size */
#define ALLOC_SIZE 64 /* 64 byte */

/* Alignment */
#define ALLOC_ALIGN 64

#endif /* End of File */
