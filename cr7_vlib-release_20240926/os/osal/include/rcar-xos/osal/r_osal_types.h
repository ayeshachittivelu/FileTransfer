/*************************************************************************************************************
* OSAL Common Header
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_types.h
* Version :      2.11.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL public header file
***********************************************************************************************************************/
#ifndef OSAL_TYPES_H
#define OSAL_TYPES_H

/* ################################################################################### */
/* Proposal: Enforce C99 ANSI C standard compatibility in header */

#if !defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* Compiler is compatible to C99 standard */
#pragma error "Compiler must be compatible with at least C99 standard"
#endif /* defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L */

/* ################################################################################### */

/***********************************************************************************************************************
 * includes
***********************************************************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h> /* PRQA S 5127 */
#include <limits.h>

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************
 * When defining OSAL types, "typedef" is used.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup OSAL_Defines
 * @defgroup OSAL_GENERIC_ENUM  OSAL Generic Enumeration
 * Enumeration for OSAL API
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @typedef e_osal_return_t
 * @brief   This type is the typedef definition of enum #e_osal_return.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_return
 * @brief   osal return value.
 **********************************************************************************************************************/
typedef enum e_osal_return
{
    OSAL_RETURN_OK                    = 0,      /*!< correct function execution */
    OSAL_RETURN_PAR                   = 1,      /*!< invalid input parameter */
    OSAL_RETURN_MEM                   = 2,      /*!< failure resource allocation */
    OSAL_RETURN_DEV                   = 3,      /*!< failure on the device side (HW failure) */
    OSAL_RETURN_TIME                  = 4,      /*!< timeout situation */
    OSAL_RETURN_CONF                  = 5,      /*!< inconsistent configuration */
    OSAL_RETURN_FAIL                  = 6,      /*!< internal non-categorized failure */
    OSAL_RETURN_IO                    = 7,      /*!< failure related to input/output operation */
    OSAL_RETURN_BUSY                  = 1024,   /*!< resource or device busy */
    OSAL_RETURN_ID                    = 1025,   /*!< wrong input ID */
    OSAL_RETURN_HANDLE                = 1026,   /*!< wrong input handle */
    OSAL_RETURN_STATE                 = 1027,   /*!< unexecutable state */
    OSAL_RETURN_PROHIBITED            = 1028,   /*!< operation is prohibited */
    OSAL_RETURN_UNSUPPORTED_OPERATION = 1029,   /*!< operation is unsupported */
    OSAL_RETURN_HANDLE_NOT_SUPPORTED  = 1030,   /*!< specified handle is unsupported */
    OSAL_RETURN_UNKNOWN_NAME          = 1031,   /*!< specified name is unknown */
    OSAL_RETURN_CONF_UNSUPPORTED      = 1032,   /*!< unsupported configuration value*/
    OSAL_RETURN_OK_RQST               = 1033,   /*!< request accepted */
} e_osal_return_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_clock_type_t
 * @brief   This type is the typedef definition of enum #e_osal_clock_type.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_clock_type
 * @brief   osal clock type.
 **********************************************************************************************************************/
typedef enum e_osal_clock_type
{
    OSAL_CLOCK_TYPE_HIGH_RESOLUTION = 1,    /*!< Clock with the highest resolution, 
                                             * suitable for measuring short time periods. */
    OSAL_CLOCK_TYPE_HIGH_PRECISION  = 2     /*!< Clock with the highest precision, suitable for measuring 
                                             * long time periods. */
} e_osal_clock_type_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_interrupt_priority_t
 * @brief   This type is the typedef definition of enum #e_osal_interrupt_priority.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_interrupt_priority
 * @brief   priority for interrupt.
 **********************************************************************************************************************/
