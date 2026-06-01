#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions */
#define portSTACK_TYPE          uint64_t
#define portBASE_TYPE           long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;
typedef uint64_t TickType_t;

/* CRITICAL FIX */
typedef uintptr_t           portPOINTER_SIZE_TYPE;

/* Architecture specifics */
#define portSTACK_GROWTH        ( -1 )
#define portTICK_PERIOD_MS      ( ( TickType_t ) 1000 / configTICK_RATE_HZ )

#define portBYTE_ALIGNMENT      16/* 64-bit tick */
//#define portBYTE_ALIGNMENT_MASK     ( portBYTE_ALIGNMENT - 1 )
#define portTICK_TYPE_IS_ATOMIC    1

/* Max delay */
#define portMAX_DELAY              ( TickType_t )0xffffffffffffffffULL

/* Critical section */
extern void vPortEnterCritical(void);
extern void vPortExitCritical(void);

#define portENTER_CRITICAL()       vPortEnterCritical()
#define portEXIT_CRITICAL()        vPortExitCritical()

#define portYIELD()             __asm volatile ( "svc 0" )

#define portDISABLE_INTERRUPTS()    __asm volatile ( "msr daifset, #2" ::: "memory" )
#define portENABLE_INTERRUPTS()     __asm volatile ( "msr daifclr, #2" ::: "memory" )

#define portNOP() __asm volatile ( "nop" )
/* Task function macros */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) \
    void vFunction( void *pvParameters )

#define portTASK_FUNCTION( vFunction, pvParameters ) \
    void vFunction( void *pvParameters )

#ifdef __cplusplus
}
#endif

#endif