/*************************************************************************************************************
* cio_canfd_main_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup canfdmain1_canfd CAN-FD Driver main
 * @ingroup cio_canfd_driver
 *
 * @brief CIO CAN-FD Driver main module
 *
 * Typedefs, macros and functions that are accesed by other CAN-FD driver modules.
 * The main module also contains error and interrupt handling functions.
 *
 * @{
 */



#ifndef R_CIO_CANFD_MAIN_H
#define R_CIO_CANFD_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "r_cio_can.h"

/*******************************************************************************
   Section: CANFD driver internal defines
 */

/**@defgroup canfdmain2 CAN-FD Register access
 *
 * @brief General bit and register access macros
 *
 * @{
 */

/**
 * @def LOC_BIT
 * @brief  local bit access helper
 *
 * Provides a mask value corresponding to the given bit number
 *
 * @param[in] nr      - bit number
 *
 * @return mask value for the given bit
 *
 */
#define LOC_BIT(nr)         (1UL << (nr))

/**
 * @def R_CIO_CANFD_WRITE8
 * @brief  Write 8 Bit value to a CAN-FD register
 *
 * @param[in] ADDR     -register address
 * @param[in] VAL      -value
 *
 * @return void
 *
 */
#define R_CIO_CANFD_WRITE8(ADDR, VAL) *((volatile uint8_t *)((size_t)ADDR)) = (VAL)

/**
 * @def R_CIO_CANFD_READ8
 * @brief  Read 8 Bit value from a CAN-FD register
 *
 * @param[in] ADDR     -register address
 *
 * @return register value
 *
 */
#define R_CIO_CANFD_READ8(ADDR) *((volatile uint8_t *)((size_t)ADDR))

/**
 * @def R_CIO_CANFD_WRITE16
 * @brief  write 16 Bit value to a CAN-FD register
 *
 * @param[in] ADDR     -register address
 * @param[in] VAL      -value
 *
 * @return void
 *
 */
#define R_CIO_CANFD_WRITE16(ADDR, VAL) *((volatile uint16_t *)((size_t)ADDR)) = (VAL)

/**
 * @def R_CIO_CANFD_READ16
 * @brief  Read 16 Bit value from a CAN-FD register
 *
 * @param[in] ADDR     -register address
 *
 * @return register value
 *
 */
#define R_CIO_CANFD_READ16(ADDR) *((volatile uint16_t *)((size_t)ADDR))

/**
 * @def R_CIO_CANFD_WRITE32
 * @brief  write 32 Bit value to a CAN-FD register
 *
 * @param[in] ADDR     -register address
 * @param[in] VAL      -value
 *
 * @return void
 *
 */
#define R_CIO_CANFD_WRITE32(ADDR, VAL) *((volatile uint32_t *)((size_t)ADDR)) = (VAL)

/**
 * @def R_CIO_CANFD_READ32
 * @brief  Read 32 Bit value from a CAN-FD register
 *
 * @param[in] ADDR     -register address
 *
 * @return register value
 *
 */
#define R_CIO_CANFD_READ32(ADDR) *((volatile uint32_t *)((size_t)ADDR))

/** @} */

/**@defgroup canfdmain3 CAN-FD Registers
 *
 * @brief Defines and helpers to get the correct register addres for a
 * given instance and channel
 *
 * The names do not fully correspond to the UM, but the correponding names
 * are supplied in comments. These can be used to search for register details the UM.
 *
 * This controller supports either Classical CAN only mode or CAN-FD only mode.
 * These modes are supported in two separate set of register maps & names.
 * However, some of the register offsets are common for both modes. Those
 * offsets are listed below as Common registers.
 * The CAN-FD only mode specific registers & Classical CAN only mode specific
 * registers are listed separately. Their register names starts with
 * R_CIO_CANFD_F_xxx & R_CIO_CANFD_C_xxx respectively.

 * @{
 */

/* Common registers */
#define R_CIO_CANFD_BASE_ADDR        (0xe66C0000)

#define R_CIO_CANFD_CCFG(m)          (0x0000 + (0x10 * (m)))                          /**< RSCFDnCFDCmNCFG / RSCFDnCmCFG */
#define R_CIO_CANFD_CCTR(m)          (0x0004 + (0x10 * (m)))                          /**< RSCFDnCFDCmCTR / RSCFDnCmCTR */
#define R_CIO_CANFD_CSTS(m)          (0x0008 + (0x10 * (m)))                          /**< RSCFDnCFDCmSTS / RSCFDnCmSTS */
#define R_CIO_CANFD_CERFL(m)         (0x000C + (0x10 * (m)))                          /**< RSCFDnCFDCmERFL / RSCFDnCmERFL */
#define R_CIO_CANFD_GCFG                (0x0084)                                      /**< RSCFDnCFDGCFG / RSCFDnGCFG */
#define R_CIO_CANFD_GCTR                (0x0088)                                      /**< RSCFDnCFDGCTR / RSCFDnGCTR */
#define R_CIO_CANFD_GSTS                (0x008c)                                      /**< RSCFDnCFDGCTS / RSCFDnGCTS */
#define R_CIO_CANFD_GERFL               (0x0090)                                      /**< RSCFDnCFDGERFL / RSCFDnGERFL */
#define R_CIO_CANFD_GTSC                (0x0094)                                      /**< RSCFDnCFDGTSC / RSCFDnGTSC */
#define R_CIO_CANFD_GAFLECTR            (0x0098)                                      /**< RSCFDnCFDGAFLECTR / RSCFDnGAFLECTR */
#define R_CIO_CANFD_GAFLCFG0            (0x009c)                                      /**< RSCFDnCFDGAFLCFG0 / RSCFDnGAFLCFG0 */
#define R_CIO_CANFD_GAFLCFG1            (0x00a0)                                      /**< RSCFDnCFDGAFLCFG1 / RSCFDnGAFLCFG1 */
#define R_CIO_CANFD_RMNB                (0x00a4)                                      /**< RSCFDnCFDRMNB / RSCFDnRMNB */
#define R_CIO_CANFD_RMND(y)             (0x00a8 + (0x04 * (y)))                       /**< RSCFDnCFDRMND / RSCFDnRMND */
#define R_CIO_CANFD_RFCC(x)             (0x00b8 + (0x04 * (x)))                       /**< RSCFDnCFDRFCCx / RSCFDnRFCCx */
#define R_CIO_CANFD_RFSTS(x)            (0x00d8 + (0x04 * (x)))                       /**< RSCFDnCFDRFSTSx / RSCFDnRFSTSx */
#define R_CIO_CANFD_RFPCTR(x)           (0x00f8 + (0x04 * (x)))                       /**< RSCFDnCFDRFPCTRx / RSCFDnRFPCTRx */
#define R_CIO_CANFD_CFCC(ch, idx)       (0x0118 + (0x0c * (ch)) + \
                                         (0x04 * (idx)))                              /**< RSCFDnCFDCFCCx / RSCFDnCFCCx */