typedef enum e_osal_interrupt_priority
{
    OSAL_INTERRUPT_PRIORITY_TYPE0   = 0,  /*!< interrupt priority 0 */
    OSAL_INTERRUPT_PRIORITY_LOWEST  = OSAL_INTERRUPT_PRIORITY_TYPE0,    /*!< Lowest Priority */
    OSAL_INTERRUPT_PRIORITY_TYPE1   = 1,    /*!< interrupt priority 1 */
    OSAL_INTERRUPT_PRIORITY_TYPE2   = 2,    /*!< interrupt priority 2 */
    OSAL_INTERRUPT_PRIORITY_TYPE3   = 3,    /*!< interrupt priority 3 */
    OSAL_INTERRUPT_PRIORITY_TYPE4   = 4,    /*!< interrupt priority 4 */
    OSAL_INTERRUPT_PRIORITY_TYPE5   = 5,    /*!< interrupt priority 5 */
    OSAL_INTERRUPT_PRIORITY_TYPE6   = 6,    /*!< interrupt priority 6 */
    OSAL_INTERRUPT_PRIORITY_TYPE7   = 7,    /*!< interrupt priority 7 */
    OSAL_INTERRUPT_PRIORITY_TYPE8   = 8,    /*!< interrupt priority 8 */
    OSAL_INTERRUPT_PRIORITY_TYPE9   = 9,    /*!< interrupt priority 9 */
    OSAL_INTERRUPT_PRIORITY_TYPE10  = 10,   /*!< interrupt priority 10 */
    OSAL_INTERRUPT_PRIORITY_TYPE11  = 11,   /*!< interrupt priority 11 */
    OSAL_INTERRUPT_PRIORITY_TYPE12  = 12,   /*!< interrupt priority 12 */
    OSAL_INTERRUPT_PRIORITY_TYPE13  = 13,   /*!< interrupt priority 13 */
    OSAL_INTERRUPT_PRIORITY_TYPE14  = 14,   /*!< interrupt priority 14 */
    OSAL_INTERRUPT_PRIORITY_HIGHEST = OSAL_INTERRUPT_PRIORITY_TYPE14,   /*!< Highest Priority */
} e_osal_interrupt_priority_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_thread_priority_t
 * @brief   This type is the typedef definition of enum #e_osal_thread_priority.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_thread_priority
 * @brief   priority for thread.
 **********************************************************************************************************************/
typedef enum e_osal_thread_priority
{
    OSAL_THREAD_PRIORITY_TYPE0   = 0,   /*!< thread priority 0 */
    OSAL_THREAD_PRIORITY_LOWEST  = OSAL_THREAD_PRIORITY_TYPE0,  /*!< Lowest Priority */
    OSAL_THREAD_PRIORITY_TYPE1   = 1,   /*!< thread priority 1 */
    OSAL_THREAD_PRIORITY_TYPE2   = 2,   /*!< thread priority 2 */
    OSAL_THREAD_PRIORITY_TYPE3   = 3,   /*!< thread priority 3 */
    OSAL_THREAD_PRIORITY_TYPE4   = 4,   /*!< thread priority 4 */
    OSAL_THREAD_PRIORITY_TYPE5   = 5,   /*!< thread priority 5 */
    OSAL_THREAD_PRIORITY_TYPE6   = 6,   /*!< thread priority 6 */
    OSAL_THREAD_PRIORITY_TYPE7   = 7,   /*!< thread priority 7 */
    OSAL_THREAD_PRIORITY_TYPE8   = 8,   /*!< thread priority 8 */
    OSAL_THREAD_PRIORITY_TYPE9   = 9,   /*!< thread priority 9 */
    OSAL_THREAD_PRIORITY_TYPE10  = 10,  /*!< thread priority 10 */
    OSAL_THREAD_PRIORITY_TYPE11  = 11,  /*!< thread priority 11 */
    OSAL_THREAD_PRIORITY_TYPE12  = 12,  /*!< thread priority 12 */
    OSAL_THREAD_PRIORITY_TYPE13  = 13,  /*!< thread priority 13 */
    OSAL_THREAD_PRIORITY_TYPE14  = 14,  /*!< thread priority 14 */
    OSAL_THREAD_PRIORITY_HIGHEST = OSAL_THREAD_PRIORITY_TYPE14, /*!< Highest Priority */
} e_osal_thread_priority_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_pm_required_state_t
 * @brief   This type is the typedef definition of enum #e_osal_pm_required_state.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_pm_required_state
 * @brief   osal pm require state.
 **********************************************************************************************************************/
