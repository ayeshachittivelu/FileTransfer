#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Target: ARMv8-A Cortex-A55
 * Mode  : EL1 (Non-secure)
 * Timer : Virtual Timer (CNTV_EL0)
 *----------------------------------------------------------*/

/*-----------------------------------------------------------
 * Clocking
 *----------------------------------------------------------*/

/* Set to your VM virtual timer frequency */
#define configCPU_CLOCK_HZ            ( 24000000UL )   /* Adjust to CNTFRQ_EL0 */
#define configTICK_RATE_HZ            ( 100U )

/*-----------------------------------------------------------
 * Scheduler
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION          1
#define configUSE_TIME_SLICING        1
#define configUSE_TICKLESS_IDLE       0

#define configMAX_PRIORITIES          5
#define configMINIMAL_STACK_SIZE      256
#define configMAX_TASK_NAME_LEN       16

#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_64_BITS


/*-----------------------------------------------------------
 * Memory
 *----------------------------------------------------------*/

#define configSUPPORT_STATIC_ALLOCATION   1
#define configSUPPORT_DYNAMIC_ALLOCATION  1

#define configTOTAL_HEAP_SIZE             ( 128 * 1024 )

#define configAPPLICATION_ALLOCATED_HEAP  0

/*-----------------------------------------------------------
 * Interrupt Priorities (GICv3 virtualized)
 *----------------------------------------------------------*/

#define configKERNEL_INTERRUPT_PRIORITY        0
#define configMAX_SYSCALL_INTERRUPT_PRIORITY   0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUNIQUE_INTERRUPT_PRIORITIES       256

/*-----------------------------------------------------------
 * Timer setup (CRITICAL)
 *----------------------------------------------------------*/

/* This calls your function in port.c */
#define configSETUP_TICK_INTERRUPT()    vConfigureTickInterrupt()


/*-----------------------------------------------------------
 * Hooks
 *----------------------------------------------------------*/

#define configUSE_IDLE_HOOK            0
#define configUSE_TICK_HOOK            0
#define configCHECK_FOR_STACK_OVERFLOW 2


/*-----------------------------------------------------------
 * Optional Features
 *----------------------------------------------------------*/

#define configUSE_MUTEXES              1
#define configUSE_RECURSIVE_MUTEXES    1
#define configUSE_COUNTING_SEMAPHORES  1
#define configUSE_TIMERS               1

#define configTIMER_TASK_PRIORITY      ( configMAX_PRIORITIES - 1 )
#define configTIMER_TASK_STACK_DEPTH   512
#define configTIMER_QUEUE_LENGTH       10

/*-----------------------------------------------------------
 * Standard ARM Generic Virtual Timer Interrupt ID
 *----------------------------------------------------------*/
#define configTICK_INTERRUPT_ID 27

/*-----------------------------------------------------------
 * API inclusion
 *----------------------------------------------------------*/

#define INCLUDE_vTaskDelay             1
#define INCLUDE_vTaskDelete            1
#define INCLUDE_vTaskSuspend           1
#define INCLUDE_vTaskDelayUntil        1
#define INCLUDE_xTaskResumeFromISR     1



/*-----------------------------------------------------------
 * ASSERT
 *----------------------------------------------------------*/

#define configASSERT( x )      \
    if( ( x ) == 0 )           \
    {                          \
        taskDISABLE_INTERRUPTS(); \
        for( ;; );             \
    }

#endif