#define R_CIO_CANFD_CFSTS(ch, idx)      (0x0178 + (0x0c * (ch)) + \
                                         (0x04 * (idx)))                              /**< RSCFDnCFDCFSTSx / RSCFDnCFSTSx */
#define R_CIO_CANFD_CFPCTR(ch, idx)     (0x01d8 + (0x0c * (ch)) + \
                                         (0x04 * (idx)))                              /**< RSCFDnCFDCFPCTRx / RSCFDnCFPCTRx */
#define R_CIO_CANFD_FESTS               (0x0238)                                      /**< RSCFDnCFDFESTS / RSCFDnFESTS */
#define R_CIO_CANFD_FFSTS               (0x023c)                                      /**< RSCFDnCFDFFSTS / RSCFDnFFSTS */
#define R_CIO_CANFD_FMSTS               (0x0240)                                      /**< RSCFDnCFDFMSTS / RSCFDnFMSTS */
#define R_CIO_CANFD_RFISTS              (0x0244)                                      /**< RSCFDnCFDRFISTS / RSCFDnRFISTS */
#define R_CIO_CANFD_CFRISTS             (0x0248)                                      /**< RSCFDnCFDCFRISTS / RSCFDnCFRISTS */
#define R_CIO_CANFD_CFTISTS             (0x024c)                                      /**< RSCFDnCFDCFTISTS / RSCFDnCFTISTS */
#define R_CIO_CANFD_TMC(p)              (0x0250 + (0x01 * (p)))                       /**< RSCFDnCFDTMCp / RSCFDnTMCp */
#define R_CIO_CANFD_TMSTS(p)            (0x02d0 + (0x01 * (p)))                       /**< RSCFDnCFDTMSTSp / RSCFDnTMSTSp */
#define R_CIO_CANFD_TMTRSTS(y)          (0x0350 + (0x04 * (y)))                       /**< RSCFDnCFDTMTRSTSp / RSCFDnTMTRSTSp */
#define R_CIO_CANFD_TMTARSTS(y)         (0x0360 + (0x04 * (y)))                       /**< RSCFDnCFDTMTARSTSp / RSCFDnTMTARSTSp */
#define R_CIO_CANFD_TMTCSTS(y)          (0x0370 + (0x04 * (y)))                       /**< RSCFDnCFDTMTCSTSp / RSCFDnTMTCSTSp */
#define R_CIO_CANFD_TMTASTS(y)          (0x0380 + (0x04 * (y)))                       /**< RSCFDnCFDTMTASTSp / RSCFDnTMTASTSp */
#define R_CIO_CANFD_TMIEC(y)            (0x0390 + (0x04 * (y)))                       /**< RSCFDnCFDTMIECy / RSCFDnTMIECy */
#define R_CIO_CANFD_TXQCC(m)            (0x03a0 + (0x04 * (m)))                       /**< RSCFDnCFDTXQCCm / RSCFDnTXQCCm */
#define R_CIO_CANFD_TXQSTS(m)           (0x03c0 + (0x04 * (m)))                       /**< RSCFDnCFDTXQSTSm / RSCFDnTXQSTSm */
#define R_CIO_CANFD_TXQPCTR(m)          (0x03e0 + (0x04 * (m)))                       /**< RSCFDnCFDTXQPCTRm / RSCFDnTXQPCTRm */
#define R_CIO_CANFD_THLCC(m)            (0x0400 + (0x04 * (m)))                       /**< RSCFDnCFDTHLCCm / RSCFDnTHLCCm */
#define R_CIO_CANFD_THLSTS(m)           (0x0420 + (0x04 * (m)))                       /**< RSCFDnCFDTHLSTSm / RSCFDnTHLSTSm */
#define R_CIO_CANFD_THLPCTR(m)          (0x0440 + (0x04 * (m)))                       /**< RSCFDnCFDTHLPCTRm / RSCFDnTHLPCTRm */
#define R_CIO_CANFD_GTINTSTS0           (0x0460)                                      /**< RSCFDnCFDGTINTSTS0 / RSCFDnGTINTSTS0 */
#define R_CIO_CANFD_GTINTSTS1           (0x0464)                                      /**< RSCFDnCFDGTINTSTS1 / RSCFDnGTINTSTS1 */
#define R_CIO_CANFD_GTSTCFG             (0x0468)                                      /**< RSCFDnCFDGTSTCFG / RSCFDnGTSTCFG */
#define R_CIO_CANFD_GTSTCTR             (0x046c)                                      /**< RSCFDnCFDGTSTCTR / RSCFDnGTSTCTR */
#define R_CIO_CANFD_GLOCKK              (0x047c)                                      /**< RSCFDnCFDGLOCKK / RSCFDnGLOCKK */
#define R_CIO_CANFD_GRMCFG              (0x04fc)                                      /**< RSCFDnCFDGRMCFG */
#define R_CIO_CANFD_GAFLID(offset, j)   ((offset) + (0x10 * (j)))                     /**< RSCFDnCFDGAFLIDj / RSCFDnGAFLIDj */
#define R_CIO_CANFD_GAFLM(offset, j)    ((offset) + 0x04 + (0x10 * (j)))              /**< RSCFDnCFDGAFLMj / RSCFDnGAFLMj */
#define R_CIO_CANFD_GAFLP0(offset, j)   ((offset) + 0x08 + (0x10 * (j)))              /**< RSCFDnCFDGAFLP0j / RSCFDnGAFLP0j */
#define R_CIO_CANFD_GAFLP1(offset, j)   ((offset) + 0x0c + (0x10 * (j)))              /**< RSCFDnCFDGAFLP1j / RSCFDnGAFLP1j */