typedef enum e_osal_pm_required_state
{
    OSAL_PM_REQUIRED_STATE_INVALID  = 0,    /*!< Invalid state */
    OSAL_PM_REQUIRED_STATE_REQUIRED = 1,    /*!< Device is required */
    OSAL_PM_REQUIRED_STATE_RELEASED = 2     /*!< Device is not required */
} e_osal_pm_required_state_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_pm_state_t
 * @brief   This type is the typedef definition of enum #e_osal_pm_state.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_pm_state
 * @brief   osal pm state.
 **********************************************************************************************************************/
typedef enum e_osal_pm_state
{
    OSAL_PM_STATE_INVALID = 0x00,   /*!< Invalid state */
    OSAL_PM_STATE_PG      = 0x01,   /*!< Power Gated state */
    OSAL_PM_STATE_CG      = 0x02,   /*!< Clock Gated state */
    OSAL_PM_STATE_ENABLED = 0x04,   /*!< Enabled state */
    OSAL_PM_STATE_RESET   = 0x08,   /*!< Reset state */
    OSAL_PM_STATE_READY   = 0x10    /*!< Ready state */
} e_osal_pm_state_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_pm_policy_t
 * @brief   This type is the typedef definition of enum #e_osal_pm_policy.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_pm_policy
 * @brief   osal pm policy.
 **********************************************************************************************************************/
typedef enum e_osal_pm_policy
{
    OSAL_PM_POLICY_INVALID  = 0,    /*!< Invalid state */
    OSAL_PM_POLICY_PG       = 1,    /*!< Power Gated Policy */
    OSAL_PM_POLICY_CG       = 2,    /*!< Clock Gated Policy */
    OSAL_PM_POLICY_HP       = 3     /*!< High Performance Policy */
} e_osal_pm_policy_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_pm_reset_state_t
 * @brief   This type is the typedef definition of enum #e_osal_pm_reset_state.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_pm_reset_state
 * @brief   osal pm reset state.
 **********************************************************************************************************************/

typedef enum e_osal_pm_reset_state
{
    OSAL_PM_RESET_STATE_INVALID     = 0,    /*!< Invalid state */
    OSAL_PM_RESET_STATE_APPLIED     = 1,    /*!< Device is in reset state */
    OSAL_PM_RESET_STATE_RELEASED    = 2     /*!< Device is not in reset status */
} e_osal_pm_reset_state_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_mmngr_event_t
 * @brief   This type is the typedef definition of enum #e_osal_mmngr_event.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_mmngr_event
 * @brief   Memory Manager Event enumeration values.
 *          Used to identify certain events that can be issued by an Memory Manager instance.
 * @see     R_OSAL_MmngrRegisterMonitor() for more details.
 **********************************************************************************************************************/
typedef enum e_osal_mmngr_event
{
    OSAL_MMNGR_EVENT_INVALID            = 0,    /*!< Invalid value */
    OSAL_MMNGR_EVENT_ALLOCATE           = 1,    /*!< Event issues after allocation completes */
    OSAL_MMNGR_EVENT_DEALLOCATE         = 2,    /*!< Event occurs before deletion of the buffer.
                                                 *   Handle of the buffer is still valid!*/
    OSAL_MMNGR_EVENT_OVERFLOW_DETECTED  = 3,    /*!< Event occurs if an buffer overflow is detected.
                                                 *   Handle of the buffer is still valid. */
    OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED = 4,    /*!< Event occurs if an buffer underflow is detected.
                                                 *   Handle of the buffer is still valid. */
    OSAL_MMNGR_EVENT_ERROR              = 5     /*!< Reserved for the future update. Some error event is detected. */
} e_osal_mmngr_event_t;

/*******************************************************************************************************************//**
 * @typedef e_osal_mmngr_allocator_mode_t
 * @brief   This type is the typedef definition of enum #e_osal_mmngr_allocator_mode.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @enum    e_osal_mmngr_allocator_mode
 * @brief   Common Memory Manager allocation mode values
 **********************************************************************************************************************/
