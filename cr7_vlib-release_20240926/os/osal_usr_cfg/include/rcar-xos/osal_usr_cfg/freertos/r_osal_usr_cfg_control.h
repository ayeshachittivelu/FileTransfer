/*************************************************************************************************************
* OSAL Resource Configurator
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_usr_cfg_control.h
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL resource configuration for FreeRTOS header file
***********************************************************************************************************************/

#ifndef OSAL_USR_CFG_CONTROL_H
#define OSAL_USR_CFG_CONTROL_H

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_USR_CFG
 * @defgroup OSAL_Public_Defines_UsrConfig_control User Configurator Public macro definitions (control)
 *
 * @{
 **********************************************************************************************************************/
#if 0
/*******************************************************************************************************************//**
 * @def OSAL_THREAD_MS_SLEEP_TIME
 * Sleep time[ms].
***********************************************************************************************************************/
#define OSAL_THREAD_MS_SLEEP_TIME    (1)

/*******************************************************************************************************************//**
 * @def OSAL_MESSAGE_MS_SLEEP_TIME
 * Sleep time[ms].
***********************************************************************************************************************/
#define OSAL_MESSAGE_MS_SLEEP_TIME   (1)

/*******************************************************************************************************************//**
 * @def OSAL_INTERRUPT_MS_SLEEP_TIME
 * Sleep time[ms].
***********************************************************************************************************************/
#define OSAL_INTERRUPT_MS_SLEEP_TIME (1)

/*******************************************************************************************************************//**
 * @def OSAL_THREAD_PRIO_BASE
 * Thread base priority.
***********************************************************************************************************************/
#define OSAL_THREAD_PRIO_BASE           (1L)

/*******************************************************************************************************************//**
 * @def OSAL_TH_JOIN_TIMEOUT
 * Value for timeout used in thread join wait.
***********************************************************************************************************************/
#define OSAL_TH_JOIN_TIMEOUT            (2500L)

/*******************************************************************************************************************//**
 * @def OSAL_INTERRUPT_PRIO_BASE
 * Interrupt thread base priority.
***********************************************************************************************************************/
#define OSAL_INTERRUPT_PRIO_BASE        (16L)

/*******************************************************************************************************************//**
 * @def OSAL_INTERRUPT_SPI_OFFSET
 * Offset to match the hardware interrupt number.
***********************************************************************************************************************/
#define OSAL_INTERRUPT_SPI_OFFSET       (32UL)

/*******************************************************************************************************************//**
 * @def OSAL_INTERRUPT_ACTIVE_TIMEOUT
 * Value for timeout used in interrupt thread active check.
***********************************************************************************************************************/
#define OSAL_INTERRUPT_ACTIVE_TIMEOUT   (500L)

/*******************************************************************************************************************//**
 * @def OSAL_MUTEX_DESTROY_TIMEOUT
 * Mutex destruction timeout value [msec].
***********************************************************************************************************************/
#define OSAL_MUTEX_DESTROY_TIMEOUT      (500L)

#endif

/*******************************************************************************************************************//**
 * @def OSAL_RCFG_THREAD_MAX_NUM
 * Max value of the thread control table.
***********************************************************************************************************************/
#define OSAL_RCFG_THREAD_MAX_NUM        (2048)

/*******************************************************************************************************************//**
 * @def OSAL_RCFG_MUTEX_MAX_NUM
 * Max value of the mutex control table.
***********************************************************************************************************************/
#define OSAL_RCFG_MUTEX_MAX_NUM         (1024)

/*******************************************************************************************************************//**
 * @def OSAL_RCFG_COND_MAX_NUM
 * Max value of the condition variable control table.
***********************************************************************************************************************/
#define OSAL_RCFG_COND_MAX_NUM          (1024)

/*******************************************************************************************************************//**
 * @def OSAL_RCFG_MQ_MAX_NUM
 * Max value of the message queue control table.
***********************************************************************************************************************/
#define OSAL_RCFG_MQ_MAX_NUM            (1024)

/*******************************************************************************************************************//**
 * @def OSAL_RCFG_INTERRUPT_MAX_NUM
 * Max value of the interrupt thread control table.
***********************************************************************************************************************/
#define OSAL_RCFG_INTERRUPT_MAX_NUM     (256)

/*******************************************************************************************************************//**
 * @def OSAL_THREAD_HANDLE_ID
 * Handle id for thread.
***********************************************************************************************************************/
#define OSAL_THREAD_HANDLE_ID       (0x4F53414C5F544852ULL) /* ascii code: OSAL_THR */

/*******************************************************************************************************************//**
 * @def OSAL_MUTEX_HANDLE_ID
 * Handle id for mutex.
***********************************************************************************************************************/
#define OSAL_MUTEX_HANDLE_ID        (0x4F53414C5F4D5458ULL) /* ascii code: OSAL_MTX */

/*******************************************************************************************************************//**
 * @def OSAL_COND_HANDLE_ID
 * Handle id for mutex.
***********************************************************************************************************************/
#define OSAL_COND_HANDLE_ID         (0x4F53414C5F434E44ULL) /* ascii code: OSAL_CND */

/*******************************************************************************************************************//**
 * @def OSAL_MESSAGE_HANDLE_ID
 * Handle id for message queue.
***********************************************************************************************************************/
#define OSAL_MESSAGE_HANDLE_ID      (0x4F53414C5F4D5347ULL) /* ascii code: OSAL_MSG */

/*******************************************************************************************************************//**
 * @def OSAL_INTERRUPT_HANDLE_ID
 * Handle id for interrupt thread.
***********************************************************************************************************************/
#define OSAL_INTERRUPT_HANDLE_ID    (0x4F53414C5F494E54ULL) /* ascii code: OSAL_INT */

/*******************************************************************************************************************//**
 * @def OSAL_MAX_EVENT_BITS
 * Maximum valeu of event bit.
***********************************************************************************************************************/
#if configUSE_16_BIT_TICKS == 0
#define OSAL_MAX_EVENT_BITS (24U)
#elif configUSE_16_BIT_TICKS == 1
#define OSAL_MAX_EVENT_BITS (8U)
#endif

/*******************************************************************************************************************//**
 * @def OSAL_EVENT_CLEAR_BITS
 * event clear target.
***********************************************************************************************************************/
#if configUSE_16_BIT_TICKS == 0
#define OSAL_EVENT_CLEAR_BITS (0x00ffffffU)
#elif configUSE_16_BIT_TICKS == 1
#define OSAL_EVENT_CLEAR_BITS (0x000000ffU)
#endif

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_USR_CFG
 * @defgroup OSAL_Public_Type_UsrConfig_control User Configurator Public type definitions (control)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @struct st_osal_thread_control_t
 * structure to control thread.
***********************************************************************************************************************/
typedef struct st_osal_thread_control
{
    uint64_t                handle_id;      /*!< Fixed value: OSAL_THREAD_HANDLE_ID */

    bool                    use;
    bool                    active;         /*!< thread active flag (true: hread is active, false: thread is inactive) */
    bool                    joined;

    SemaphoreHandle_t       inner_mtx;      /*!< Internal mutex object */
    osal_milli_sec_t        inner_timeout;  /*!< Waiting time of lock inner_mtx */

    TaskHandle_t            thread;         /*!< Handle of thread for FreeRTOS */
    TaskHandle_t            parent_thread;  /*!< Determine thread has already been joined by another thread or not */
    int64_t                 thread_ret;     /*!< Return value of worker function. refer to join */

    st_osal_thread_config_t config;          /*!< thread configuration parameter */
    const struct st_osal_rcfg_thread_config *usr_config;      /*!< User configuration for thread */

}st_osal_thread_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_mutex_control_t
 * structure to control mutex.
***********************************************************************************************************************/
typedef struct st_osal_mutex_control
{
    uint64_t            handle_id;          /*!< Fixed value: OSAL_MUTEX_HANDLE_ID */

    bool                use;                /*!< Indicates lock state(true: mutex is lock, false: mutex is unlock) */
    bool                active;             /*!< Mutex active flag (true: already called create, false: no called create) */

    SemaphoreHandle_t inner_mtx;            /*!< Mutex for control inside OSAL */
    osal_milli_sec_t    inner_timeout;      /*!< Waiting time of lock mutex object */

    SemaphoreHandle_t     mtx;            /*!< Handle of Mutex of FREERTOS */

    const struct st_osal_rcfg_mutex_config *usr_config; /*!< User configuration for mutex */

}st_osal_mutex_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_event_list_t
 * structure to control event.
***********************************************************************************************************************/
typedef struct st_osal_event_list {

    EventBits_t bits;                       /*!< Event bit by using FreeRTOS. */
    struct st_osal_event_list* next;        /*!< Next list pointer */

}st_osal_event_list_t;

/*******************************************************************************************************************//**
 * @struct st_osal_cond_control_t
 * structure to control cond.
***********************************************************************************************************************/
typedef struct st_osal_cond_control
{
    uint64_t            handle_id;          /*!< Fixed value: OSAL_COND_HANDLE_ID */

    bool                active;             /*!< Cond active flag (true: already called create, false: no called create) */

    SemaphoreHandle_t   inner_mtx;          /*!< Internal mutex object */
    osal_milli_sec_t    inner_timeout;      /*!< Waiting time of lock inner_mtx */
    int64_t inner_cond_timeout;             /*!< Waiting time of relocking mutex */

    EventGroupHandle_t event;               /*!< Handle of Event for FreeRTOS */
    st_osal_event_list_t bits_pattern[OSAL_MAX_EVENT_BITS];	/*!< Bit pattern for event */
    st_osal_event_list_t *empty_list;       /*!< Empty event list */
    st_osal_event_list_t *wait_list;        /*!< Wait event list */

    const struct st_osal_rcfg_cond_config *usr_config; /*!< User configuration for cndition variable */

}st_osal_cond_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_queue_t
 * structure to queue.
***********************************************************************************************************************/
typedef struct st_osal_queue
{
  void                  *p_buffer;          /*!< Buffer to store messages */

}st_osal_queue_t;

/*******************************************************************************************************************//**
 * @struct st_osal_mq_control_t
 * structure to control message queue.
***********************************************************************************************************************/
typedef struct st_osal_mq_control
{
    uint64_t            handle_id;          /*!< Fixed value: OSAL_MESSAGE_HANDLE_ID */
    bool                active;             /*!< message queue active flag (true: already called create, false: no called create) */

    bool                send_wait;
    bool                receive_wait;

    SemaphoreHandle_t     inner_mtx;          /*!< Internal mutex object */
    osal_milli_sec_t    inner_timeout;      /*!< Waiting time of lock inner_mtx */

//    pthread_cond_t      send_cond;          /*!< Handle of Condition Variable for waiting to send */
//    pthread_cond_t      receive_cond;       /*!< Handle of Condition Variable for waiting to receive */

    QueueHandle_t mqd;                      /*!< Handle of Message queue for FreeRTOS */

    st_osal_mq_config_t config;             /*!< message queue configuration parameter */
    const struct st_osal_rcfg_mq_config *usr_config; /*!< User configuration for message queue */

}st_osal_mq_control_t;

/*******************************************************************************************************************//**
 * @struct st_osal_interrupt_control_t
 * structure to control interrupt thread.
***********************************************************************************************************************/
typedef struct st_osal_interrupt_control
{
    uint64_t              handle_id;        /*!< Fixed value: OSAL_INTERRUPT_HANDLE_ID */

    bool                  init_state;       /*!< Initial flag(true: already initialized, false: initialize not yet) */
    bool                  active;           /*!< handle state(true: handle is active, false: handle is inactive) */

    SemaphoreHandle_t     inner_mtx;        /*!< Internal mutex object */
    osal_milli_sec_t      inner_timeout;    /*!< Waiting time of lock inner_mtx */

    osal_device_handle_t  deviceHandle;     /*!< Handle of device according this interrupt */

    p_osal_isr_func_t     irq_func;         /*!< Pointer of Worker function of Interrupt function */
    void                  *irq_arg;         /*!< Pointer of Worker function argument */
    uint64_t              irq_channel;      /*!< irq_channel for device information */
    bool                  irq_enable;       /*!< Interrupt status(true: Enable interrupt, false: Disable interrupt) */
    bool                  irq_use;          /*!< Interrupt use(true: using interrupt, false: not using interrupt) */


    const struct st_osal_interrupt_config *usr_config; /*!< User configuration for message queue */

}st_osal_interrupt_control_t;

/** @} */

/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS_USR_CFG
 * @defgroup OSAL_Public_Function_UsrConfig User Condifurator Public function definitions (control)
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
* @brief            Get number of Thread Control information.
* @return           size_t  number of thread Control information
***********************************************************************************************************************/
size_t R_OSAL_RCFG_GetNumOfThread(void);

/*******************************************************************************************************************//**
* @brief            Get Thread Control information.
* @param[in]        num      To set the thread number
* @param[out]       pp_info  To set the address of the thread control structure pointer
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_RCFG_GetThreadInfo(uint32_t num, st_osal_thread_control_t **pp_info);

/*******************************************************************************************************************//**
* @brief            Get number of Interrupt Thread Control information.
* @return           size_t  number of Interrupt Thread Control information
***********************************************************************************************************************/
size_t R_OSAL_RCFG_GetNumOfInterruptThread(void);

/*******************************************************************************************************************//**
* @brief            Get Interrupt Thread Control information.
* @param[in]        num      To set the interrupt thread number
* @param[out]       pp_info  To set the address of the interrupt thread control structure pointer
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_RCFG_GetInterruptThreadInfo(uint32_t num, st_osal_interrupt_control_t **pp_info);

/*******************************************************************************************************************//**
* @brief            Get number of Mutex Control information.
* @return           size_t number of Mutex Control information
***********************************************************************************************************************/
size_t R_OSAL_RCFG_GetNumOfMutex(void);

/*******************************************************************************************************************//**
* @brief            Get Mutex Control information.
* @param[in]        num      To set the mutex number
* @param[out]       pp_info  To set the address of the mutex control structure pointer
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_RCFG_GetMutexInfo(uint32_t num, st_osal_mutex_control_t **pp_info);

/*******************************************************************************************************************//**
* @brief            Get number of Condition variable Control information.
* @return           size_t  number of Condition variable Control information
***********************************************************************************************************************/
size_t R_OSAL_RCFG_GetNumOfCond(void);

/*******************************************************************************************************************//**
* @brief            Get Condition variable Control information.
* @param[in]        num      To set the condition variable number
* @param[out]       pp_info  To set the address of the condition variable control structure pointer
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_RCFG_GetCondInfo(uint32_t num, st_osal_cond_control_t **pp_info);

/*******************************************************************************************************************//**
* @brief            Get number of Message queue Control information.
* @return           size_t  number of Message queue Control information
***********************************************************************************************************************/
size_t R_OSAL_RCFG_GetNumOfMq(void);

/*******************************************************************************************************************//**
* @brief            Get Message queue Control information.
* @param[in]        num      To set the message queue number
* @param[out]       pp_info  To set the address of the message queue control structure pointer
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK   Success
* @retval           OSAL_RETURN_PAR  Parameter error
***********************************************************************************************************************/
e_osal_return_t R_OSAL_RCFG_GetMqInfo(uint32_t num, st_osal_mq_control_t **pp_info);

/** @} */

#endif /* OSAL_USR_CFG_CONTROL_H */