/* Classical CAN only mode register map */
#define R_CIO_CANFD_C_GAFL_OFFSET       (0x0500)                                      /**< RSCFDnGAFLXXXj offset */
#define R_CIO_CANFD_C_RMID(q)           (0x0600 + (0x10 * (q)))                       /**< RSCFDnRMIDq */
#define R_CIO_CANFD_C_RMPTR(q)          (0x0604 + (0x10 * (q)))                       /**< RSCFDnRMPTRq */
#define R_CIO_CANFD_C_RMDF0(q)          (0x0608 + (0x10 * (q)))                       /**< RSCFDnRMDF0q */
#define R_CIO_CANFD_C_RMDF1(q)          (0x060c + (0x10 * (q)))                       /**< RSCFDnRMDF1q */
#define R_CIO_CANFD_C_RFOFFSET          (0x0e00)                                      /**< RSCFDnRF */
#define R_CIO_CANFD_C_RFID(x)           (R_CIO_CANFD_C_RFOFFSET + (0x10 * (x)))       /**< RSCFDnRFIDx*/
#define R_CIO_CANFD_C_RFPTR(x)          (R_CIO_CANFD_C_RFOFFSET + 0x04 + \
                                         (0x10 * (x)))                                /**< RSCFDnRFPTRx */
#define R_CIO_CANFD_C_RFDF(x, df)       (R_CIO_CANFD_C_RFOFFSET + 0x08 + \
                                         (0x10 * (x)) + (0x04 * (df)))                /**< RSCFDnRFDFx */
#define R_CIO_CANFD_C_CFOFFSET          (0x0e80)
#define R_CIO_CANFD_C_CFID(ch, idx)     (R_CIO_CANFD_C_CFOFFSET + (0x30 * (ch)) + \
                                         (0x10 * (idx)))                              /**< RSCFDnCFIDx */
#define R_CIO_CANFD_C_CFPTR(ch, idx)    (R_CIO_CANFD_C_CFOFFSET + 0x04 + \
                                         (0x30 * (ch)) + (0x10 * (idx)))              /**< RSCFDnCFPTR x*/
#define R_CIO_CANFD_C_CFDF(ch, idx, df) (R_CIO_CANFD_C_CFOFFSET + 0x08 + \
                                         (0x30 * (ch)) + (0x10 * (idx)) + \
                                         (0x04 * (df)))                               /**< RSCFDnCFDFx */
#define R_CIO_CANFD_C_TMID(p)           (0x1000 + (0x10 * (p)))                       /**< RSCFDnTMIDp  */
#define R_CIO_CANFD_C_TMPTR(p)          (0x1004 + (0x10 * (p)))                       /**< RSCFDnTMPTRp */
#define R_CIO_CANFD_C_TMDF0(p)          (0x1008 + (0x10 * (p)))                       /**< RSCFDnTMDF0p */
#define R_CIO_CANFD_C_TMDF1(p)          (0x100c + (0x10 * (p)))                       /**< RSCFDnTMDF1p */
#define R_CIO_CANFD_C_THLACC(m)         (0x1800 + (0x04 * (m)))                       /**< RSCFDnTHLACCm */
#define R_CIO_CANFD_C_RPGACC(r)         (0x1900 + (0x04 * (r)))                       /**< RSCFDnRPGACCr  */

/* CAN-FD mode specific register map */
#define R_CIO_CANFD_F_DCFG(m)           (0x0500 + (0x20 * (m)))                       /**< RSCFDnCFDCm    */
#define R_CIO_CANFD_F_CFDCFG(m)         (0x0504 + (0x20 * (m)))                       /**< RSCFDnCFDCFGCm */
#define R_CIO_CANFD_F_CFDCTR(m)         (0x0508 + (0x20 * (m)))                       /**< RSCFDnCFDCTRm */
#define R_CIO_CANFD_F_CFDSTS(m)         (0x050c + (0x20 * (m)))                       /**< RSCFDnCFDSTSm */
#define R_CIO_CANFD_F_CFDCRC(m)         (0x0510 + (0x20 * (m)))                       /**< RSCFDnCFDCRCm */
#define R_CIO_CANFD_F_GAFL_OFFSET       (0x1000)                                      /**< RSCFDnCFDGAFL  */
#define R_CIO_CANFD_F_RMID(q)           (0x2000 + (0x20 * (q)))                       /**< RSCFDnCFDRMIDq */
#define R_CIO_CANFD_F_RMPTR(q)          (0x2004 + (0x20 * (q)))                       /**< RSCFDnCFDRMPTRq  */
#define R_CIO_CANFD_F_RMFDSTS(q)        (0x2008 + (0x20 * (q)))                       /**< RSCFDnCFDRMFDSTq  */
#define R_CIO_CANFD_F_RMDF(q, b)        (0x200c + (0x04 * (b)) + (0x20 * (q)))        /**< RSCFDnCFDRMDFq  */
#define R_CIO_CANFD_F_RFOFFSET          (0x3000)                                      /**< RSCFDnCFDRF  */
#define R_CIO_CANFD_F_RFID(x)           (R_CIO_CANFD_F_RFOFFSET + (0x80 * (x)))       /**< RSCFDnCFDRFIDx */
#define R_CIO_CANFD_F_RFPTR(x)          (R_CIO_CANFD_F_RFOFFSET + 0x04 + \
                                         (0x80 * (x)))                                /**< RSCFDnCFDRFPTRx */