typedef enum e_osal_mmngr_allocator_mode
{
    OSAL_MMNGR_ALLOC_MODE_INVALID           = 0,    /*!< Invalid/error value */
    OSAL_MMNGR_ALLOC_MODE_STACK             = 1,    /*!< Implementations may always fall to protection enabled mode 
                                                     *   #OSAL_MMNGR_ALLOC_MODE_STACK_PROT */
    OSAL_MMNGR_ALLOC_MODE_STACK_PROT        = 2,    /*!< Same as stack mode but with underflow and overflow detection */
    OSAL_MMNGR_ALLOC_MODE_FREE_LIST         = 3,    /*!< FreeList(BestFit) Allocation mode. Implementations may always 
                                                     *   fall to protection enabled mode 
                                                     *   #OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT */
    OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT    = 4,    /*!< Same as free list mode but with underflow and overflow 
                                                         detection */
    OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED       = 5,    /*!< If Specific Memory Managers implement other allocation modes 
                                                     *   than OSAL API, they can set this value when 
                                                         R_OSAL_MmngrGetConfig() is called. This value is not expected 
                                                     *   as input configuration for R_OSAL_MmngrOpen() */
    OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED_PROT  = 6     /*!< If Specific Memory Managers implement other allocation modes 
                                                     *   than OSAL API with underflow/overflow detection, they can set 
                                                     *   this value when R_OSAL_MmngrGetConfig() is called.
                                                     *   This value is not expected as input configuration for 
                                                     *   R_OSAL_MmngrOpen() */
} e_osal_mmngr_allocator_mode_t;

/*******************************************************************************************************************//**
 * @enum    r_osal_Cache_t
 * @brief   Cache selection for instruction, data, or both
 **********************************************************************************************************************/
typedef enum {
    R_OSAL_CACHE_I,   /**< Instruction cache */
    R_OSAL_CACHE_D,   /**< Data cache */
    R_OSAL_CACHE_ALL  /**< Instruction & data cache */
} r_osal_Cache_t;

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_Defines
 * @defgroup OSAL_GENERIC_TYPES OSAL Generic Type definitions
 * Type definitions for OSAL API
 * @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @typedef st_osal_time_t
 * @brief   This type is the typedef definition of struct st_osal_time.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct  st_osal_time
 * @brief   timespec structure compatible.
 **********************************************************************************************************************/
/* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct st_osal_time
{
    time_t  tv_sec;     /*!< whole seconds (valid values are >= 0) */
    int32_t tv_nsec;    /*!< nano seconds (valid values are [0, 999999999]) */
} st_osal_time_t;

/*******************************************************************************************************************//**
 * @typedef osal_milli_sec_t
 * @brief   time in milliseconds.
 **********************************************************************************************************************/
typedef int64_t osal_milli_sec_t; /*!< OSAL type for milli seconds */

/*******************************************************************************************************************//**
 * @typedef osal_nano_sec_t
 * @brief   time in nanoseconds.
 **********************************************************************************************************************/
typedef int64_t osal_nano_sec_t; /*!< OSAL type for nano seconds */

/*******************************************************************************************************************//**
 * @typedef osal_thread_id_t
 * @brief   Unique identifier of ID for thread.
 **********************************************************************************************************************/

typedef uint64_t osal_thread_id_t;

/*******************************************************************************************************************//**
 * @typedef p_osal_thread_func_t
 * @brief   thread worker function.
 **********************************************************************************************************************/
typedef int64_t (*p_osal_thread_func_t)(void* user_arg);

/*******************************************************************************************************************//**
 * @typedef st_osal_thread_config_t
 * @brief   This type is the typedef definition of struct st_osal_thread_config.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct  st_osal_thread_config
 * @brief   configuration of thread.
 **********************************************************************************************************************/
/* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct st_osal_thread_config
{
    p_osal_thread_func_t     func;          /*!< Worker function */
    void*                    userarg;       /*!< Pointer of Argument for worker function */
    e_osal_thread_priority_t priority;      /*!< Thread priority */
    const char*              task_name;     /*!< Pointer of const char task name */
    size_t                   stack_size;    /*!< Stack size */
} st_osal_thread_config_t;

