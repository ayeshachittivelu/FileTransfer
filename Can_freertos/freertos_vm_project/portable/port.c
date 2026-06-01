/*
 * FreeRTOS Cortex-A55 AArch64 EL1 Guest Port
 * Designed for Hypervisor (UVMM / L4Re)
 */

#include <stdlib.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------*/
/* EL1 only */
#define portEL1                 ( 1U )
#define portNO_CRITICAL_NESTING ( 0U )
#define portDAIF_I              ( 0x80U )

#define SPSR_EL1h 0x4 | 0x1   /* EL1h mode, interrupts enabled */

/*-----------------------------------------------------------*/
/* Scheduler variables (saved in context) */

// volatile uint64_t ullCriticalNesting      = 9999ULL;
volatile uint64_t ullCriticalNesting      = 0ULL;
uint64_t ullPortTaskHasFPUContext         = pdFALSE;
uint64_t ullPortYieldRequired             = pdFALSE;
uint64_t ullPortInterruptNesting          = 0;

// /* GICv3 Redistributor Base Addresses from your DTB */
// #define GICR_BASE           0x30100000
// #define GICR_SGI_BASE       ( GICR_BASE + 0x10000 )
// #define GICR_IGROUPR0       ( *( volatile uint32_t * ) ( GICR_SGI_BASE + 0x0080 ) ) /* NEW */
// #define GICR_ISENABLER0     ( *( volatile uint32_t * ) ( GICR_SGI_BASE + 0x0100 ) )

/*-----------------------------------------------------------
 * GICv2 Register Definitions for R-Car H3 (UVMM Mapping)
 *----------------------------------------------------------*/
#define GICD_BASE               0x00040000UL  /* From your Device Tree */
#define GICC_BASE               0x00050000UL  /* From your Device Tree */

/* Distributor Registers */
#define GICD_CTLR               ( *( volatile uint32_t * ) ( GICD_BASE + 0x0000 ) )
#define GICD_ISENABLER( n )     ( *( volatile uint32_t * ) ( GICD_BASE + 0x0100 + ( ( n ) * 4 ) ) )
#define GICD_ICENABLER( n )     ( *( volatile uint32_t * ) ( GICD_BASE + 0x0180 + ( ( n ) * 4 ) ) )
#define GICD_IPRIORITYR( n )    ( *( volatile uint32_t * ) ( GICD_BASE + 0x0400 + ( ( n ) * 4 ) ) )
#define GICD_ITARGETSR( n )     ( *( volatile uint32_t * ) ( GICD_BASE + 0x0800 + ( ( n ) * 4 ) ) )

/* CPU Interface Registers */
#define GICC_CTLR               ( *( volatile uint32_t * ) ( GICC_BASE + 0x0000 ) )
#define GICC_PMR                ( *( volatile uint32_t * ) ( GICC_BASE + 0x0004 ) )
#define GICC_IAR                ( *( volatile uint32_t * ) ( GICC_BASE + 0x000C ) )
#define GICC_EOIR               ( *( volatile uint32_t * ) ( GICC_BASE + 0x0010 ) )

/*-----------------------------------------------------------*/

extern void vPortRestoreTaskContext( void );

/*-----------------------------------------------------------*/
/* Stack Initialization */

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* X0 - X1 */
    *--pxTopOfStack = 0; /* X1 */
    *--pxTopOfStack = 0; /* X0 */

    *--pxTopOfStack = 0; /* X3 */
    *--pxTopOfStack = 0; /* X2 */

    *--pxTopOfStack = 0; /* X5 */
    *--pxTopOfStack = 0; /* X4 */

    *--pxTopOfStack = 0; /* X7 */
    *--pxTopOfStack = 0; /* X6 */

    *--pxTopOfStack = 0; /* X9 */
    *--pxTopOfStack = 0; /* X8 */

    *--pxTopOfStack = 0; /* X11 */
    *--pxTopOfStack = 0; /* X10 */

    *--pxTopOfStack = 0; /* X13 */
    *--pxTopOfStack = 0; /* X12 */

    *--pxTopOfStack = 0; /* X15 */
    *--pxTopOfStack = 0; /* X14 */

    *--pxTopOfStack = 0; /* X17 */
    *--pxTopOfStack = 0; /* X16 */

    *--pxTopOfStack = 0; /* X19 */
    *--pxTopOfStack = 0; /* X18 */

    *--pxTopOfStack = 0; /* X21 */
    *--pxTopOfStack = 0; /* X20 */

    *--pxTopOfStack = 0; /* X23 */
    *--pxTopOfStack = 0; /* X22 */

    *--pxTopOfStack = 0; /* X25 */
    *--pxTopOfStack = 0; /* X24 */

    *--pxTopOfStack = 0; /* X27 */
    *--pxTopOfStack = 0; /* X26 */

    *--pxTopOfStack = 0; /* X29 */
    *--pxTopOfStack = 0; /* X28 */

    *--pxTopOfStack = 0; /* XZR */
    *--pxTopOfStack = 0; /* X30 */

    /* ELR + SPSR */
    *--pxTopOfStack = SPSR_EL1h;          /* SPSR_EL1 */
    *--pxTopOfStack = (StackType_t)pxCode;/* ELR_EL1 */

    /* dummy + critical */
    *--pxTopOfStack = portNO_CRITICAL_NESTING;
    *--pxTopOfStack = 0;

    return pxTopOfStack;
}