#define R_CIO_CANFD_F_RFFDSTS(x)        (R_CIO_CANFD_F_RFOFFSET + 0x08 + \
                                         (0x80 * (x)))                                /**< RSCFDnCFDRFSTSx */
#define R_CIO_CANFD_F_RFDF(x, df)       (R_CIO_CANFD_F_RFOFFSET + 0x0c + \
                                         (0x80 * (x)) + (0x04 * (df)))                /**< RSCFDnCFDRFDFx  */
#define R_CIO_CANFD_F_CFOFFSET          (0x3400)                                      /**< RSCFDnCFDCF  */
#define R_CIO_CANFD_F_CFID(ch, idx)     (R_CIO_CANFD_F_CFOFFSET + (0x180 * (ch)) + \
                                         (0x80 * (idx)))                              /**< RSCFDnCFDCFIDx */
#define R_CIO_CANFD_F_CFPTR(ch, idx)    (R_CIO_CANFD_F_CFOFFSET + 0x04 + \
                                         (0x180 * (ch)) + (0x80 * (idx)))             /**< RSCFDnCFDCFPTRx */
#define R_CIO_CANFD_F_CFFDCSTS(ch, idx) (R_CIO_CANFD_F_CFOFFSET + 0x08 + \
                                         (0x180 * (ch)) + (0x80 * (idx)))             /**< RSCFDnCFDCFFDCSTSk */
#define R_CIO_CANFD_F_CFDF(ch, idx, df) (R_CIO_CANFD_F_CFOFFSET + 0x0c + \
                                         (0x180 * (ch)) + (0x80 * (idx)) + \
                                         (0x04 * (df)))                               /**< RSCFDnCFDCFDFx  */
#define R_CIO_CANFD_F_TMPTR(p)          (0x4004 + (0x20 * (p)))                       /**< RSCFDnCFDTMXXp */
#define R_CIO_CANFD_F_TMFDCTR(p)        (0x4008 + (0x20 * (p)))                       /**< RSCFDnCFDTMFDCTRp */
#define R_CIO_CANFD_F_TMDF(p, b)        (0x400c + (0x20 * (p)) + (0x04 * (b)))        /**< RSCFDnCFDTMDFp  */
#define R_CIO_CANFD_F_THLACC(m)         (0x6000 + (0x04 * (m)))                       /**< RSCFDnCFDTHLACCm */
#define R_CIO_CANFD_F_RPGACC(r)         (0x6400 + (0x04 * (r)))                       /**< RSCFDnCFDRPGACCr */
/** @} */


/**@defgroup canfdmain4 CAN-FD Regsister Bits
 *
 * @brief Macro defines for accessing selected bits of CAN-FD registers.
 *
 * The names in the comments correspond to the UM.
 * Please look there for detailed description.
 *
 * @{
 */

/* RSCFDnCFDGRMCFG */
#define R_CIO_CANFD_GRMCFG_RCMC      LOC_BIT(0)

/* RSCFDnCFDGCFG / RSCFDnGCFG */
#define R_CIO_CANFD_GCFG_EEFE        LOC_BIT(6)
#define R_CIO_CANFD_GCFG_CMPOC       LOC_BIT(5)  /*  CAN-FD only */
#define R_CIO_CANFD_GCFG_DCS         LOC_BIT(4)
#define R_CIO_CANFD_GCFG_DCE         LOC_BIT(1)
#define R_CIO_CANFD_GCFG_TPRI        LOC_BIT(0)
/* RSCFDnCFDGCTR / RSCFDnGCTR */
#define R_CIO_CANFD_GCTR_TSRST       LOC_BIT(16)
#define R_CIO_CANFD_GCTR_CFMPOFIE    LOC_BIT(11)    /* CAN-FD only */
#define R_CIO_CANFD_GCTR_THLEIE      LOC_BIT(10)
#define R_CIO_CANFD_GCTR_MEIE        LOC_BIT(9)
#define R_CIO_CANFD_GCTR_DEIE        LOC_BIT(8)
#define R_CIO_CANFD_GCTR_GSLPR       LOC_BIT(2)
#define R_CIO_CANFD_GCTR_GMDC_MASK   (0x3)
#define R_CIO_CANFD_GCTR_GMDC_GOPM   (0x0)
#define R_CIO_CANFD_GCTR_GMDC_GRESET (0x1)
#define R_CIO_CANFD_GCTR_GMDC_GTEST  (0x2)

/* RSCFDnCFDGSTS / RSCFDnGSTS */
#define R_CIO_CANFD_GSTS_GRAMINIT    LOC_BIT(3)
#define R_CIO_CANFD_GSTS_GSLPSTS     LOC_BIT(2)
#define R_CIO_CANFD_GSTS_GHLTSTS     LOC_BIT(1)
#define R_CIO_CANFD_GSTS_GRSTSTS     LOC_BIT(0)

/* Non-operational status */
#define R_CIO_CANFD_GSTS_GNOPM       (LOC_BIT(0) | LOC_BIT(1) | LOC_BIT(2) | LOC_BIT(3))

/* RSCFDnCFDGERFL / RSCFDnGERFL */
#define R_CIO_CANFD_GERFL_EEF1       LOC_BIT(17)
#define R_CIO_CANFD_GERFL_EEF0       LOC_BIT(16)
#define R_CIO_CANFD_GERFL_CMPOF      LOC_BIT(3)  /* CAN-FD only */
#define R_CIO_CANFD_GERFL_THLES      LOC_BIT(2)
#define R_CIO_CANFD_GERFL_MES        LOC_BIT(1)
#define R_CIO_CANFD_GERFL_DEF        LOC_BIT(0)

