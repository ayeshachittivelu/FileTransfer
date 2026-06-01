/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

        .globl R_OS_ASMIF_FlushICache
R_OS_ASMIF_FlushICache:
        bx      lr

        
        .globl R_OS_ASMIF_FlushDCache
R_OS_ASMIF_FlushDCache:
        push {r0 - r10}

    .if 1
    nop
    ands r2, r0, #0x1f
    beq 1f
    add r1, r2
    and r0, r0, #0xffffffe0
1:
    ands r2, r1, #0x1f
    beq 2f
    and r1, r1, #0xffffffe0
    add r1, #0x20
2:  
    mov r1, r1,  LSR #5

3:  
    cmp r1, #0
    beq 4f
    MCR     p15, 0, r0, c7, c14, 1
    add r0, #0x20
    sub r1, #0x01
    b 3b
4:
    dsb
    isb    

    .else
        /*
        The following code is based chaprter "7.7 Invalidating and cleaning cache memory"
        of the "ARM® Cortex®-R Series Version: 1.0Programmer’s Guide"

        It is disabled, because it will cause a data abort later ; to be invesitaged (TODO)
        */
        mrc     p15, 1, r0, c0, c0, 1      /* Read CLIDR */
        ands    r3, r0, #0x07000000        /* Extract coherency level */
        mov     r3, r3, lsr #23            /* Total cache levels << 1 */
        beq     R_OS_ASMIF_FlushDCache_Finished     /* If 0, no need to clean */

        mov     r10, #0                    /* R10 holds current cache level << 1 */
R_OS_ASMIF_FlushDCache_Loop1:
	add     r2, r10, r10, lsr #1       /* R2 holds cache "Set" position  */
        mov     r1, r0, lsr r2             /* Bottom 3 bits are the Cache-type for this level */
        and     r1, r1, #7                 /* Isolate those lower 3 bits */
        cmp     r1, #2
        blt     R_OS_ASMIF_FlushDCache_Skip  /* No cache or only instruction cache at this level */

        mcr     p15, 2, r10, c0, c0, 0     /* Write the Cache Size selection register */
        isb                                /* ISB to sync the change to the CacheSizeID reg */
        mrc     p15, 1, r1, c0, c0, 0      /* Reads current Cache Size ID register */
        and     r2, r1, #7                 /* Extract the line length field */
        add     r2, r2, #4                 /* Add 4 for the line length offset (log2 16 bytes) */
        ldr     r4, =0x3FF
        ands    r4, r4, r1, lsr #3         /* R4 is the max number on the way size (right aligned) */
        clz     r5, r4                     /* R5 is the bit position of the way size increment */
        ldr     r7, =0x7FFF
        ands    r7, r7, r1, lsr #13        /* R7 is the max number of the index size (right aligned) */

R_OS_ASMIF_FlushDCache_Loop2:
	mov     r9, r4                     /* R9 working copy of the max way size (right aligned) */

R_OS_ASMIF_FlushDCache_Loop3:
	orr     r11, r10, r9, lsl r5       /* Factor in the Way number and cache number into R11 */
        orr     r11, r11, r7, lsl r2       /* Factor in the Set number */
        MCR p15, 0, R11, c7, c14, 1     /* DCCIMVAC, clean & invalidate data cache by MVA to PoC */
        subs    r9, r9, #1                 /* Decrement the Way number */
        bge     R_OS_ASMIF_FlushDCache_Loop3
        subs    r7, r7, #1                 /* Decrement the Set number */
        bge     R_OS_ASMIF_FlushDCache_Loop2
R_OS_ASMIF_FlushDCache_Skip:
	add     r10, r10, #2               /* Increment the cache number */
        cmp     r3, r10 
        bgt     R_OS_ASMIF_FlushDCache_Loop1

R_OS_ASMIF_FlushDCache_Finished:
	isb
	nop
	nop


        .endif
        
        pop {r0 - r10}
        bx      lr      

        
        .globl R_OS_ASMIF_FlushAllCache
