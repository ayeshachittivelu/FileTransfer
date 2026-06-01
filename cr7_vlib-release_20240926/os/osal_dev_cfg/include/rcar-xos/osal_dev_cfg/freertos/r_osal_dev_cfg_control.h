/*************************************************************************************************************
* OSAL Device Configurator
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_dev_cfg_control.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL device configuration for FREERTOS header file
***********************************************************************************************************************/

#ifndef OSAL_DEV_CFG_CONTROL_H
#define OSAL_DEV_CFG_CONTROL_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal/r_osal_memory_impl.h"
#include "rcar-xos/memory_allocator/r_meml.h"
#include "pma/r_pma.h"

//#include <pthread.h>


/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Defines_DevConfig_control Device Configurator Public macro definitions (control)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def OSAL_DCFG_DEVICE_MAX_NUM
 * Max value of the device control table.
***********************************************************************************************************************/
#define OSAL_DCFG_DEVICE_MAX_NUM        (256)

/*******************************************************************************************************************//**
 * @def OSAL_PMA_ID_NONE
 * Invalid pma id.
***********************************************************************************************************************/
#define OSAL_PMA_ID_NONE                (0xFFFFU)

/*******************************************************************************************************************//**
 * @def OSAL_AXI_BUS_NAME_MAX_LENGTH
 * Length of the AXI bus name.
***********************************************************************************************************************/
#define OSAL_AXI_BUS_NAME_MAX_LENGTH    (31UL)

/*******************************************************************************************************************//**
 * @def OSAL_MEMORY_INNER_TIMEOUT
 * Value for timeout used in memory manager internal exclusive control.
***********************************************************************************************************************/
#define OSAL_MEMORY_INNER_TIMEOUT       (500L)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_MEM_4K
 * Minimum size of Continuous Memory.
***********************************************************************************************************************/
#define OSAL_MMNGR_MEM_4K               (4 * 1024)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_GUARD_PAGE_SIZE
 * Guard area size.
***********************************************************************************************************************/
#define OSAL_MMNGR_GUARD_PAGE_SIZE      (4096U)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_GUARD_PAGE_PATTERN
 * Pattern value to be set in the guard area.
***********************************************************************************************************************/
#define OSAL_MMNGR_GUARD_PAGE_PATTERN   (0xAAU)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_BUFFER_UNDERRUN
 * Error bit value when underflow occurs.
***********************************************************************************************************************/
#define OSAL_MMNGR_BUFFER_UNDERRUN      (1U << 0)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_BUFFER_OVERRUN
 * Error bit value when overflow occurs.
***********************************************************************************************************************/
#define OSAL_MMNGR_BUFFER_OVERRUN       (1U << 1)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_MEM_RESERVE_SIZE
 * Reserve area size of Continuous Memory.
 * (((Guard Page Size * 2) + Alignment-adjusted Library Management Size) * Max Allowed Allocations)
***********************************************************************************************************************/
#define OSAL_MMNGR_MEM_RESERVE_SIZE     (((4096U * 2U) + 4096U) * 4096U)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_GB_UNIT
 * Gigabyte unit.
***********************************************************************************************************************/
#define OSAL_MMNGR_GB_UNIT              (1024 * 1024 * 1024)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_MB_UNIT
 * Megabyte unit.
***********************************************************************************************************************/
#define OSAL_MMNGR_MB_UNIT              (1024 * 1024)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_KB_UNIT
 * kilobyte unit.
***********************************************************************************************************************/
#define OSAL_MMNGR_KB_UNIT              (1024)

/*******************************************************************************************************************//**
 * @def OSAL_DEVICE_HANDLE_ID
 * Handle id for device.
***********************************************************************************************************************/
#define OSAL_DEVICE_HANDLE_ID    (0x4F53414C5F444556ULL) /* ascii code: OSAL_DEV */

