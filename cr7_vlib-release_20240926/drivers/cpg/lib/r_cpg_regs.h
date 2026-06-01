/*************************************************************************************************************
* cpg_regs
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_CPG_REGS_H_
#define R_CPG_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Clock Pulse Generator (CPG) register base Address */
#define R_CPG_BASE        (0xE6150000u)

/* CPG Register offset */
#define R_CPG_WPCR        (0x0904) /* Write protect control register */
#define R_CPG_WPR         (0x0900) /* write protect register */


/* CPG Module Standby, software Reset (MSSR) Registers offsets*/

#define R_CPG_RMSTPCR0       (0x0110)  /* Realtime module stop control register 0 */
#define R_CPG_RMSTPCR1       (0x0114)  /* Realtime module stop control register 1 */
#define R_CPG_RMSTPCR2       (0x0118)  /* Realtime module stop control register 2 */
#define R_CPG_RMSTPCR3       (0x011C)  /* Realtime module stop control register 3 */
#define R_CPG_RMSTPCR4       (0x0120)  /* Realtime module stop control register 4 */
#define R_CPG_RMSTPCR5       (0x0124)  /* Realtime module stop control register 5 */
#define R_CPG_RMSTPCR6       (0x0128)  /* Realtime module stop control register 6 */
#define R_CPG_RMSTPCR7       (0x012C)  /* Realtime module stop control register 7 */
#define R_CPG_RMSTPCR8       (0x0980)  /* Realtime module stop control register 8 */
#define R_CPG_RMSTPCR9       (0x0984)  /* Realtime module stop control register 9 */
#define R_CPG_RMSTPCR10      (0x0988)  /* Realtime module stop control register 10 */
#define R_CPG_RMSTPCR11      (0x098C)  /* Realtime module stop control register 11 */

/* Added DHD workaround */
#define	R_CPG_SAMSTPCR0      (0x0C20) /* Safety Module Stop Control Register 0 */
#define	R_CPG_SAMSTPCR1      (0x0C24) /* Safety Module Stop Control Register 1 */
#define	R_CPG_SAMSTPCR2      (0x0C28) /* Safety Module Stop Control Register 2 */
#define	R_CPG_SAMSTPCR3      (0x0C2C) /* Safety Module Stop Control Register 3 */
#define	R_CPG_SAMSTPCR4      (0x0C30) /* Safety Module Stop Control Register 4 */
#define	R_CPG_SAMSTPCR5      (0x0C34) /* Safety Module Stop Control Register 5 */
#define	R_CPG_SAMSTPCR6      (0x0C38) /* Safety Module Stop Control Register 6 */
#define	R_CPG_SAMSTPCR7      (0x0C3c) /* Safety Module Stop Control Register 7 */
#define	R_CPG_SAMSTPCR8      (0x0C40) /* Safety Module Stop Control Register 8 */
#define	R_CPG_SAMSTPCR9      (0x0C44) /* Safety Module Stop Control Register 9 */
#define	R_CPG_SAMSTPCR10     (0x0C48) /* Safety Module Stop Control Register 10 */
#define	R_CPG_SAMSTPCR11     (0x0C4c) /* Safety Module Stop Control Register 11 */

#define R_CPG_SRCR0          (0x00A0)  /* Software reset register 0 */
#define R_CPG_SRCR1          (0x00A8)  /* Software reset register 1 */
#define R_CPG_SRCR2          (0x00B0)  /* Software reset register 2 */
#define R_CPG_SRCR3          (0x00B8)  /* Software reset register 3 */
#define R_CPG_SRCR4          (0x00BC)  /* Software reset register 4 */
#define R_CPG_SRCR5          (0x00C4)  /* Software reset register 5 */
#define R_CPG_SRCR6          (0x01C8)  /* Software reset register 6 */
#define R_CPG_SRCR7          (0x01CC)  /* Software reset register 7 */
#define R_CPG_SRCR8          (0x0920)  /* Software reset register 8 */
#define R_CPG_SRCR9          (0x0924)  /* Software reset register 9 */
#define R_CPG_SRCR10         (0x0928)  /* Software reset register 10 */
#define R_CPG_SRCR11         (0x092C)  /* Software reset register 11 */

#define R_CPG_SRSTCLR0       (0x0940)  /* Software reset clear register 0 */
#define R_CPG_SRSTCLR1       (0x0944)  /* Software reset clear register 1 */
#define R_CPG_SRSTCLR2       (0x0948)  /* Software reset clear register 2 */
#define R_CPG_SRSTCLR3       (0x094C)  /* Software reset clear register 3 */
#define R_CPG_SRSTCLR4       (0x0950)  /* Software reset clear register 4 */
#define R_CPG_SRSTCLR5       (0x0954)  /* Software reset clear register 5 */
#define R_CPG_SRSTCLR6       (0x0958)  /* Software reset clear register 6 */
#define R_CPG_SRSTCLR7       (0x095C)  /* Software reset clear register 7 */
#define R_CPG_SRSTCLR8       (0x0960)  /* Software reset clear register 8 */
#define R_CPG_SRSTCLR9       (0x0964)  /* Software reset clear register 9 */
#define R_CPG_SRSTCLR10      (0x0968)  /* Software reset clear register 10 */
#define R_CPG_SRSTCLR11      (0x096C)  /* Software reset clear register 11 */

#define R_CPG_MSTPSR0       (0x0030)  /* Module Stop Status Register 0 */
#define R_CPG_MSTPSR1       (0x0038)  /* Module Stop Status Register 1 */
#define R_CPG_MSTPSR2       (0x0040)  /* Module Stop Status Register 2 */
#define R_CPG_MSTPSR3       (0x0048)  /* Module Stop Status Register 3 */
#define R_CPG_MSTPSR4       (0x004C)  /* Module Stop Status Register 4 */
#define R_CPG_MSTPSR5       (0x003C)  /* Module Stop Status Register 5 */
#define R_CPG_MSTPSR6       (0x01C0)  /* Module Stop Status Register 6 */
#define R_CPG_MSTPSR7       (0x01C4)  /* Module Stop Status Register 7 */
#define R_CPG_MSTPSR8       (0x09A0)  /* Module Stop Status Register 8 */
#define R_CPG_MSTPSR9       (0x09A4)  /* Module Stop Status Register 9 */
#define R_CPG_MSTPSR10      (0x09A8)  /* Module Stop Status Register 10*/
#define R_CPG_MSTPSR11      (0x09AC)  /* Module Stop Status Register 11*/

#ifdef __cplusplus
}
#endif

#endif /* R_CPG_REGS_H_ */