#define R_CIO_CANFD_GERFL_ERR(inst, x)  ((x) & (R_CIO_CANFD_GERFL_EEF1 | \
                                                R_CIO_CANFD_GERFL_EEF0 | R_CIO_CANFD_GERFL_MES | \
                                                (inst->FdMode ? \
                                                 R_CIO_CANFD_GERFL_CMPOF : 0)))

/* AFL Rx rules register bits */

/* RSCFDnCFDGAFLCFG0 / RSCFDnGAFLCFG0 */
#define R_CIO_CANFD_GAFLCFG_SETRNC(n, x) (((x) & 0xff) << (24 - n * 8))
#define R_CIO_CANFD_GAFLCFG_GETRNC(n, x) (((x) >> (24 - n * 8)) & 0xff)

/* RSCFDnCFDGAFLECTR / RSCFDnGAFLECTR */
#define R_CIO_CANFD_GAFLECTR_AFLDAE      LOC_BIT(8)
#define R_CIO_CANFD_GAFLECTR_AFLPN(x)    ((x) & 0x1f)

/* RSCFDnCFDGAFLIDj / RSCFDnGAFLIDj */
#define R_CIO_CANFD_GAFLID_GAFLLB        LOC_BIT(29)

/* RSCFDnCFDGAFLP1_j / RSCFDnGAFLP1_j */
#define R_CIO_CANFD_GAFLP1_GAFLFDP(x)    (1 << (x))

/* Channel register bits */

/* RSCFDnCmCFG - Classical CAN only */
#define R_CIO_CANFD_CFG_SJW(x)       (((x) & 0x3) << 24)
#define R_CIO_CANFD_CFG_TSEG2(x)     (((x) & 0x7) << 20)
#define R_CIO_CANFD_CFG_TSEG1(x)     (((x) & 0xf) << 16)
#define R_CIO_CANFD_CFG_BRP(x)       (((x) & 0x3ff) << 0)

/* RSCFDnCFDCmNCFG - CAN-FD only */
#define R_CIO_CANFD_NCFG_NTSEG2(x)   (((x) & 0x1f) << 24)
#define R_CIO_CANFD_NCFG_NTSEG1(x)   (((x) & 0x7f) << 16)
#define R_CIO_CANFD_NCFG_NSJW(x)     (((x) & 0x1f) << 11)
#define R_CIO_CANFD_NCFG_NBRP(x)     (((x) & 0x3ff) << 0)

/* RSCFDnCFDCmCTR / RSCFDnCmCTR */
#define R_CIO_CANFD_CCTR_CTME         LOC_BIT(24)
#define R_CIO_CANFD_CCTR_ERRD         LOC_BIT(23)
#define R_CIO_CANFD_CCTR_BOM_MASK     (0x3 << 21)
#define R_CIO_CANFD_CCTR_BOM_ISO      (0x0 << 21)
#define R_CIO_CANFD_CCTR_BOM_BENTRY   (0x1 << 21)
#define R_CIO_CANFD_CCTR_BOM_BEND     (0x2 << 21)
#define R_CIO_CANFD_CCTR_TDCVFIE      LOC_BIT(19)
#define R_CIO_CANFD_CCTR_SOCOIE       LOC_BIT(18)
#define R_CIO_CANFD_CCTR_EOCOIE       LOC_BIT(17)
#define R_CIO_CANFD_CCTR_TAIE         LOC_BIT(16)
#define R_CIO_CANFD_CCTR_ALIE         LOC_BIT(15)
#define R_CIO_CANFD_CCTR_BLIE         LOC_BIT(14)
#define R_CIO_CANFD_CCTR_OLIE         LOC_BIT(13)
#define R_CIO_CANFD_CCTR_BORIE        LOC_BIT(12)
#define R_CIO_CANFD_CCTR_BOEIE        LOC_BIT(11)
#define R_CIO_CANFD_CCTR_EPIE         LOC_BIT(10)
#define R_CIO_CANFD_CCTR_EWIE         LOC_BIT(9)
#define R_CIO_CANFD_CCTR_BEIE         LOC_BIT(8)
#define R_CIO_CANFD_CCTR_CSLPR        LOC_BIT(2)
#define R_CIO_CANFD_CCTR_CHMDC_MASK   (0x3)
#define R_CIO_CANFD_CCTR_CHDMC_COPM   (0x0)
#define R_CIO_CANFD_CCTR_CHDMC_CRESET (0x1)
#define R_CIO_CANFD_CCTR_CHDMC_CHLT   (0x2)

/* RSCFDnCFDCmSTS / RSCFDnCmSTS */
#define R_CIO_CANFD_CSTS_COMSTS       LOC_BIT(7)
#define R_CIO_CANFD_CSTS_RECSTS       LOC_BIT(6)
#define R_CIO_CANFD_CSTS_TRMSTS       LOC_BIT(5)
#define R_CIO_CANFD_CSTS_BOSTS        LOC_BIT(4)
#define R_CIO_CANFD_CSTS_EPSTS        LOC_BIT(3)
#define R_CIO_CANFD_CSTS_SLPSTS       LOC_BIT(2)
#define R_CIO_CANFD_CSTS_HLTSTS       LOC_BIT(1)
#define R_CIO_CANFD_CSTS_CRSTSTS      LOC_BIT(0)

#define R_CIO_CANFD_CSTS_TECCNT(x)       (((x) >> 24) & 0xff)
#define R_CIO_CANFD_CSTS_RECCNT(x)       (((x) >> 16) & 0xff)

