/*************************************************************************************************************
* OSAL Resource Configurator
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_usr_cfg_info.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL resource configuration header file
***********************************************************************************************************************/

#ifndef OSAL_USR_CFG_INFO_H
#define OSAL_USR_CFG_INFO_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"


/*******************************************************************************************************************//**
 * @defgroup OSAL_WRAPPER_FREERTOS_USR_CFG OSAL Wrapper User Configurator For FREERTOS
 * @ingroup osal_wrapper
 * OSAL Wrapper User Configurator for FREERTOS
 * @{
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_USR_CFG
 * @defgroup OSAL_Public_Defines_UsrConfig_info User Configurator Public macro definitions (info)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def OSAL_THREAD_INVALID_ID
 * Invalid thread ID.
***********************************************************************************************************************/
#define    OSAL_THREAD_INVALID_ID       (0xFFFF)

/*******************************************************************************************************************//**
 * @def OSAL_MUTEX_INVALID_ID
 * Invalid mutex ID.
***********************************************************************************************************************/
#define    OSAL_MUTEX_INVALID_ID        (0xFFFF)

/*******************************************************************************************************************//**
 * @def OSAL_COND_INVALID_ID
 * Invalid condition variable ID.
***********************************************************************************************************************/
#define    OSAL_COND_INVALID_ID         (0xFFFF)

/*******************************************************************************************************************//**
 * @def OSAL_MQ_INVALID_ID
 * Invalid message queue ID.
***********************************************************************************************************************/
#define    OSAL_MQ_INVALID_ID           (0xFFFF)

/*******************************************************************************************************************//**
 * @def OSAL_INTERRUPT_INVALID_ID
 * Invalid interrupt ID.
***********************************************************************************************************************/
#define    OSAL_INTERRUPT_INVALID_ID    (NULL)

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_USR_CFG
 * @defgroup OSAL_Public_Type_UsrConfig_info User Configurator Public type definitions (info)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct st_osal_rcfg_thread_config_t
 * structure to hold thread configuration
***********************************************************************************************************************/
typedef struct st_osal_rcfg_thread_config
{
    osal_thread_id_t        id;         /*!< thread ID */
    st_osal_thread_config_t config;     /*!< thread configuration */

}st_osal_rcfg_thread_config_t;

/*******************************************************************************************************************//**
 * @struct st_osal_interrupt_config_t
 * structure to hold interrupt configuration
***********************************************************************************************************************/
typedef struct st_osal_interrupt_config
{
    uint64_t                    irq_number;     /*!< IRQ number */
    char                        *id;            /*!< Device ID */
    e_osal_interrupt_priority_t int_priority;   /*!< Interrupt priority */
}st_osal_interrupt_config_t;

/*******************************************************************************************************************//**
 * @struct st_osal_rcfg_mutex_config_t
 * structure to hold mutex configuration
***********************************************************************************************************************/
typedef struct st_osal_rcfg_mutex_config
{
    osal_mutex_id_t     id;     /*!< mutex ID */

}st_osal_rcfg_mutex_config_t;

/*******************************************************************************************************************//**
 * @struct st_osal_rcfg_cond_config_t
 * structure to hold condition variable configuration
***********************************************************************************************************************/
typedef struct st_osal_rcfg_cond_config
{
    osal_cond_id_t      id;     /*!< cond ID */

}st_osal_rcfg_cond_config_t;

/*******************************************************************************************************************//**
 * @struct st_osal_mq_rcfg_config_t
 * structure to hold message queue configuration
***********************************************************************************************************************/
typedef struct st_osal_rcfg_mq_config
{
    osal_mq_id_t        id;     /*!< message queue ID */
    st_osal_mq_config_t config; /*!< message queue configuration */

}st_osal_mq_rcfg_config_t;

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_USR_CFG
 * @defgroup OSAL_Public_Defines_Extern_Variables_UsrConfig User Configurator Public extern variables (info)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @var gs_osal_thread_config
 * osal user configuration : thread configuration table
***********************************************************************************************************************/
extern st_osal_rcfg_thread_config_t gs_osal_thread_config[];

/*******************************************************************************************************************//**
 * @var gs_osal_mutex_config
 * osal user configuration : mutex configuration table
***********************************************************************************************************************/
extern st_osal_rcfg_mutex_config_t gs_osal_mutex_config[];

/*******************************************************************************************************************//**
 * @var gs_osal_cond_config
 * osal user configuration : cond configuration table
***********************************************************************************************************************/
extern st_osal_rcfg_cond_config_t gs_osal_cond_config[];

/*******************************************************************************************************************//**
 * @var gs_osal_mq_config
 * osal user configuration : message queue configuration table
***********************************************************************************************************************/
extern st_osal_mq_rcfg_config_t gs_osal_mq_config[];

/**
 * @brief Interrupt thread configuration for the current SoC.
 *
 * Return structure that holds interrupt thread configuration for the current SoC.
 *
 * @return == pointer to the valid structure
 * @return == NULL
 */
st_osal_interrupt_config_t* osal_interrupt_thread_config(void);

/** @} */

/** @} */

#endif /* OSAL_USR_CFG_INFO_H */
