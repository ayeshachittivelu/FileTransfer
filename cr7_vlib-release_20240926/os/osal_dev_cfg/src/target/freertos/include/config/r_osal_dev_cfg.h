/*************************************************************************************************************
* OSAL Device Configurator
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_dev_cfg.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL device configuration file
***********************************************************************************************************************/
#ifndef OSAL_DEV_CFG_H
#define OSAL_DEV_CFG_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_info.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_control.h"
#include "target/freertos/pma/r_pma.h"

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Private_Defines_DevConfig Device Condifurator Private macro definitions
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def OSAL_DCFG_INNER_TIMEOUT
 * osal internal timeout[msec] value.
***********************************************************************************************************************/
#define OSAL_DCFG_INNER_TIMEOUT     (500L)

/*******************************************************************************************************************//**
 * @def OSAL_PMA_PRIORITY
 * osal pma internal thread priority.
***********************************************************************************************************************/
#define OSAL_PMA_PRIORITY (OSAL_THREAD_PRIORITY_HIGHEST)

/*******************************************************************************************************************//**
 * @def OSAL_PMA_TIMEOUT
 * osal pma timeout[msec] value.
***********************************************************************************************************************/
#define OSAL_PMA_TIMEOUT (1000U)

/*******************************************************************************************************************//**
 * @def OSAL_PMA_POLLING_MAX_NUM
 * osal pma maximum nunber of busy-loop.
***********************************************************************************************************************/
#define OSAL_PMA_POLLING_MAX_NUM (10U)

/*******************************************************************************************************************//**
 * @def OSAL_PMA_LOOP_NUM_FOR_RCLK_CYCLE
 * osal pma number of a busy loop to wait for the completion of SoftwareReset.
***********************************************************************************************************************/
#define OSAL_PMA_LOOP_NUM_FOR_RCLK_CYCLE (40U)

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Private_Defines_Static_Variables_DevConfig Device Condifurator Private static variables
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @var gs_osal_device_control
 * osal device configuration : device control structure
***********************************************************************************************************************/
static st_osal_device_control_t gs_osal_device_control[OSAL_DCFG_DEVICE_MAX_NUM] =
{
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0},
    {.handle_id = 0}
};

/*******************************************************************************************************************//**
 * @var gs_osal_axi_bus_control
 * osal device configuration : axi bus control structure
***********************************************************************************************************************/
static st_osal_axi_bus_control_t gs_osal_axi_bus_control =
{
    0, NULL
};

/*******************************************************************************************************************//**
 * @var gs_osal_memory_max_configration
 * osal device configuration : max memory configuration structure
***********************************************************************************************************************/
static st_osal_mmngr_config_t gs_osal_memory_max_configration =
{
    .mode = OSAL_MMNGR_ALLOC_MODE_STACK_PROT,
    .memory_limit = 0, // To be set in R_OSAL_DCFG_GetMaxConfigOfMemory.
    .max_allowed_allocations = (4 * 1024),
    .max_registered_monitors_cbs = (4 * 1024)
};

/** @} */

#endif /* OSAL_DEV_CFG_H */