/*-----------------------------------------------------------*/
/* Virtual Timer Setup */

// static void vSetupTickInterrupt( void )
//  {

// uint64_t freq;
//     uint64_t cval;

//     /* Calculate and set the Timer Interval */
//     __asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
//     uint64_t interval = freq / configTICK_RATE_HZ;

//     uint64_t now;
//     __asm volatile("mrs %0, cntvct_el0" : "=r"(now));
//     cval = now + interval;

//     __asm volatile("msr cntv_cval_el0, %0" :: "r"(cval));
//     __asm volatile("msr cntv_ctl_el0, %0" :: "r"(1ULL));

//     /* Enable System Register access to GIC CPU Interface */
//     uint64_t sre;
//     __asm volatile("mrs %0, icc_sre_el1" : "=r"(sre));
//     sre |= 1; 
//     __asm volatile("msr icc_sre_el1, %0" :: "r"(sre));
//     __asm volatile("isb");

//     /* Set Priority Mask to allow all priorities */
//     __asm volatile("msr icc_pmr_el1, %0" :: "r"(0xFF));

//     /* Enable Group 1 interrupts in the CPU interface */
//     __asm volatile("msr icc_igrpen1_el1, %0" :: "r"(1ULL));

//     /* NEW: Assign all PPIs (including ID 27) and SGIs to Group 1 Non-Secure */
//     GICR_IGROUPR0 = 0xFFFFFFFF;

//     /* Unmask the FreeRTOS Tick Interrupt (ID 27) in the GIC Redistributor */
//     GICR_ISENABLER0 = ( 1 << configTICK_INTERRUPT_ID );
// }

static void vSetupTickInterrupt( void )
{
    uint64_t freq;
    uint64_t cval;

    /* 1. Set up the Core CPU Timer Registers */
    __asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    uint64_t interval = freq / configTICK_RATE_HZ;

    uint64_t now;
    __asm volatile("mrs %0, cntvct_el0" : "=r"(now));
    cval = now + interval;

    __asm volatile("msr cntv_cval_el0, %0" :: "r"(cval));
    __asm volatile("msr cntv_ctl_el0, %0" :: "r"(1ULL));
    __asm volatile("isb");

    /* 2. Configure GICv2 Distributor for Interrupt ID 27 */
    uint32_t ulParameter = configTICK_INTERRUPT_ID;
    uint32_t ulRegOffset = ulParameter / 32;
    uint32_t ulBitPosition = ulParameter % 32;

    /* Unmask/Enable Interrupt ID 27 in the Distributor */
    GICD_ISENABLER( ulRegOffset ) = ( 1U << ulBitPosition );

    /* Target this interrupt to CPU Core 0 */
    uint32_t ulByteOffset = ulParameter / 4;
    uint32_t ulBytePosition = ulParameter % 4;
    uint32_t ulTargetReg = GICD_ITARGETSR( ulByteOffset );
    
    ulTargetReg &= ~( 0xFFU << ( ulBytePosition * 8 ) );
    ulTargetReg |= ( 0x01U << ( ulBytePosition * 8 ) ); /* Target CPU 0 */
    GICD_ITARGETSR( ulByteOffset ) = ulTargetReg;

    /* 3. Configure GICv2 CPU Interface */
    GICC_PMR = 0xFFU;      /* Allow all interrupt priority levels through */
    GICC_CTLR = 0x01U;     /* Enable signaling of Secure/Group0 interrupts */
    GICD_CTLR = 0x01U;     /* Enable the GIC Distributor global logic */
}

/*-----------------------------------------------------------*/
/* Scheduler Start */

