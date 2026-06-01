.section .text
.global _start
.global vector_table

.extern main
.extern FreeRTOS_SVC_Handler
.extern FreeRTOS_IRQ_Handler

/* ===================================================== */
/* ===================== Entry ========================= */
/* ===================================================== */

_start:
    /* ------------------------------------------------- */
    /* Set EL1 stack pointer                             */
    /* ------------------------------------------------- */
    ldr x0, =_stack_top
    mov sp, x0

    /* ------------------------------------------------- */
    /* Enable EL0 access to virtual counter (CNTV_EL0)  */
    /* REQUIRED for FreeRTOS tick using virtual timer   */
    /* ------------------------------------------------- */
    mrs x0, CNTKCTL_EL1
    orr x0, x0, #(1 << 0)        // EL0 access to CNTVCT_EL0
    msr CNTKCTL_EL1, x0
    isb

    /* ------------------------------------------------- */
    /* Install EL1 vector table                          */
    /* ------------------------------------------------- */
    ldr x0, =vector_table
    msr VBAR_EL1, x0
    isb

    /* ------------------------------------------------- */
    /* Enable IRQ (clear I bit in DAIF)                  */
    /* ------------------------------------------------- */
    msr DAIFClr, #2

    /* ------------------------------------------------- */
    /* Jump to C main()                                  */
    /* ------------------------------------------------- */
    bl main

1:
    wfi
    b 1b


/* ===================================================== */
/* ================= EL1 Vector Table ================== */
/* ===================================================== */

.section .vectors, "ax", %progbits
.align 11
.global vector_table
vector_table:

    /* Current EL SP_EL0 */
    b sync_handler_el1
    .space 124
    b irq_handler_el1
    .space 124
    b fiq_handler_el1
    .space 124
    b error_handler_el1
    .space 124

    /* Current EL SP_ELx */
    b sync_handler_el1
    .space 124
    b irq_handler_el1
    .space 124
    b fiq_handler_el1
    .space 124
    b error_handler_el1
    .space 124

    /* Lower EL AArch64 */
    b sync_handler_el1
    .space 124
    b irq_handler_el1
    .space 124
    b fiq_handler_el1
    .space 124
    b error_handler_el1
    .space 124

    /* Lower EL AArch32 */
    b sync_handler_el1
    .space 124
    b irq_handler_el1
    .space 124
    b fiq_handler_el1
    .space 124
    b error_handler_el1


/* ===================================================== */
/* ================= Exception Handlers ================ */
/* ===================================================== */

sync_handler_el1:
    /* SVC used by portYIELD() */
    b FreeRTOS_SVC_Handler

irq_handler_el1:
    /* Virtual timer interrupt from GICv3 (via uvmm) */
    b FreeRTOS_IRQ_Handler

fiq_handler_el1:
    b .

error_handler_el1:
    b .


/* ===================================================== */
/* ===================== Stack ========================= */
/* ===================================================== */

.section .bss
.align 12

_stack:
    .skip 0x4000          /* 16KB stack */
_stack_top: