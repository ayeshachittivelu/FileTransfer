.text
.align 11

.extern pxCurrentTCB
.extern vTaskSwitchContext
.extern ullCriticalNesting
.extern ullPortInterruptNesting
.extern ullPortYieldRequired
.extern _freertos_vector_table

.global FreeRTOS_SVC_Handler
.global FreeRTOS_IRQ_Handler
.global FreeRTOS_FIQ_Handler
.global FreeRTOS_Error_Handler
.global vPortRestoreTaskContext

/* -----------------------------------------------------------
 * SAVE FULL TASK CONTEXT
 * ----------------------------------------------------------*/
.macro portSAVE_CONTEXT

    STP  X0,  X1,  [SP, #-16]!
    STP  X2,  X3,  [SP, #-16]!
    STP  X4,  X5,  [SP, #-16]!
    STP  X6,  X7,  [SP, #-16]!
    STP  X8,  X9,  [SP, #-16]!
    STP  X10, X11, [SP, #-16]!
    STP  X12, X13, [SP, #-16]!
    STP  X14, X15, [SP, #-16]!
    STP  X16, X17, [SP, #-16]!
    STP  X18, X19, [SP, #-16]!
    STP  X20, X21, [SP, #-16]!
    STP  X22, X23, [SP, #-16]!
    STP  X24, X25, [SP, #-16]!
    STP  X26, X27, [SP, #-16]!
    STP  X28, X29, [SP, #-16]!
    STP  X30, XZR, [SP, #-16]!

    MRS  X2, ELR_EL1
    MRS  X3, SPSR_EL1
    STP  X2, X3, [SP, #-16]!

    LDR  X0, =ullCriticalNesting
    LDR  X3, [X0]
    STP  XZR, X3, [SP, #-16]!      /* <-- NOTE the ! here */

    LDR  X0, =pxCurrentTCB
    LDR  X1, [X0]
    MOV  X0, SP
    STR  X0, [X1]

.endm

/* -----------------------------------------------------------
 * RESTORE TASK CONTEXT
 * ----------------------------------------------------------*/
.macro portRESTORE_CONTEXT


    LDR X0, =pxCurrentTCB
    LDR X1, [X0]
    LDR X0, [X1]
    MOV SP, X0

    LDP XZR, X3, [SP], #16

    LDR X0, =ullCriticalNesting
    STR X3, [X0]

    LDP X2, X3, [SP], #16
    MSR ELR_EL1, X2
    MSR SPSR_EL1, X3

    LDP X30, XZR, [SP], #16
    LDP X28, X29, [SP], #16
    LDP X26, X27, [SP], #16
    LDP X24, X25, [SP], #16
    LDP X22, X23, [SP], #16
    LDP X20, X21, [SP], #16
    LDP X18, X19, [SP], #16
    LDP X16, X17, [SP], #16
    LDP X14, X15, [SP], #16
    LDP X12, X13, [SP], #16
    LDP X10, X11, [SP], #16
    LDP X8,  X9,  [SP], #16
    LDP X6,  X7,  [SP], #16
    LDP X4,  X5,  [SP], #16
    LDP X2,  X3,  [SP], #16
    LDP X0,  X1,  [SP], #16


    DSB SY
    ISB
    ERET

.endm

/* -----------------------------------------------------------
 * SVC Handler
 * ----------------------------------------------------------*/
FreeRTOS_SVC_Handler:
    portSAVE_CONTEXT
    BL vTaskSwitchContext
    portRESTORE_CONTEXT

FreeRTOS_FIQ_Handler:   
    eret

FreeRTOS_Error_Handler:   
    eret
/* -----------------------------------------------------------
 * IRQ Handler (Virtual Timer)
 * ----------------------------------------------------------*/
FreeRTOS_IRQ_Handler:
    /* 1. MUST save context BEFORE modifying ANY registers! */
    portSAVE_CONTEXT

    /* 2. Call the C handler. It will read the GIC, reload the timer, and write EOIR. */
    BL FreeRTOS_Tick_Handler

    /* 3. Check if a context switch is required */
    LDR x0, =ullPortYieldRequired
    LDR x1, [x0]
    CBZ x1, no_switch

    MOV x1, #0
    STR x1, [x0]
    BL vTaskSwitchContext

no_switch:
    portRESTORE_CONTEXT

/* -----------------------------------------------------------
 * Scheduler Start
 * ----------------------------------------------------------*/
vPortRestoreTaskContext:
    portRESTORE_CONTEXT