BaseType_t xPortStartScheduler( void )
{
    uint32_t ulEL;

    __asm volatile ( "MRS %0, CurrentEL" : "=r" ( ulEL ) );
    ulEL = ( ulEL >> 2 ) & 0x3;

    configASSERT( ulEL == portEL1 );

    __asm volatile ( "msr daifset, #2" ::: "memory" );

    vSetupTickInterrupt();

    __asm volatile ( "msr daifclr, #2" ::: "memory" );

    vPortRestoreTaskContext();

    return 0;
}

/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not supported */
    configASSERT( ullCriticalNesting == 1000ULL );
}

/*-----------------------------------------------------------*/
/* Critical Section Handling */

void vPortEnterCritical( void )
{
    __asm volatile ( "msr daifset, #2" ::: "memory" );
    ullCriticalNesting++;
}

/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    if( ullCriticalNesting > portNO_CRITICAL_NESTING )
    {
        ullCriticalNesting--;

        if( ullCriticalNesting == portNO_CRITICAL_NESTING )
        {
            __asm volatile ( "msr daifclr, #2" ::: "memory" );
        }
    }
}

/*-----------------------------------------------------------*/
/* Tick Handler (Called from IRQ handler in portASM.s) */

// void FreeRTOS_Tick_Handler( void )
// {
//     uint32_t ulInterruptID;
//     uint64_t ulCompare;

//     /* 1. Acknowledge the interrupt with the GIC (Read IAR) */
//     __asm volatile("mrs %0, icc_iar1_el1" : "=r"(ulInterruptID));

//     /* Check if it's actually our timer tick (ID 27) */
//     if ( ulInterruptID == configTICK_INTERRUPT_ID )
//     {
//         /* Calculate next tick interval */
//         ulCompare = (uint64_t)24000000 / configTICK_RATE_HZ;

//         /* Reload timer - clears the CPU timer interrupt condition */
//         __asm volatile("msr cntv_tval_el0, %0" :: "r"(ulCompare));
//         __asm volatile("msr cntv_ctl_el0, %0" :: "r"(1ULL));

//         /* Update kernel tick */
//         if (xTaskIncrementTick() != pdFALSE)
//         {
//             ullPortYieldRequired = pdTRUE;
//         }
//     }

//     /* 2. End of Interrupt (Write EOIR) - Tells GIC we are done */
//     __asm volatile("msr icc_eoir1_el1, %0" :: "r"(ulInterruptID));
// }

void FreeRTOS_Tick_Handler( void )
{
    uint32_t ulInterruptID;
    uint64_t ulCompare;

    /* 1. Acknowledge the interrupt by reading the GICv2 Interface Interrupt Acknowledge Register */
    ulInterruptID = GICC_IAR;

    /* Check if the active interrupt matches our virtual timer ID (27) */
    if ( ( ulInterruptID & 0x3FFU ) == configTICK_INTERRUPT_ID )
    {
        /* Calculate next baseline tick interval directly using system clocks */
        uint64_t freq;
        __asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
        ulCompare = freq / configTICK_RATE_HZ;

        /* Reload the core timer value - this drops the core's native interrupt signal line */
        __asm volatile("msr cntv_tval_el0, %0" :: "r"(ulCompare));
        __asm volatile("msr cntv_ctl_el0, %0" :: "r"(1ULL));
        __asm volatile("isb");

        /* Drive the main FreeRTOS Scheduler Engine */
        if ( xTaskIncrementTick() != pdFALSE )
        {
            ullPortYieldRequired = pdTRUE;
        }
    }

    /* 2. Clear state and release the interrupt handling line by writing End of Interrupt */
    GICC_EOIR = ulInterruptID;
}

/*-----------------------------------------------------------*/
/* Interrupt Mask Helpers */

void vPortClearInterruptMask( UBaseType_t uxNewMaskValue )
{
    if( uxNewMaskValue == pdFALSE )
    {
        __asm volatile ( "msr daifclr, #2" ::: "memory" );
    }
}

/*-----------------------------------------------------------*/

UBaseType_t uxPortSetInterruptMask( void )
{
    uint32_t ulReturn;

    __asm volatile ( "mrs %0, daif" : "=r" ( ulReturn ) );

    /* Disable IRQ */
    __asm volatile ( "msr daifset, #2" ::: "memory" );

    return ( ( ulReturn & portDAIF_I ) != 0 );
}

void vConfigureTickInterrupt(void)
{
    /* Enable virtual counter */
    __asm volatile("msr cntv_ctl_el0, %0" :: "r"(1));

    /* Set compare value */
    uint64_t ticks = 100000; /* depends on cntfrq_el0 */
    __asm volatile("msr cntv_cval_el0, %0" :: "r"(ticks));
}


/*-----------------------------------------------------------*/

void vPortYield(void)
{
    __asm volatile("svc #0");
}