/*******************************************************************************************************************//**
 * @struct st_osal_thread_control
 * @brief control of thread (Defined in OSAL Wrapper implementation files).
 **********************************************************************************************************************/
struct st_osal_thread_control; /* forward declaration */

/*******************************************************************************************************************//**
 * @typedef osal_thread_handle_t
 * @brief   handle of Thread. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_thread_control.
 **********************************************************************************************************************/
typedef struct st_osal_thread_control* osal_thread_handle_t; /*!< Handle of a created thread */

/*******************************************************************************************************************//**
 * @typedef osal_mutex_id_t
 * @brief   Unique identifier of ID for mutex.
 **********************************************************************************************************************/
typedef uint64_t osal_mutex_id_t;

/*******************************************************************************************************************//**
 * @struct  st_osal_mutex_control
 * @brief   control of mutex (Defined in OSAL Wrapper implementation files).
 **********************************************************************************************************************/
struct st_osal_mutex_control; /* Forward declaration. */

/*******************************************************************************************************************//**
 * @typedef osal_mutex_handle_t
 * @brief   Handle of a created mutex. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_mutex_control.
 **********************************************************************************************************************/
typedef struct st_osal_mutex_control* osal_mutex_handle_t;

/*******************************************************************************************************************//**
 * @typedef osal_cond_id_t
 * @brief   Unique identifier of a condition variable.
 **********************************************************************************************************************/
typedef uint64_t osal_cond_id_t;

/*******************************************************************************************************************//**
 * @struct  st_osal_cond_control
 * @brief   control of condition variable (Defined in OSAL Wrapper implementation files).
 **********************************************************************************************************************/
struct st_osal_cond_control; /* Forward declaration. */

/*******************************************************************************************************************//**
 * @typedef osal_cond_handle_t
 * @brief   Handle of a created condition variable. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_cond_control.
 **********************************************************************************************************************/
typedef struct st_osal_cond_control* osal_cond_handle_t;

/*******************************************************************************************************************//**
 * @typedef osal_mq_id_t
 * @brief   Unique identifier of a message queue.
 **********************************************************************************************************************/
typedef uint64_t osal_mq_id_t;

/*******************************************************************************************************************//**
 * @typedef st_osal_mq_config_t
 * @brief   This type is the typedef definition of struct st_osal_mq_config.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct  st_osal_mq_config
 * @brief   configuration of message queue.
 **********************************************************************************************************************/
 /* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct st_osal_mq_config
{
    uint64_t max_num_msg;   /*!< The maximum number of messages that can be stored on the queue. */
    uint64_t msg_size;      /*!< The fixed size in bytes of each message on the given message queue. */
} st_osal_mq_config_t;

/*******************************************************************************************************************//**
 * @struct  st_osal_mq_control
 * @brief   control of message queue (Defined in OSAL Wrapper implementation files).
 **********************************************************************************************************************/
struct st_osal_mq_control; /* Forward declaration. */

/*******************************************************************************************************************//**
 * @typedef osal_mq_handle_t
 * @brief   Handle of a created message queue. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_mq_control.
 **********************************************************************************************************************/
typedef struct st_osal_mq_control* osal_mq_handle_t;

/*******************************************************************************************************************//**
 * @struct  st_osal_device_control
 * @brief   control of device (Defined in OSAL Wrapper implementation files).
 **********************************************************************************************************************/
struct st_osal_device_control; /* Forward declaration. */

/*******************************************************************************************************************//**
 * @typedef osal_device_handle_t
 * @brief   Handle of opened device. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_device_control.
 **********************************************************************************************************************/
typedef struct st_osal_device_control* osal_device_handle_t;

/*******************************************************************************************************************//**
 * @typedef p_osal_isr_func_t
 * @brief   interrupt worker function.
 **********************************************************************************************************************/
typedef void (*p_osal_isr_func_t)(osal_device_handle_t device_handle, uint64_t irq_channel, void* irq_arg);

/*******************************************************************************************************************//**
 * @typedef osal_axi_bus_id_t
 * @brief   AXI Bus ID type that is assigned to a device. \n
 *          This ID reflects and index of the string list of AXI bus names  provided by 
 *          R_OSAL_IoGetAxiBusIdFromDeviceName()
 **********************************************************************************************************************/
