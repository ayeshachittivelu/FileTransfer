/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_common.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper common for FreeRTOS header file
***********************************************************************************************************************/

#ifndef OSAL_COMMON_H
#define OSAL_COMMON_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pma/r_pma.h"

#include "rcar-xos/osal/r_osal.h"
#include "rcar-xos/osal_usr_cfg/freertos/r_osal_usr_cfg_info.h"
#include "rcar-xos/osal_usr_cfg/freertos/r_osal_usr_cfg_control.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_info.h"
#include "rcar-xos/osal_dev_cfg/freertos/r_osal_dev_cfg_control.h"

/*******************************************************************************************************************//**
 * @defgroup OSAL_WRAPPER_FREERTOS OSAL Wrapper For FreeRTOS
 * @ingroup osal_wrapper
 * OSAL Wrapper for FreeRTOS
 * @{
 **********************************************************************************************************************/
/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER
 * @defgroup OSAL_Private_Defines_Common Common macro definitions
 *
 * @{
 *********************************************************************************************************************/
 
/*******************************************************************************************************************//**
 * @def OSAL_DEBUG
 * debug print.
***********************************************************************************************************************/
#define OSAL_DEBUG(...)
#define OSAL_DEBUG_INFO printf

/*******************************************************************************************************************//**
 * @def OSAL_STATIC
 * STATIC for Unit test
***********************************************************************************************************************/
#ifdef UNIT_TEST
#define OSAL_STATIC
#else
#define OSAL_STATIC static
#endif

/*******************************************************************************************************************//**
 * @def OSAL_MAX_SEC
 * Max value of second
***********************************************************************************************************************/
#define OSAL_MAX_SEC   (0x7FFFFFFFL)

/*******************************************************************************************************************//**
 * @def OSAL_MAX_TV_NSEC
 * Max value of nano second
***********************************************************************************************************************/
#define OSAL_MAX_TV_NSEC (1000000000L - 1L)

/*******************************************************************************************************************//**
 * @def OSAL_SECTOMSEC
 * Mask value for converting second to miili second
***********************************************************************************************************************/
#define OSAL_SECTOMSEC (1000L)

/*******************************************************************************************************************//**
 * @def OSAL_SECTONSEC
 * Mask value for converting second to nano second
***********************************************************************************************************************/
#define OSAL_SECTONSEC (1000000000L)

/*******************************************************************************************************************//**
 * @def OSAL_MSECTONSEC
 * Mask value for converting milli second to nano second
***********************************************************************************************************************/
#define OSAL_MSECTONSEC (1000000L)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_CLOCKTIME
 * Initialization bit setting value for Clock&Time Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_CLOCKTIME  (0x01U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_MQ
 * Initialization bit setting value for Message Queue Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_MQ         (0x02U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_THREADSYNC
 * Initialization bit setting value for ThreadSync Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_THREADSYNC (0x04U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_INTERRUPT
 * Initialization bit setting value for Interrupt Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_INTERRUPT  (0x08U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_PM
 * Initialization bit setting value for Power Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_PM         (0x10U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_IO
 * Initialization bit setting value for IO Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_IO         (0x20U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_MEMORY
 * Initialization bit setting value for Memory Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_MEMORY     (0x40U)

/*******************************************************************************************************************//**
 * @def OSAL_ISINIT_THREAD
 * Initialization bit setting value for Thread Manager
***********************************************************************************************************************/
#define OSAL_ISINIT_THREAD     (0x80U)

/*******************************************************************************************************************//**
 * @def OSAL_READ_CPUID
 * OSAL_READ_CPUID macro definition for unit test
***********************************************************************************************************************/
#define OSAL_READ_CPUID(reg)  __asm volatile ("mrs %0, MPIDR_EL1" : "=r"((reg)))

/*******************************************************************************************************************//**
 * @def OSAL_CONV_PRIORITY
 * Convert priority level (30 steps -> 256 steps)
***********************************************************************************************************************/
#define OSAL_CONV_PRIORITY(prio)  ((uint16_t)(((prio) * 255) / 29))

/*******************************************************************************************************************//**
 * @def OSAL_CONV_BYTE_TO_WORD
 * Convert byte to word(4byte = 1word)
***********************************************************************************************************************/
#define OSAL_CONV_BYTE_TO_WORD(byte)  (byte / 4)

/** @} */

/*******************************************************************************************************************//**
 * @typedef osal_inner_thread_attr_t
 * @brief   thread attribute. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_inner_thread_attr.
 **********************************************************************************************************************/
typedef struct st_osal_inner_thread_attr *osal_inner_thread_attr_t;

/*******************************************************************************************************************//**
 * @typedef osal_inner_thread_handle_t
 * @brief   Handle of Thread. \n
 *          This type is a typedef definition for a pointer to a st_osal_inner_thread_control.
 **********************************************************************************************************************/
typedef struct st_osal_inner_thread_control *osal_inner_thread_handle_t;

/*******************************************************************************************************************//**
 * @typedef osal_inner_mutexattr_t
 * @brief   mutex attribute. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_inner_mutexattr.
 **********************************************************************************************************************/
typedef struct st_osal_inner_mutexattr *osal_inner_mutexattr_t;

/*******************************************************************************************************************//**
 * @typedef osal_inner_mutex_handle_t
 * @brief   Handle of Mutex. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_inner_mutex_control.
 **********************************************************************************************************************/
typedef struct st_osal_inner_mutex_control *osal_inner_mutex_handle_t;

/*******************************************************************************************************************//**
 * @typedef osal_inner_cond_handle_t
 * @brief   Handle of condition. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_inner_cond_control.
 **********************************************************************************************************************/
typedef struct st_osal_inner_cond_control *osal_inner_cond_handle_t;

/*******************************************************************************************************************//**
 * @typedef osal_inner_mq_handle_t
 * @brief   Message information. \n
 *          This type is a typedef definition for a pointer to a struct st_osal_inner_mq_control.
 **********************************************************************************************************************/
typedef struct st_osal_inner_mq_control *osal_inner_mq_handle_t;

/*******************************************************************************************************************//**
 * @typedef p_osal_inner_thread_func_t
 * @brief   inner thread worker function.
 **********************************************************************************************************************/
typedef void (*p_osal_inner_thread_func_t)(void* user_arg);

/*******************************************************************************************************************//**
 * @struct osal_inner_thread_attr_t
 * structure to thread attribute.
***********************************************************************************************************************/
typedef struct st_osal_inner_thread_attr
{
    void* dmy;
}st_osal_inner_thread_attr_t;

/*******************************************************************************************************************//**
 * @struct st_osal_inner_thread_control_t
 * structure to Handle of thread.
***********************************************************************************************************************/
typedef struct st_osal_inner_thread_control
{
    TaskHandle_t        thread;         /*!< Handle of thread for FreeRTOS */
    uint16_t            num;            /*!< Index number containing the handle */
}st_osal_inner_thread_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_inner_mutexattr_t
 * structure to mutex attribute.
***********************************************************************************************************************/
typedef struct st_osal_inner_mutexattr
{
    void* dmy;
}st_osal_inner_mutexattr_t;

/*******************************************************************************************************************//**
 * @struct st_osal_inner_mutex_control_t
 * structure to Handle of Mutex.
***********************************************************************************************************************/
typedef struct st_osal_inner_mutex_control
{
    SemaphoreHandle_t     mutex;         /*!< Handle of Mutex of FreeRTOS */
}st_osal_inner_mutex_control_t;

/*******************************************************************************************************************//**
 * @enum e_osal_inner_thread_policy_t
 * Thread Policy definitions.
***********************************************************************************************************************/
typedef enum e_osal_inner_thread_policy
{
    OSAL_SCHED_INVALID = 0,
    OSAL_SCHED_FIFO,
    OSAL_SCHED_RR
}e_osal_inner_thread_policy_t;

/*******************************************************************************************************************//**
 * @st_osal_inner_thread_attrinfo_t
 * structure to thread attribute information.
***********************************************************************************************************************/
/* PRQA S 3630 1 # This definition must be published because the another module reference the member after calling. */
typedef struct st_osal_inner_thread_attrinfo
{
    e_osal_inner_thread_policy_t policy;
    e_osal_thread_priority_t     priority;
    size_t                       stack_size;
} st_osal_inner_thread_attrinfo_t;

/*******************************************************************************************************************//**
 * @struct st_osal_inner_thread_config_t
 * structure to thread configuration.
***********************************************************************************************************************/
/* PRQA S 3630 1 # This definition must be published because the another module reference the member after calling. */
typedef struct st_osal_inner_thread_config
{
    osal_inner_thread_attr_t    *attr;
    p_osal_inner_thread_func_t  func;      /*!< Worker function */
    void*                       userarg;   /*!< Pointer of Argument for worker function */
} st_osal_inner_thread_config_t;

/*******************************************************************************************************************//**
 * @struct st_osal_inner_mq_config_t
 * structure to Message queue configuration.
***********************************************************************************************************************/
/* PRQA S 3630 1 # This definition must be published because the another module reference the member after calling. */
typedef struct st_osal_inner_mq_config
{
    uint64_t max_num_msg;
    uint64_t msg_size;
} st_osal_inner_mq_config_t;


/*******************************************************************************************************************//**
 * @struct st_osal_inner_mq_control_t
 * structure to Handle of message.
***********************************************************************************************************************/
typedef struct st_osal_inner_mq_control
{
    bool                      send_wait;     /*!< Send wait flag */
    bool                      receive_wait;  /*!< Receive wait flag */
    st_osal_queue_t           *queue;        /*!< Pointer of queue */
    int32_t                   head;          /*!< Element number of the next queue to read */
    int32_t                   tail;          /*!< Element number of the next queue to write */
    SemaphoreHandle_t         inner_mtx;     /*!< Internal mutex object */
    osal_milli_sec_t          inner_timeout; /*!< Waiting time of lock inner_mtx */
//    pthread_cond_t            send_cond;     /*!< Handle of Condition Variable for waiting to send */
//    pthread_cond_t            receive_cond;  /*!< Handle of Condition Variable for waiting to receive */
    QueueHandle_t mqd;                      /*!< Handle of Message queue for FreeRTOS */
    uint32_t                  numofmsg;      /*!< Number of messages in the queue */
    st_osal_inner_mq_config_t config;        /*!< Structure to Message queue configuration */
}st_osal_inner_mq_control_t;

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER
 * @defgroup OSAL_Private_Defines_Static_Variables_Common Common Private static variables
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @var ullPortInterruptNesting
 * The value that determines whether it is an interrupt context in FreeRTOS
***********************************************************************************************************************/
extern uint32_t ulPortInterruptNesting;

/** @} */

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER
 * @defgroup OSAL_Private_Functions_Common Common function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Initialize Thread Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_thread_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Thread Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_thread_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Thread Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_thread_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the Thread Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_thread_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Initialize ThreadSync Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_threadsync_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize ThreadSync Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_threadsync_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize ThreadSync Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_threadsync_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the ThreadSync Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_threadsync_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Initialize Message Queue Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_mq_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Message Queue Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_mq_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Message Queue Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_mq_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the Message Queue Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_mq_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Resolve all SoC specific dependencies (choose correct device and resource settings).
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
***********************************************************************************************************************/
extern e_osal_return_t osal_resolve_soc_dependencies(void);

/*******************************************************************************************************************//**
* @brief            Initialize Clock Time Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
***********************************************************************************************************************/
extern e_osal_return_t osal_clock_time_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Clock Time Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_clock_time_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Clock Time Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_clock_time_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the Clock Time Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_clock_time_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Initialize interrupt Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_interrupt_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize interrupt Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_DEV
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_interrupt_deinitialize(void);

/***********************************************************************************************************************
* @brief            Pre-check for Deinitialize interrupt Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_interrupt_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the interrupt Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_interrupt_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Initialize IO Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_io_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize IO Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_io_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize IO Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_io_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the IO Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_io_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Initialize Power Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_MEM
* @retval           OSAL_ERROR_CONF
* @retval           OSAL_ERROR_FAIL
* @retval           OSAL_ERROR_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_pm_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Power Manager.
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t osal_pm_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Power Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_pm_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the Power Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_pm_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Initialize Memory Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
extern e_osal_return_t osal_memory_initialize(void);

/*******************************************************************************************************************//**
* @brief            Deinitialize Memory Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_memory_deinitialize(void);

/*******************************************************************************************************************//**
* @brief            Pre-check for Deinitialize Memory Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
extern e_osal_return_t osal_memory_deinitialize_pre(void);

/*******************************************************************************************************************//**
* @brief            Set the Memory Manager initialization flag to true.
* @return           void
***********************************************************************************************************************/
extern void osal_memory_set_is_init(void);

/*******************************************************************************************************************//**
* @brief            Get ISR Context
* @param[out]       is_rsr  To set the address of isr status
* @return           none
***********************************************************************************************************************/
extern void R_OSAL_Internal_GetISRContext(bool *is_rsr);

/*******************************************************************************************************************//**
* @brief            Check Timestamp
* @param[in]        p_time_stamp  To set the time stamp
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_PAR
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_CheckTimestamp(const st_osal_time_t *const p_time_stamp);

/*******************************************************************************************************************//**
* @brief            Get Timestamp
* @param[out]       p_time_stamp  To set the address of time stamp
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_PAR
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_GetTimestamp(st_osal_time_t *const p_time_stamp);

/*******************************************************************************************************************//**
* @brief            Convert tick time to millisec
* @param[out]       time_period  To set the time period
* @param[in]        tick  To set the address of tick time
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_PAR
***********************************************************************************************************************/
extern void R_OSAL_Internal_TickToMillisec(osal_milli_sec_t *time_period, TickType_t tick);

/*******************************************************************************************************************//**
* @brief            Convert millisec to tick time
* @param[in]        time_period  To set the time period
* @param[out]       tick  To set the address of tick time
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_PAR
***********************************************************************************************************************/
extern void R_OSAL_Internal_MillisecToTick(osal_milli_sec_t time_period, TickType_t *tick);

/*******************************************************************************************************************//**
* @brief            Get difference time
* @param[in]        p_time2  To set the time stamp
* @param[in]        p_time1  To set the time stamp
* @param[out]       p_time_difference  To set the address of difference time
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_PAR
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_GetDiffTime(const st_osal_time_t *const p_time2,
                                                   const st_osal_time_t *const p_time1,
                                                   osal_nano_sec_t *const p_time_difference);

/*******************************************************************************************************************//**
* @brief            Calcurate difference time
* @param[in]        p_time2  To set the time stamp
* @param[in]        p_time1  To set the time stamp
* @param[out]       p_time_difference  To set the address of difference time
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_PAR
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_CalcDiffTime(const st_osal_time_t *const p_time2,
                                                    const st_osal_time_t *const p_time1,
                                                    osal_nano_sec_t *const p_time_difference);

/*******************************************************************************************************************//**
* @brief            Get Head from List
* @param[in]        list_head  To set the base list pointer
* @return           st_osal_event_list_t
* @retval           st_osal_event_list_t data
***********************************************************************************************************************/
extern st_osal_event_list_t* R_OSAL_Internal_GetHeadFromList(st_osal_event_list_t **list_head);

/*******************************************************************************************************************//**
* @brief            Add to list
* @param[in]        list  To set the base list pointer
* @param[in]        add_list  To set the additional list poinnter
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_AddList(st_osal_event_list_t **list, st_osal_event_list_t *add_list);

/*******************************************************************************************************************//**
* @brief            Remove an item from the list
* @param[in]        list  To set the base list pointer
* @param[in]        remove_list  To set the list poinnter to be removed
* @return           e_osal_return_t
* @retval           OSAL_ERROR_OK
* @retval           OSAL_ERROR_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_RemoveList(st_osal_event_list_t **list, st_osal_event_list_t *remove_list);

/*******************************************************************************************************************//**
* @brief            Check list
* @param[in]        list_head  To set the base list pointer
* @return           bool
* @retval           true list is empty
* @retval           false list is not empty
***********************************************************************************************************************/
extern bool R_OSAL_Internal_CheckListEmpty(const st_osal_event_list_t * const list_head);

/*******************************************************************************************************************//**
* @brief            call callback function
* @return           void
***********************************************************************************************************************/
extern void R_OSAL_Internal_BusInterfaceCallBack ( void );

/*******************************************************************************************************************//**
* @brief            Sleep in milli second.
* @param[in]        milliseconds  Specify the sleep time in milliseconds.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MsSleep(osal_milli_sec_t milliseconds);

/*******************************************************************************************************************//**
* @brief            Set the attribute.
* @param[in]        p_attr_info    A pointer to thread attribute information.
* @param[in,out]    p_attr         A pointer to osal_inner_thread_attr_t.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_SetThreadAttr(const st_osal_inner_thread_attrinfo_t *const p_attr_info,
                                                     osal_inner_thread_attr_t *const p_attr);

/*******************************************************************************************************************//**
* @brief            Create a new thread in the calling process as the configuration specified in p_config.
* @param[in]        p_config       A pointer to thread configuration.
* @param[in,out]    p_handle       A pointer to Handle of Thread.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_ThreadCreate(const st_osal_inner_thread_config_t *const p_config,
                                                    osal_inner_thread_handle_t *const p_handle);

/*******************************************************************************************************************//**
* @brief            Join a thread, with a time limit.
* @param[in]        handle         Handle of Thread.
* @param[out]       p_return_value A pointer to NULL, or a pointer to a location where the function can store the value.
* @param[in]        time_period    Specify the timeout time in milliseconds.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_ThreadTimedJoin(osal_inner_thread_handle_t const handle,
                                                       int64_t *const p_return_value,
                                                       const osal_milli_sec_t time_period);

/*******************************************************************************************************************//**
* @brief            Set the mutex attribute.
* @param[in,out]    p_mutex_attr   A pointer to mutex attribute.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_SetMutexAttr(osal_inner_mutexattr_t *const p_mutex_attr);

/*******************************************************************************************************************//**
* @brief            Initialize mutex assigned and set acquired mutex handle to p_handle.
* @param[in]        p_mutex_attr   A pointer to mutex attribute.
* @param[in,out]    p_handle       The address of osal_inner_mutex_handle_t.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MutexInit(const osal_inner_mutexattr_t *const p_mutex_attr,
                                                 osal_inner_mutex_handle_t *const p_handle);

/*******************************************************************************************************************//**
* @brief            Lock a mutex(timed lock).
* @param[in]        handle        The address of osal_inner_mutex_handle_t.
* @param[in]        time_period   Specify the timeout time in milliseconds.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_TIME
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MutexTimedLock(osal_inner_mutex_handle_t handle,
                                                      osal_milli_sec_t time_period);

/*******************************************************************************************************************//**
* @brief            Unlock mutex assigned to handle.
* @param[in]        handle         The address of osal_inner_mutex_handle_t.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Internal_MutexUnlock(osal_inner_mutex_handle_t handle);

/*******************************************************************************************************************//**
* @brief            Destroy mutex assigned to handle.
* @param[in]        handle         The address of osal_inner_mutex_handle_t.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MutexDestroy(osal_inner_mutex_handle_t handle);

/*******************************************************************************************************************//**
* @brief            Create the message queue in the configuration specified in p_config.
* @param[in]        p_config       A pointer to Message queue configuration.
* @param[in,out]    p_handle       The address of osal_inner_mq_handle_t. 
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_CONF
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MqCreate(const st_osal_inner_mq_config_t *const p_config,
                                                osal_inner_mq_handle_t *const p_handle);

/*******************************************************************************************************************//**
* @brief            Send a message to message queue with timeout until time stamp.
* @param[in]        handle         The address of osal_inner_mq_handle_t.
* @param[in]        time_period    Specify the timeout time in milliseconds.
* @param[in]        p_buffer       The address of void* for the send data.
* @param[in]        buffer_size    The p_buffer size by Byte. 
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_TIME
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MqTimedSend(osal_inner_mq_handle_t handle,
                                                   osal_milli_sec_t time_period,
                                                   const void * p_buffer,
                                                   size_t buffer_size);

/*******************************************************************************************************************//**
* @brief            Receive a message to message queue with timeout until time stamp.
* @param[in]        handle         The address of osal_inner_mq_handle_t.
* @param[in]        time_period    Specify the timeout time in milliseconds.
* @param[out]       p_buffer       The address of void* for the receive data.
* @param[in]        buffer_size    The p_buffer size by Byte. 
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_TIME
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MqTimedReceive(osal_inner_mq_handle_t handle,
                                                      osal_milli_sec_t time_period,
                                                      void * p_buffer,
                                                      size_t buffer_size);

/*******************************************************************************************************************//**
* @brief            Delete message queue assigned to handle.
* @param[in]        handle         The address of osal_inner_mq_handle_t.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_MqDelete(osal_inner_mq_handle_t handle);

/*******************************************************************************************************************//**
* @brief            Acquire the time stamp on system according to HIGH_RESOLUTION.
* @param[out]       p_time_stamp   Set the pointer address of st_osal_time_t as timestamp.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_GetHighResoTimeStamp(st_osal_time_t *const p_time_stamp);

/*******************************************************************************************************************//**
* @brief           Acquire the difference between two times.
* @param[in]       p_time2           Set the pointer address of st_osal_time_t as timestamp.
* @param[in]       p_time1           Set the pointer address of st_osal_time_t as timestamp.
* @param[in,out]   p_time_difference To set the address of osal_nano_sec_t.
*                                    The OSAL sets the difference between time2 and time1 with a signed value.
* @return          e_osal_return_t
* @retval          OSAL_RETURN_OK
* @retval          OSAL_RETURN_FAIL
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_CalcTimeDifference(const st_osal_time_t *const p_time2, 
                                                          const st_osal_time_t *const p_time1,
                                                          osal_nano_sec_t *const p_time_difference);

/** @} */

/** @} OSAL_WRAPPER_FREERTOS */

#endif /* OSAL_COMMON_H */