/* RSCFDnCFDCmERFL / RSCFDnCmERFL */
#define R_CIO_CANFD_CERFL_ADERR       LOC_BIT(14)
#define R_CIO_CANFD_CERFL_B0ERR       LOC_BIT(13)
#define R_CIO_CANFD_CERFL_B1ERR       LOC_BIT(12)
#define R_CIO_CANFD_CERFL_CERR        LOC_BIT(11)
#define R_CIO_CANFD_CERFL_AERR        LOC_BIT(10)
#define R_CIO_CANFD_CERFL_FERR        LOC_BIT(9)
#define R_CIO_CANFD_CERFL_SERR        LOC_BIT(8)
#define R_CIO_CANFD_CERFL_ALF         LOC_BIT(7)
#define R_CIO_CANFD_CERFL_BLF         LOC_BIT(6)
#define R_CIO_CANFD_CERFL_OVLF        LOC_BIT(5)
#define R_CIO_CANFD_CERFL_BORF        LOC_BIT(4)
#define R_CIO_CANFD_CERFL_BOEF        LOC_BIT(3)
#define R_CIO_CANFD_CERFL_EPF         LOC_BIT(2)
#define R_CIO_CANFD_CERFL_EWF         LOC_BIT(1)
#define R_CIO_CANFD_CERFL_BEF         LOC_BIT(0)

#define R_CIO_CANFD_CERFL_ERR(x)      ((x) & (0x7fff)) /* above bits 14:0 */

/* RSCFDnCFDCmDCFG */
#define R_CIO_CANFD_DCFG_DSJW(x)      (((x) & 0x7) << 24)
#define R_CIO_CANFD_DCFG_DTSEG2(x)    (((x) & 0x7) << 20)
#define R_CIO_CANFD_DCFG_DTSEG1(x)    (((x) & 0xf) << 16)
#define R_CIO_CANFD_DCFG_DBRP(x)      (((x) & 0xff) << 0)

/* RSCFDnCFDCmFDCFG */
#define R_CIO_CANFD_FDCFG_TDCE        LOC_BIT(9)
#define R_CIO_CANFD_FDCFG_TDCOC       LOC_BIT(8)
#define R_CIO_CANFD_FDCFG_TDCO(x)         (((x) & 0x7f) >> 16)

/* RSCFDnCFDRFCCx */
#define R_CIO_CANFD_RFCC_RFIM         LOC_BIT(12)
#define R_CIO_CANFD_RFCC_RFDC(x)      (((x) & 0x7) << 8)
#define R_CIO_CANFD_RFCC_RFPLS(x)         (((x) & 0x7) << 4)
#define R_CIO_CANFD_RFCC_RFIE         LOC_BIT(1)
#define R_CIO_CANFD_RFCC_RFE          LOC_BIT(0)

/* RSCFDnCFDRFSTSx */
#define R_CIO_CANFD_RFSTS_RFMC        (0xff << 8)
#define R_CIO_CANFD_RFSTS_RFIF        LOC_BIT(3)
#define R_CIO_CANFD_RFSTS_RFMLT       LOC_BIT(2)
#define R_CIO_CANFD_RFSTS_RFFLL       LOC_BIT(1)
#define R_CIO_CANFD_RFSTS_RFEMP       LOC_BIT(0)

/* RSCFDnCFDRFIDx */
#define R_CIO_CANFD_RFID_RFIDE        LOC_BIT(31)
#define R_CIO_CANFD_RFID_RFRTR        LOC_BIT(30)

/* RSCFDnCFDRFPTRx */
#define R_CIO_CANFD_RFPTR_RFDLC(x)    (((x) >> 28) & 0xf)
#define R_CIO_CANFD_RFPTR_RFPTR(x)    (((x) >> 16) & 0xfff)
#define R_CIO_CANFD_RFPTR_RFTS(x)     (((x) >> 0) & 0xffff)

/* RSCFDnCFDRFFDSTSx */
#define R_CIO_CANFD_RFFDSTS_RFFDF      LOC_BIT(2)
#define R_CIO_CANFD_RFFDSTS_RFBRS      LOC_BIT(1)
#define R_CIO_CANFD_RFFDSTS_RFESI      LOC_BIT(0)

/* Common FIFO bits */

/* RSCFDnCFDCFCCk */
#define R_CIO_CANFD_CFCC_CFTML(x)     (((x) & 0xf) << 20)
#define R_CIO_CANFD_CFCC_CFM(x)       (((x) & 0x3) << 16)
#define R_CIO_CANFD_CFCC_CFIM         LOC_BIT(12)
#define R_CIO_CANFD_CFCC_CFDC(x)      (((x) & 0x7) << 8)
#define R_CIO_CANFD_CFCC_CFPLS(x)     (((x) & 0x7) << 4)
#define R_CIO_CANFD_CFCC_CFTXIE       LOC_BIT(2)
#define R_CIO_CANFD_CFCC_CFE          LOC_BIT(0)

/* RSCFDnCFDCFSTSk */
#define R_CIO_CANFD_CFSTS_CFMC(x)     (((x) >> 8) & 0xff)
#define R_CIO_CANFD_CFSTS_CFTXIF      LOC_BIT(4)
#define R_CIO_CANFD_CFSTS_CFMLT       LOC_BIT(2)
#define R_CIO_CANFD_CFSTS_CFFLL       LOC_BIT(1)
#define R_CIO_CANFD_CFSTS_CFEMP       LOC_BIT(0)

/* RSCFDnCFDCFIDk */
#define R_CIO_CANFD_CFID_CFIDE        LOC_BIT(31)
#define R_CIO_CANFD_CFID_CFRTR        LOC_BIT(30)
#define R_CIO_CANFD_CFID_CFID_MASK(x) ((x) & 0x1fffffff)

/* RSCFDnCFDCFPTRk */
#define R_CIO_CANFD_CFPTR_CFDLC(x)    (((x) & 0xf) << 28)
#define R_CIO_CANFD_CFPTR_CFPTR(x)    (((x) & 0xfff) << 16)
#define R_CIO_CANFD_CFPTR_CFTS(x)     (((x) & 0xff) << 0)