/*******************************************************************************************************************//**
 * @def OSAL_MEMORY_HANDLE_ID
 * Handle id for memory manager object.
***********************************************************************************************************************/
#define OSAL_INVALID_HANDLE_ID   (0)
#define OSAL_MEMORY_HANDLE_ID    (0x4F53414C4D4E4752ULL) /* ascii code: OSALMNGR */

/*******************************************************************************************************************//**
 * @def OSAL_BUFFER_HANDLE_ID
 * Handle id for memory buffer object.
***********************************************************************************************************************/
#define OSAL_BUFFER_HANDLE_ID    (0x4F53414C42554646ULL) /* ascii code: OSALBUFF */

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_STATE_NONE
 * Memory Manager status none.
***********************************************************************************************************************/
#define OSAL_MMNGR_STATE_NONE              (0)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_STATE_INIT
 * Memory Manager status initialized.
***********************************************************************************************************************/
#define OSAL_MMNGR_STATE_INIT              (1)

/*******************************************************************************************************************//**
 * @def OSAL_MMNGR_STATE_OPEN
 * Memory Manager status opened.
***********************************************************************************************************************/
#define OSAL_MMNGR_STATE_OPEN              (2)

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Types_DevConfig_control Device Configurator Public type definitions (control)
 *
 * @{
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct st_osal_device_control_t
 * structure to control device.
***********************************************************************************************************************/
typedef struct st_osal_device_control
{
    uint64_t                         handle_id;             /*!< Fixed value: OSAL_DEVICE_HANDLE_ID */

    bool                             use;                   /*!< handle state(true: handle is active, false: handle is inactive) */
    bool                             active;                /*!< The device handle active flag (true: opened device handle, false: closed device handle) */

    SemaphoreHandle_t                inner_mtx;             /*!< Internal mutex object */
    osal_milli_sec_t                 inner_timeout;         /*!< Waiting time of lock inner_mtx */

    const struct st_osal_device_info *dev_info;             /*!< Device configuration */
    st_pma_handle_t                  pma_handle;            /*!< Handle of Power Manager Agent */
    bool                             pm_wait_state;
    e_osal_pm_state_t                lowest_power_state;    /*!< Power manager status when changing to non- required status */

    osal_axi_bus_id_t                axi_bus_id;            /*!< AXI bus ID corresponding to the device */

}st_osal_device_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_axi_bus_list_t
 * structure to hold AXI bus information
***********************************************************************************************************************/
typedef struct st_osal_axi_bus_list
{
    char    axi_bus_name[OSAL_AXI_BUS_NAME_MAX_LENGTH]; /*!< AXI Bus name */

}st_osal_axi_bus_list_t;

/*******************************************************************************************************************//**
 * @struct st_osal_axi_bus_control_t
 * structure to control axi bus.
***********************************************************************************************************************/
typedef struct st_osal_axi_bus_control
{
    uint64_t                    num;                /*!< number of registrations with "axi_bus_list" */
    st_osal_axi_bus_list_t      *axi_bus_list;      /*!< AXI Bus information */

}st_osal_axi_bus_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_mem_buf_ptr_self_t
 * structure to buffer information area
***********************************************************************************************************************/
typedef struct st_osal_mem_buf_ptr_self
{
    struct st_osal_memory_buffer_obj mbuff_obj;     /*!< this area is exposed to interface as memory buffer object*/
    uint64_t                         handle_id;     /*!< Fixed value: OSAL_BUFFER_HANDLE_ID */
    size_t                           buffer_id;     /*!< ID of buffer */

    SemaphoreHandle_t                  inner_mtx;     /*!< Internal mutex object */
    osal_milli_sec_t                 inner_timeout; /*!< Waiting time of lock inner_mtx */

    size_t                           size;          /*!< User-specified size when allocating */
    size_t                           lib_mngt_size; /*!< Library management area size */

    uintptr_t                        buffer_pa;     /*!< Physical address of the buffer used by the user */
    uintptr_t                        chunk_pa;      /*!< Physical address of the allocated chunk */
    size_t                           chunk_size;    /*!< Size of chunk */

    uint8_t                          *p_chunk_va;   /*!< Virtual address of the allocated chunk */
    uint8_t                          *p_buffer_va;  /*!< Virtual address of the buffer used by the user */

}st_osal_mem_buf_ptr_self_t;

/*******************************************************************************************************************//**
 * @struct st_osal_mem_monitor_desc_t
 * structure to monitor information area
***********************************************************************************************************************/
typedef struct st_osal_mem_monitor_desc
{
    e_osal_mmngr_event_t            event_id;       /*!< Event ID */
    fp_osal_memory_mngr_cb_t        *p_handler;     /*!< Function pointer of memory manager callback */
    void                            *p_arg;         /*!< Argument of the callback */

}st_osal_mem_monitor_desc_t;

/*******************************************************************************************************************//**
 * @struct st_osal_mem_mmngr_ptr_self_t
 * structure to memory manager control table
***********************************************************************************************************************/
typedef struct st_osal_mem_mmngr_ptr_self
{
    struct st_osal_memory_manager_obj mmngr_obj;        /*!< this area is exposed to interface as memory manager object */
    uint64_t                          handle_id;        /*!< Fixed value: OSAL_MEMORY_HANDLE_ID */

    SemaphoreHandle_t                   inner_mtx;        /*!< Internal mutex object */
    osal_milli_sec_t                  inner_timeout;    /*!< Waiting time of lock inner_mtx */

    st_meml_manage_t                  meml_handle;      /*!< Handle of the Memory Allocator Library */
    st_osal_mmngr_config_t            config;           /*!< Memory Manager configuration */

    uintptr_t                         physical_addr;    /*!< Physical Address of the continuous memory */
    void                              *p_virtual_addr;  /*!< Virtual Address of the continuous memory */
    size_t                            size;             /*!< Size of the continuous memory */

    st_osal_mem_buf_ptr_self_t        *p_buffers;       /*!< Pointer to the structure of memory buffer */
    size_t                            num_buffers;      /*!< The number of buffers */
    size_t                            use_buffer_total; /*!< Size of the user allocate total. */
    st_osal_mem_monitor_desc_t        *p_monitor;       /*!< Pointer to the structure of monitor */
    size_t                            num_monitor;      /*!< The number of monitor */

}st_osal_mem_mmngr_ptr_self_t;

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_DEV_CFG
 * @defgroup OSAL_Public_Function_DevConfig Device Condifurator Public function definitions (control)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
* @brief            Get number of device control information.
* @return           size_t  number of device
***********************************************************************************************************************/
size_t R_OSAL_DCFG_GetNumOfDevice(void);

/*******************************************************************************************************************//**
* @brief            Get device control information.
* @param[in]        num      To set the device number
* @param[out]       pp_info  To set the address of device control structure pointer
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_DCFG_GetDeviceInfo(uint32_t num, st_osal_device_control_t **pp_info);

/*******************************************************************************************************************//**
* @brief            Get PMA configuration.
* @param[out]       p_config    The pointor of power manager agent configuration
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_DCFG_GetPMAConfiguration(st_pma_config_t * const p_config);

/*******************************************************************************************************************//**
* @brief            Get axi bus information.
* @param[out]       pp_info     The pointor of axi bus control information
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_DCFG_GetAxiBusInfo(st_osal_axi_bus_control_t **pp_info);

/*******************************************************************************************************************//**
* @brief            Get max configuration of the memory.
* @param[out]       pp_config   The pointor of memory manager configuration
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_DCFG_GetMaxConfigOfMemory(st_osal_mmngr_config_t **pp_config);

/*******************************************************************************************************************//**
* @brief            Get Device SoC type.
* @return           uint64_t  SoC type identifier
***********************************************************************************************************************/
uint64_t R_OSAL_DCFG_GetSocType(void);

/** @} */

#endif /* OSAL_DEV_CFG_CONTROL_H */