typedef uint64_t osal_axi_bus_id_t;

struct st_osal_memory_buffer_obj;   /* forward declaration */
struct st_osal_memory_manager_obj;  /* forward declaration */

/*******************************************************************************************************************//**
 * @typedef osal_memory_manager_handle_t
 * @brief   Handle type of a Memory Manager. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_memory_manager_obj.
 **********************************************************************************************************************/
typedef struct st_osal_memory_manager_obj * osal_memory_manager_handle_t;

/*******************************************************************************************************************//**
 * @typedef osal_memory_buffer_handle_t
 * @brief   Handle type of a Buffer Object. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_memory_buffer_obj.
 **********************************************************************************************************************/
typedef struct st_osal_memory_buffer_obj * osal_memory_buffer_handle_t;

/*******************************************************************************************************************//**
 * @typedef fp_osal_memory_mngr_cb_t
 * @brief   event callback function for memory manager.
 * @param[in]   user_arg    value is the unchanged
 * @param[in]   event       Memory Manager Event
 * @param[in]   error       osal return value
 * @param[in]   buffer_hndl handle of a buffer object
 * @see     R_OSAL_MmngrRegisterMonitor() for more details.
 **********************************************************************************************************************/
typedef void(fp_osal_memory_mngr_cb_t)(void * user_arg, e_osal_mmngr_event_t event, e_osal_return_t error,
                                       osal_memory_buffer_handle_t buffer_hndl);

/*******************************************************************************************************************//**
 * @typedef st_osal_mmngr_config_t
 * @brief   This type is the typedef definition of struct st_osal_mmngr_config.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct  st_osal_mmngr_config
 * @brief   configuration of Memory Manager.
 * 
 * (member variables of this structure)
 * - memory_limit: Value range [4KB -- #OSAL_MEMORY_MANAGER_CFG_UNLIMITED-1] [Safety Criteria] \n
 *                 If #OSAL_MEMORY_MANAGER_CFG_UNLIMITED is specified in R_OSAL_MmngrOpen(), 
 *                 the maximum memory size area is allocated. \n
 *                 The value acquired by R_OSAL_MmngrGetOsalMaxConfig() may be lower than the actual available memory 
 *                 and shall not be larger than the available memory.
 * - max_allowed_allocations: Value range [1 -- #OSAL_MEMORY_MANAGER_CFG_UNLIMITED-1] [Safety Criteria] \n
 *                            Implementations may use this value to allocate memory required for operation overhead. \n
 *                            If #OSAL_MEMORY_MANAGER_CFG_UNLIMITED is specified in R_OSAL_MmngrOpen(), 
 *                            this function does not allocate bookkeeping area.
 * - max_registered_monitors_cbs: Value range [0 -- #OSAL_MEMORY_MANAGER_CFG_UNLIMITED-1] [Safety Criteria] \n
 *                                Implementation may use this value to allocate memory required for operation overhead.
 *                                \n If #OSAL_MEMORY_MANAGER_CFG_UNLIMITED is specified in R_OSAL_MmngrOpen(), 
 *                                this function does not allocate an bookkeeping area for the monitor function.
 **********************************************************************************************************************/
/* PRQA S 3630 1 # This definition must be published because the user references the member after calling */
typedef struct st_osal_mmngr_config
{
    e_osal_mmngr_allocator_mode_t mode; /*!< Allocation mode the memory manager uses */
    size_t memory_limit;                /*!< Set the upper limit of the finite value of memory used by this instance. \n
                                         *   For more details, refer to the description section of this structure. */
    size_t max_allowed_allocations;     /*!< Set the maximum allowed allocations for finite values. \n
                                         *   For more details, refer to the description section of this structure. */
    size_t max_registered_monitors_cbs; /*!< Limitation of the monitor functions to be registered (finite value). \n
                                         *   For more details, refer to the description section of this structure. */
} st_osal_mmngr_config_t;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* OSAL_TYPES_H */
/*======================================================================================================================
End of File
======================================================================================================================*/