/* RSCFDnCFDCFFDCSTSk */
#define R_CIO_CANFD_CFFDCSTS_CFFDF    LOC_BIT(2)
#define R_CIO_CANFD_CFFDCSTS_CFBRS    LOC_BIT(1)
#define R_CIO_CANFD_CFFDCSTS_CFESI    LOC_BIT(0)

/** @} */


/**@defgroup canfdmain5 CAN-FD Driver Constants
 *
 * @brief CAN-FD driver configuration
 *
 * @{
 */

#define R_CIO_CANFD_FIFO_DEPTH       (8)                                        /**< Tx FIFO depth */
#define R_CIO_CANFD_NUM_CHANNELS     (2)                                        /**< Two channels per instance max */
#define R_CIO_CANFD_CHANNELS_MASK    LOC_BIT((R_CIO_CANFD_NUM_CHANNELS)-1)      /**< delimiter mask */
#define R_CIO_CANFD_GAFL_PAGENUM(entry)  ((entry) / 16)                         /**< Get the page number for an table entry */
#define R_CIO_CANFD_CHANNEL_NUMRULES     (1)                                    /**< only one rule per channel */

/**
 * @def R_CIO_CANFD_RFFIFO_IDX
 * Rx FIFO is a global resource of the controller. There are 8 such FIFOs
 * available. Each channel gets a dedicated Rx FIFO (i.e.) the channel
 * number is added to RFFIFO index.
 */
#define R_CIO_CANFD_RFFIFO_IDX       (0)

/**
 * @def R_CIO_CANFD_CFFIFO_IDX
 * Tx/Rx or Common FIFO is a per channel resource. Each channel has 3 Common
 * FIFOs dedicated to them. Use the first (index 0) FIFO out of the 3 for Tx.
 */
#define R_CIO_CANFD_CFFIFO_IDX       (0)

#define R_CIO_CANFD_PRV_RXBUF_SIZE   (10)                                       /**< Receive ring buffer number of messages */
#define R_CIO_CANFD_PRV_TXBUF_SIZE   (10)                                       /**< Transmit ring buffer number of messages */
#define R_CIO_CANFD_MAX_ERR_ACTIVE   (96)                                       /**< accepted amount of errors befor warning msg */
#define R_CIO_CANFD_MAX_ERR_WARNING  (128)                                      /**< accepted amount of errors befor error msg */
#define R_CIO_CANFD_PRV_MAX_OPEN     (1)                                        /**< max number of open channels to one instance */


/* controller area network (CAN) kernel definitions */

/* special address description flags for the CAN_ID */
#define R_CIO_CANFD_EFF_FLAG (0x80000000U) /**< EFF/SFF is set in the MSB */
#define R_CIO_CANFD_RTR_FLAG (0x40000000U) /**< remote transmission request */
#define R_CIO_CANFD_ERR_FLAG (0x20000000U) /**< error message frame */

/* valid bits in CAN ID for frame formats */
#define R_CIO_CANFD_SFF_MASK (0x000007FFU) /**< standard frame format (SFF) */
#define R_CIO_CANFD_EFF_MASK (0x1FFFFFFFU) /**< extended frame format (EFF) */
#define R_CIO_CANFD_ERR_MASK (0x1FFFFFFFU) /**< omit EFF, RTR, ERR flags */

/**
 * Controller Area Network Identifier structure
 *
 * bit 0-28 : CAN identifier (11/29 bit)
 * bit 29   : error message frame flag (0 = data frame, 1 = error message)
 * bit 30   : remote transmission request flag (1 = rtr frame)
 * bit 31   : frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
#define R_CIO_CANFD_SFF_ID_BITS     (11)
#define R_CIO_CANFD_EFF_ID_BITS     (29)

/** CAN payload length and DLC definitions according to ISO 11898-1 */
#define R_CIO_CANFD_CAN_MAX_DLC  (8)
#define R_CIO_CANFD_CAN_MAX_DLEN (8)

/** CAN-FD payload length and DLC definitions according to ISO 11898-7 */
#define R_CIO_CANFD_FD_MAX_DLC  (15)
#define R_CIO_CANFD_FD_MAX_DLEN (64)

#define R_CIO_CANFD_BRS (0x01) /**< bit rate switch (second bitrate for payload data) */
#define R_CIO_CANFD_ESI (0x02) /**< error state indicator of the transmitting node */


#define R_CIO_CANFD_FDF (0x4) /**< Flag to indicate that a CAN-FD frame has indeed CANFD format */

/** @} */

/*******************************************************************************
   Section: CANFD driver internal types
 */

/**@enum r_cio_canfd_BusState_t
 * @brief Possible states of the CAN bus
 */

typedef enum  {
    R_CIO_CANFD_STATE_ERROR_ACTIVE,    /**< Bus active, no Error detected  */
    R_CIO_CANFD_STATE_ERROR_WARNING,   /**< Warning, lostt messages  */
    R_CIO_CANFD_STATE_ERROR_PASSIVE,   /**< Bus passive, error state  */
    R_CIO_CANFD_STATE_BUS_OFF,         /**< Bus shut down   */
    R_CIO_CANFD_STATE_STOPPED          /**< Bus halted  */
} r_cio_canfd_BusState_t;

/**@struct  r_cio_canfd_Stats_t
 * @brief Counting communication errors
 */
typedef struct  {
    uint32_t RxErrors;                /**< Receive errors */
    uint32_t TxErrors;                /**< Receive errors */
    uint32_t RxDropped;               /**< Dropped Rx frames */
    uint32_t TxDropped;               /**< Dropped Tx frames */
} r_cio_canfd_Stats_t;

/**@struct r_cio_canfd_prv_DevInst_t
 * @brief CAN-FD Device definition
 */
typedef struct {
    size_t BaseAddr;                 /**< Register base address of the instance  */
    uint32_t ChannelMask;                   /**< Allowed/enabled Channels  */
    uint32_t FdMode;                        /**< Mode setting, FdMode == 1 means CanFD */
    int32_t IrqNumGl;                       /**< Global IRQ number */
    int32_t IrqNumCh;                       /**< Channel IRQ number */
    int32_t InUseCnt;                       /**< Instance counter */
    int32_t Error;                          /**< Actual error  */
    osal_device_handle_t device_handle;
} r_cio_canfd_prv_DevInst_t;