R_OS_ASMIF_FlushAllCache:
        push {lr}
        push {r0,r1}
        bl      R_OS_ASMIF_FlushICache
        push {r0,r1}
        bl      R_OS_ASMIF_FlushDCache
        pop {lr}
        bx      lr
        
        
        .globl R_OS_ASMIF_InvICache
R_OS_ASMIF_InvICache:
        MOV r0, #0
        MCR p15, 0, r0, c7, c5, 0 /* Invalidate Instruction Cache */
        MCR p15, 0, r0, c7, c5, 6 /* Invalidate branch prediction array */
        ISB                       /* Instruction Synchronization Barrier */
        bx      lr      

        
        .globl R_OS_ASMIF_InvDCache
R_OS_ASMIF_InvDCache:
        push    {r0 - r10}

    .if 1
    ands r2, r0, #0x1f
    beq 1f
    add r1, r2
    and r0, r0, #0xffffffe0
1:
    ands r2, r1, #0x1f
    beq 2f
    and r1, r1, #0xffffffe0
    add r1, #0x20
2:  
    mov r1, r1,  LSR #5

3:  
    cmp r1, #0
    beq 4f
    MCR     p15, 0, r0, c7, c6, 1
    add r0, #0x20
    sub r1, #0x01
    b 3b
4:
    dsb
    isb
    .else
    
        /*
        The following code is based chaprter "7.7 Invalidating and cleaning cache memory"
        of the "ARM® Cortex®-R Series Version: 1.0Programmer’s Guide"

        It is disabled, because it will cause a data abort later ; to be invesitaged (TODO)
        */

        MRC p15, 1, R0, c0, c0, 1       /* Read CLIDR into R0 */
        ANDS R3, R0, #0x07000000
        MOV R3, R3, LSR #23             /* Cache level value (naturally aligned) */
        BEQ Finished
        MOV R10, #0
Loop1:  
        ADD R2, R10, R10, LSR #1        /* Work out 3 x cachelevel */
        MOV R1, R0, LSR R2              /* bottom 3 bits are the Cache type for this level */
        AND R1, R1, #7                  /* get those 3 bits alone */
        CMP R1, #2
        BLT Skip                        /* no cache or only instruction cache at this level */
        MCR p15, 2, R10, c0, c0, 0      /* write CSSELR from R10 */
        ISB                             /* ISB to sync the change to the CCSIDR */
        MRC p15, 1, R1, c0, c0, 0       /* read current CCSIDR to R1 */
        AND R2, R1, #7                  /* extract the line length field */
        ADD R2, R2, #4                  /* add 4 for the line length offset (log2 16 bytes) */
        LDR R4, =0x3FF
        ANDS R4, R4, R1, LSR #3         /* R4 is the max number on the way size (right aligned) */
        CLZ R5, R4                      /* R5 is the bit position of the way size increment */
        MOV R9, R4                      /* R9 working copy of the max way size (right aligned) */
Loop2:  
        LDR R7, =0x00007FFF
        ANDS R7, R7, R1, LSR #13        /* R7 is the max num of the index size (right aligned) */
Loop3:  
        ORR R11, R10, R9, LSL R5        /* factor in the way number and cache number into R11 */
        ORR R11, R11, R7, LSL R2        /* factor in the index number */
        MCR p15, 0, R11, c7, c6, 1     /* DCIMVAC, invalidate data cache by MVA to PoC */
        //MCR p15, 0, R11, c7, c6, 2     /* DCISW, invalidate data cache by set/way */
        SUBS R7, R7, #1                 /* decrement the index */
        BGE Loop3
        SUBS R9, R9, #1                 /* decrement the way number */
        BGE Loop2
Skip:   
        ADD R10, R10, #2                /* increment the cache number */
        CMP R3, R10
        BGT Loop1
        DSB
Finished:       


        .endif
        
        pop     {r0 - r10}
        bx      lr      

        
        .globl R_OS_ASMIF_InvAllCache
R_OS_ASMIF_InvAllCache:
        push    {lr}
        push    {r0,r1}
        bl      R_OS_ASMIF_InvICache
        pop     {r0,r1}
        bl      R_OS_ASMIF_InvDCache 
        pop     {lr}
        bx      lr



        


        
        /* end of file */