/**@struct r_cio_canfd_prv_ChInst_t
 * @brief CAN-FD Channel (instance) definition
 */
typedef struct {
    size_t BaseAddr;                  /**< Register base address of the instance  */
    uint32_t Ch;                      /**< Active Channel number */
    int32_t InUseCnt;                 /**< Instance counter */
    uint32_t FdMode;                  /**< Mode setting, FdMode == 1 means CanFD */
    int32_t Error;                    /**< Actual error  */
    r_cio_canfd_BusState_t State;     /**< Actual bus state */
    r_cio_canfd_Stats_t Stats;        /**< Statisitics */
    r_cio_can_Msg_t        *RxStrt;   /**< Receive buffer start reading postion */
    r_cio_can_Msg_t        *RxStop;   /**< Receive buffer stop reading postion*/
    r_cio_can_Msg_t        *RxEob;    /**< Receive buffer end */
    r_cio_can_Msg_t        *TxStrt;   /**< Transmit buffer start writing postion*/
    r_cio_can_Msg_t        *TxStop;   /**< Transmit buffer stop writing postion*/
    r_cio_can_Msg_t        *TxEob;    /**< Transmit buffer end */
    osal_mq_handle_t       MqHandle;  /**< Message to be sent when new data arrives,
                                           will be initilazied from "ouside" andd passed here by IOCTL */
    r_cio_can_Msg_t RxBuf[R_CIO_CANFD_PRV_RXBUF_SIZE];  /**<Receive Mesage buffer */
    r_cio_can_Msg_t TxBuf[R_CIO_CANFD_PRV_TXBUF_SIZE];  /**<Transmit Mesage buffer */
} r_cio_canfd_prv_ChInst_t;

/*******************************************************************************
   Section: Global (private) functions
 */

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_FindInstance
 */
/**
 * @brief  Find the one matching the given base addres in the local list of
 *  initialised instances
 *
 *  @param[in] Channel       - Channel number (one canFD has two channels)
 *
 *  @return  >0, pointer to the instance
 *  @return ==-1 - NG
 *
 */

r_cio_canfd_prv_ChInst_t *R_CIO_CANFD_PRV_FindInstance(size_t Channel);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_AllocInstance
 */
/**
 * @brief  Add the given instanace to the local list of initialised instances
 *
 *
 *  @param[in] Addr       - Channel address (one canFD has two channels)
 *
 *  @return >0, pointer to the instance
 *  @return ==-1 - NG
 *
 */
r_cio_canfd_prv_ChInst_t *R_CIO_CANFD_PRV_AllocInstance(size_t Addr);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_FreeInstance
 */
/**
 * @brief  Remove the given instanace from the local list of initialised instances
 *
 *
 *  @param[in] Inst       - pointer to the channel instance
 *
 *  @return == 0 - OK
 *  @return ==-1 - NG
 *
 */
int32_t R_CIO_CANFD_PRV_FreeInstance(r_cio_canfd_prv_ChInst_t *Inst);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_GlobalErr
 */
/**
 * @brief  Handle global CANFD errors for the given device
 *
 *
 * @param[in] Inst       - pointer to the Device instance
 * @param[in] GLError    - Global Errors
 *
 * @return void
 */
void R_CIO_CANFD_PRV_GlobalErr(r_cio_canfd_prv_DevInst_t *Inst, uint32_t GLError);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_ChannelErr
 */
/**
 * @brief  Handle channel related CANFD errors for the given instance
 *
 *
 *  @param[in] Inst       - pointer to the Channel instance
 *  @param[in] Cerfl      - Channel error flags
 *
 *  @return void
 */
void R_CIO_CANFD_PRV_ChannelErr(r_cio_canfd_prv_ChInst_t *Inst, uint32_t Cerfl);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_SetTiming
 */
/**
 * @brief  Set timing (baudrate) parameters for the given instance and channel
 *
 *  @param[in] Inst       - pointer to the Channel instance
 *
 *  @return void
 */
void R_CIO_CANFD_PRV_SetTiming(r_cio_canfd_prv_ChInst_t *Inst);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_EnableChannelInterrupts
 */
/**
 * @brief  Enable channel related (error) interrupts  for the given instance and channel
 *
 *  @param[in] Inst       - pointer to the Channel instance
 *
 *  @return void
 *
 **/
void R_CIO_CANFD_PRV_EnableChannelInterrupts(r_cio_canfd_prv_ChInst_t *Inst);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_DisableChannelInterrupts
 */
/**
 * @brief  Disable channel related (error) interrupts  for the given instance
 *
 *  @param[in] Inst       - pointer to the instance
 *
 *  @return void
 */
void R_CIO_CANFD_PRV_DisableChannelInterrupts(r_cio_canfd_prv_ChInst_t *Inst);

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_Start
 */
/**
 * @brief  Start CAN communication for the given instance
 *
 * @param[in] Inst       - pointer to the instance
 *
 * @return == 0 - OK
 * @return != 0 - NG
 *
 */
int32_t R_CIO_CANFD_PRV_Start(r_cio_canfd_prv_ChInst_t *Inst);

/*******************************************************************************
 * Function: R_CIO_CANFD_PRV_Stop
 */
/**
 * @brief  Stop CAN communication for the given instance
 *
 * @param[in] Inst       - pointer to the instance
 *
 * @return == 1 - OK
 * @return ==-1 - NG
 *
 */
int32_t R_CIO_CANFD_PRV_Stop(r_cio_canfd_prv_ChInst_t *Inst);

int32_t R_CIO_CANFD_PRV_IrqSetup(size_t Addr);

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_CANFD_MAIN_H */
/** @} */
