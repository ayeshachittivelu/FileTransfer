/*************************************************************************************************************
* cio_canfd_main_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Section: Includes
 */

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_bridge.h"
#include "r_cio_canfd.h"
#include "r_cio_canfd_main.h"
#include "r_cio_canfd_helper.h"
#include "r_cio_canfd_irq.h"
#include "r_cio_canfd_read.h"
#include "r_cio_canfd_write.h"
#include "r_cio_canfd_status.h"
#include "r_cio_canfd_ioctl.h"
#include "r_cio_can.h"
#include "r_cio_canfd_connect.h"
#include "r_cio_bsp_canfd_config.h"
#include "r_print_api.h"

/*******************************************************************************
   Section: Local types
 */

#define ANOTHER_CHAN(addr)        ((addr == 0) ? 1 : 0)

/*******************************************************************************
   Section: Local Variables
 */

static r_cio_canfd_prv_DevInst_t loc_CanFdDevInst;
static r_cio_canfd_prv_ChInst_t loc_CanFdChInst[R_CIO_CANFD_NUM_CHANNELS];

/* Counter for init/deint
   we have two channels handled separately, but just one CanFD
 */
static uint32_t loc_CanDevInstCnt[R_CIO_CANFD_NUM_CHANNELS] = {0, 0};

/*******************************************************************************
   Section: Local Functions
 */

/*******************************************************************************
   Function: loc_ResetCanFd
 */
static int32_t loc_ResetCanFd(r_cio_canfd_prv_DevInst_t *Inst)
{
    uint32_t status;
    uint32_t channel;
    int32_t err = 0;
    int32_t res;

    /* Check RAMINIT flag as CAN RAM initialization takes place
     * after the MCU reset
     */
    res = R_CIO_CANFD_PRV_READX_LOOP(R_CIO_CANFD_READ32, Inst->BaseAddr + R_CIO_CANFD_GSTS, status,
                                     !(status & R_CIO_CANFD_GSTS_GRAMINIT), 500000);
    if (0 == res) {
        R_PRINT_Log("[CioCanfdMain]: loc_ResetCanFd Global raminit failed\r\n");
        err = 1;
        goto quit;
    }

    /* Transition to Global Reset mode */
    R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr, R_CIO_CANFD_GCTR, R_CIO_CANFD_GCTR_GSLPR);
    R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_GCTR,
                              R_CIO_CANFD_GCTR_GMDC_MASK, R_CIO_CANFD_GCTR_GMDC_GRESET);

    /* Ensure Global reset mode */
    res = R_CIO_CANFD_PRV_READX_LOOP(R_CIO_CANFD_READ32, Inst->BaseAddr + R_CIO_CANFD_GSTS, status,
                                     (status & R_CIO_CANFD_GSTS_GRSTSTS), 500000);
    if (0 == res) {
        R_PRINT_Log("[CioCanfdMain]: loc_ResetCanFd Global reset failed\r\n");
        err = 1;
        goto quit;
    }

    /* Reset Global error flags */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GERFL, 0x0);

    /* Set the controller into appropriate mode */
    if (0 != Inst->FdMode) {
        R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_GRMCFG,
                               R_CIO_CANFD_GRMCFG_RCMC);
    } else {
        R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr, R_CIO_CANFD_GRMCFG,
                                 R_CIO_CANFD_GRMCFG_RCMC);
    }
    /* Transition all Channels to reset mode (CAN-FD hast two channels: 0 and 1) */
    for (channel = 0; channel < R_CIO_CANFD_NUM_CHANNELS; channel++) {
        R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr,
                                 R_CIO_CANFD_CCTR(channel), R_CIO_CANFD_CCTR_CSLPR);

        R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_CCTR(channel),
                                  R_CIO_CANFD_CCTR_CHMDC_MASK,
                                  R_CIO_CANFD_CCTR_CHDMC_CRESET);

        /* Ensure Channel reset mode */
        res = R_CIO_CANFD_PRV_READX_LOOP(R_CIO_CANFD_READ32, (Inst->BaseAddr + R_CIO_CANFD_CSTS(channel)), status,
                                         (status & R_CIO_CANFD_CSTS_CRSTSTS),
                                         500000);
        if (0 == res) {
            R_PRINT_Log("[CioCanfdMain]: loc_ResetCanFd channel %lu reset failed\r\n", channel);
            err = 1;
        }
    }
quit:
    return err;
}

/*******************************************************************************
   Function: loc_ConfigureCanFd
 */
static void loc_ConfigureCanFd(r_cio_canfd_prv_DevInst_t *Inst)
{
    uint32_t channel;
    uint32_t cfg = 0;

    /* Global configuration settings */
    /* ECC Error flag Enable */
    cfg = R_CIO_CANFD_GCFG_EEFE;
    if (0 != Inst->FdMode) {
        /* Truncate payload to configured message size RFPLS */
        cfg |= R_CIO_CANFD_GCFG_CMPOC;
    }

    /* Set External Clock if selected */
#if RCAR_CAN_FCANCLK_EXT
    cfg |= R_CIO_CANFD_GCFG_DCS;
#endif
    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_GCFG, cfg);

    /* Channel configuration settings */
    for (channel = 0; channel < R_CIO_CANFD_NUM_CHANNELS; channel++) {
        R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_CCTR(channel),
                               R_CIO_CANFD_CCTR_ERRD);
        R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_CCTR(channel),
                                  R_CIO_CANFD_CCTR_BOM_MASK,
                                  R_CIO_CANFD_CCTR_BOM_BENTRY);
    }
}

/*******************************************************************************
   Function: loc_ConfigureAflRules
 */
static void loc_ConfigureAflRules(r_cio_canfd_prv_ChInst_t *Inst)
{
    uint32_t cfg;
    int32_t offset;
    int32_t start;
    int32_t page;
    int32_t num_rules = R_CIO_CANFD_CHANNEL_NUMRULES;
    uint32_t ridx = Inst->Ch + R_CIO_CANFD_RFFIFO_IDX;

    if (0 == Inst->Ch) {
        start = 0; /* Channel 0 always starts from 0th rule */
    } else {
        /* Get number of Channel 0 rules and adjust */
        cfg = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_GAFLCFG0);
        start = R_CIO_CANFD_GAFLCFG_GETRNC(0, cfg);
    }

    /* Enable write access to entry */
    page = R_CIO_CANFD_GAFL_PAGENUM(start);
    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_GAFLECTR,
                           (R_CIO_CANFD_GAFLECTR_AFLPN(page) |
                            R_CIO_CANFD_GAFLECTR_AFLDAE));

    /* Write number of rules for channel */
    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_GAFLCFG0,
                           R_CIO_CANFD_GAFLCFG_SETRNC(Inst->Ch, num_rules));
    if (0 != Inst->FdMode) {
        offset = R_CIO_CANFD_F_GAFL_OFFSET;
    } else {
        offset = R_CIO_CANFD_C_GAFL_OFFSET;
    }
    /* Accept all IDs */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GAFLID(offset, start), 0);
    /* IDE or RTR is not considered for matching */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GAFLM(offset, start), 0);
    /* Any data length accepted */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GAFLP0(offset, start), 0);
    /* Place the msg in corresponding Rx FIFO entry */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GAFLP1(offset, start),
                             R_CIO_CANFD_GAFLP1_GAFLFDP(ridx));

    /* Disable write access to page */
    R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr,
                             R_CIO_CANFD_GAFLECTR, R_CIO_CANFD_GAFLECTR_AFLDAE);
}

/*******************************************************************************
   Function: loc_ConfigureRx
 */
static void loc_ConfigureRx(r_cio_canfd_prv_ChInst_t *Inst)
{
    /* Rx FIFO is used for reception */
    uint32_t cfg;
    uint16_t rfdc;
    uint16_t rfpls;

    /* Select Rx FIFO based on channel */
    uint32_t ridx = Inst->Ch + R_CIO_CANFD_RFFIFO_IDX;

    rfdc = 2;       /* b010 - 8 messages Rx FIFO depth */
    if (0 != Inst->FdMode) {
        rfpls = 7;  /* b111 - Max 64 bytes payload */
    } else {
        rfpls = 0;  /* b000 - Max 8 bytes payload */
    }
    cfg = (R_CIO_CANFD_RFCC_RFIM | R_CIO_CANFD_RFCC_RFDC(rfdc) |
           R_CIO_CANFD_RFCC_RFPLS(rfpls) | R_CIO_CANFD_RFCC_RFIE);
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_RFCC(ridx), cfg);
}

/*******************************************************************************
   Function: loc_ConfigureTx
 */
static void loc_ConfigureTx(r_cio_canfd_prv_ChInst_t *Inst)
{
    /* Tx/Rx(Common) FIFO configured in Tx mode is
     * used for transmission
     *
     * Each channel has 3 Common FIFO dedicated to them.
     * Use the 1st (index 0) out of 3
     */
    uint32_t cfg;
    uint16_t cftml;
    uint16_t cfm;
    uint16_t cfdc;
    uint16_t cfpls;

    cftml = 0;      /* 0th buffer */
    cfm = 1;        /* b01 - Transmit mode */
    cfdc = 2;       /* b010 - 8 messages Tx FIFO depth */
    if (0 != Inst->FdMode) {
        cfpls = 7;  /* b111 - Max 64 bytes payload */
    } else {
        cfpls = 0;  /* b000 - Max 8 bytes payload */
    }
    cfg = (R_CIO_CANFD_CFCC_CFTML(cftml) | R_CIO_CANFD_CFCC_CFM(cfm) |
           R_CIO_CANFD_CFCC_CFIM | R_CIO_CANFD_CFCC_CFDC(cfdc) |
           R_CIO_CANFD_CFCC_CFPLS(cfpls) | R_CIO_CANFD_CFCC_CFTXIE);
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CFCC(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), cfg);

    if (0 != Inst->FdMode) {
        /* Clear FD mode specific control/status register */
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                 R_CIO_CANFD_F_CFFDCSTS(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), 0);
    }
}


/*******************************************************************************
   Section: Global private Functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_GlobalErr
 */
void R_CIO_CANFD_PRV_GlobalErr(r_cio_canfd_prv_DevInst_t *Inst, uint32_t GLError)
{
    uint32_t status = 0;
    uint32_t ridx = 0;
    uint32_t channel = 0;

    R_CIO_CANFD_PRV_FOR_EACH_SET_BIT(channel, Inst->ChannelMask, R_CIO_CANFD_NUM_CHANNELS) {
        if ((0 != (GLError & R_CIO_CANFD_GERFL_EEF0)) && (0 == channel)) {
            R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Ch0 ECC Error\r\n");
            loc_CanFdChInst[channel].Stats.TxDropped++;
        }
        if ((0 != (GLError & R_CIO_CANFD_GERFL_EEF1)) && (1 == channel)) {
            R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Ch1 ECC Error\r\n");
            loc_CanFdChInst[channel].Stats.TxDropped++;
        }
        ridx = channel + R_CIO_CANFD_RFFIFO_IDX;
        if (0 != (GLError & R_CIO_CANFD_GERFL_MES)) {
            status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr,
                                             R_CIO_CANFD_CFSTS(channel, R_CIO_CANFD_CFFIFO_IDX));
            if (0 != (status & R_CIO_CANFD_CFSTS_CFMLT)) {
                R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Tx Message Lost\r\n");
                loc_CanFdChInst[channel].Stats.TxDropped++;
                R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                         R_CIO_CANFD_CFSTS(channel, R_CIO_CANFD_CFFIFO_IDX),
                                         status & ~R_CIO_CANFD_CFSTS_CFMLT);
            }

            status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_RFSTS(ridx));
            if (0 != (status & R_CIO_CANFD_RFSTS_RFMLT)) {
                R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Rx Message Lost\r\n");
                loc_CanFdChInst[channel].Stats.RxDropped++;
                R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_RFSTS(ridx),
                                         status & ~R_CIO_CANFD_RFSTS_RFMLT);
            }
        }
    }
    if ((0 != Inst->FdMode) && (0 != (GLError & R_CIO_CANFD_GERFL_CMPOF))) {
        /* Message Lost flag will be set for respective channel
         * when this condition happens with counters and flags
         * already updated.
         */
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr global payload overflow interrupt\r\n");
    }
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_ChannelErr
 */
void R_CIO_CANFD_PRV_ChannelErr(r_cio_canfd_prv_ChInst_t *Inst, uint32_t Cerfl)
{

/* Channel error interrupts */
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_BEF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Bus error\r\n");
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_ADERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr ACK Delimiter Error\r\n");
        Inst->Stats.TxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_B0ERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Bit Error (dominant)\r\n");
        Inst->Stats.TxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_B1ERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Bit Error (recessive)\r\n");
        Inst->Stats.TxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_CERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr CRC Error\r\n");
        Inst->Stats.RxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_AERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr ACK Error\r\n");
        Inst->Stats.TxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_FERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Form Error\r\n");
        Inst->Stats.RxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_SERR)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Stuff Error\r\n");
        Inst->Stats.RxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_ALF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Arbitration lost Error\r\n");
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_BLF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Bus Lock Error\r\n");
        Inst->Stats.RxErrors++;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_EWF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Error warning interrupt\r\n");
        Inst->State = R_CIO_CANFD_STATE_ERROR_WARNING;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_EPF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Error passive interrupt\r\n");
        Inst->State = R_CIO_CANFD_STATE_ERROR_PASSIVE;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_BOEF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Bus-off entry interrupt\r\n");
        Inst->State = R_CIO_CANFD_STATE_BUS_OFF;
    }
    if (0 != (Cerfl & R_CIO_CANFD_CERFL_OVLF)) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_GlobalErr Overload Frame Transmission error interrupt\r\n");
        Inst->Stats.TxErrors++;
    }
    /* Clear channel error interrupts that are handled */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CERFL(Inst->Ch),
                             R_CIO_CANFD_CERFL_ERR(~Cerfl));
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Start
*/

int32_t R_CIO_CANFD_PRV_Start(r_cio_canfd_prv_ChInst_t *Inst)
{
    int32_t err = 0;
    uint32_t status = 0;
    uint32_t ridx = Inst->Ch + R_CIO_CANFD_RFFIFO_IDX;
    /* Clear all global error interrupts. Only affected channels bits
     * get cleared
     */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GERFL, 0);

    /* Clear channel error interrupts */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CERFL(Inst->Ch),
                             R_CIO_CANFD_CERFL_ERR(0));
    R_CIO_CANFD_PRV_SetTiming(Inst);

    R_CIO_CANFD_PRV_EnableChannelInterrupts(Inst);

    /* Check Global operation mode */
    status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_GSTS);
    if (0 != (status & R_CIO_CANFD_GSTS_GNOPM))
    {
        /* Start Global operation mode */
        R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_GCTR, R_CIO_CANFD_GCTR_GMDC_MASK,
                                  R_CIO_CANFD_GCTR_GMDC_GOPM);
        do
        {
            status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_GSTS);
        } while (0 != (status & R_CIO_CANFD_GSTS_GNOPM));
    }

    /* Set channel to Operational mode */
    R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_CCTR(Inst->Ch),
                              R_CIO_CANFD_CCTR_CHMDC_MASK, R_CIO_CANFD_CCTR_CHDMC_COPM);
    /* Verify channel mode change */
    do
    {
        status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_CSTS(Inst->Ch));
    } while (0 == (status & R_CIO_CANFD_CSTS_COMSTS));

    /* Enable Common & Rx FIFO */
    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_CFCC(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX),
                           R_CIO_CANFD_CFCC_CFE);
    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_RFCC(ridx), R_CIO_CANFD_RFCC_RFE);
    /* Initial CAN bus State */
    Inst->State = R_CIO_CANFD_STATE_ERROR_ACTIVE;
    /* Clear all global error interrupts. Only affected channels bits
     * get cleared
     */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GERFL, 0);
    /* Clear channel error interrupts */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CERFL(Inst->Ch),
                             R_CIO_CANFD_CERFL_ERR(0));

    return err;
}
/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Stop
 */

int32_t R_CIO_CANFD_PRV_Stop(r_cio_canfd_prv_ChInst_t *Inst)
{
    int32_t err = 0;
    uint32_t status;
    uint32_t ridx = Inst->Ch + R_CIO_CANFD_RFFIFO_IDX;

    /* Transition to channel reset mode  */
    R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_CCTR(Inst->Ch),
                              R_CIO_CANFD_CCTR_CHMDC_MASK, R_CIO_CANFD_CCTR_CHDMC_CRESET);
    do
    {
        status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_CSTS(Inst->Ch));
    } while (0 == (status & R_CIO_CANFD_CSTS_CRSTSTS));

    /* Transition to global reset mode  */
    R_CIO_CANFD_PRV_UpdateReg(Inst->BaseAddr + R_CIO_CANFD_GCTR,
                              R_CIO_CANFD_GCTR_GMDC_MASK, R_CIO_CANFD_GCTR_GMDC_GRESET);

    do
    {
        status = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_GSTS);
    } while (0 == (status & R_CIO_CANFD_GSTS_GRSTSTS));

    R_CIO_CANFD_PRV_DisableChannelInterrupts(Inst);

    /* Disable Common & Rx FIFO */
    R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr, R_CIO_CANFD_CFCC(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX),
                             R_CIO_CANFD_CFCC_CFE);
    R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr, R_CIO_CANFD_RFCC(ridx), R_CIO_CANFD_RFCC_RFE);
    /* Set the state as STOPPED */
    Inst->State = R_CIO_CANFD_STATE_STOPPED;

    return err;
}

int32_t R_CIO_CANFD_PRV_IrqSetup(size_t Addr)
{
    int err = 0;

    /* Interrupt setting */
    if ((0 == loc_CanDevInstCnt[0]) && (0 == loc_CanDevInstCnt[1])) {
        err = (int)R_CIO_CANFD_PRV_IrqInit(&loc_CanFdDevInst);
        if (0 != err) {
            R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_IrqInit failed\r\n");
        }
    }

    if (0 == err) {
        loc_CanDevInstCnt[Addr]++;
    }

    return err;
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Init
 */
/**
 * @brief  CANFD internal initialization
 *
 * Initialize one CAN-FD driver instance and CAN-FD device instance (global and specified channel).
 * Initialize the hardware CANFD interrupts.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] IrqNum     - Interrupt ID of CAN-FD global
 *
 * @return == 0 - OK
 * @return != 0 - NG
 */
static int R_CIO_CANFD_PRV_Init(size_t Addr, int IrqNum)
{
    int err = 0;
    r_cio_canfd_prv_ChInst_t *inst;

    if (R_CIO_CANFD_NUM_CHANNELS <= Addr) {
        err = -1;
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_Init Channel NG! Channel(%d)\r\n", Addr);
    }

    if (0 == err) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_Init CIO %s driver channel %d init, IRQ %d\n",
                    R_CIO_CANFD_Driver.Name, Addr, IrqNum);
        inst = R_CIO_CANFD_PRV_AllocInstance(Addr);
        if (0 == inst) {
            R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_Init R_CIO_CANFD_PRV_AllocInstance failed return 0\r\n");
            err = -1;
            goto exit;
        }
        /* 2Global" Device */

        loc_CanFdDevInst.IrqNumGl = IrqNum;
        loc_CanFdDevInst.IrqNumCh = IrqNum - 1;        /* That should be hopefully ;) correct for all R-Car devices */
        loc_CanFdDevInst.FdMode = 1;          /* */
        loc_CanFdDevInst.Error = 0;
        loc_CanFdDevInst.BaseAddr = inst->BaseAddr;
        loc_CanFdDevInst.ChannelMask |= (1 << Addr);

        /* Channel set up */
        inst->RxStrt = inst->RxBuf;
        /* Some better configurability should be addded later */
        inst->FdMode = 1;


        inst->RxStop = inst->RxStrt;
        inst->RxEob = inst->RxStop + (R_CIO_CANFD_PRV_RXBUF_SIZE - 1);
        inst->TxStrt = inst->TxBuf;
        inst->TxStop = inst->TxStrt;
        inst->TxEob = inst->TxStop + (R_CIO_CANFD_PRV_TXBUF_SIZE - 1);
        inst->Error = 0;
        inst->InUseCnt = 0;
        inst->State = R_CIO_CANFD_STATE_STOPPED;
        inst->Stats.RxDropped = 0;
        inst->Stats.TxDropped = 0;
        inst->Stats.RxErrors = 0;
        inst->Stats.TxErrors = 0;

        /*           */
        /* First, reset controller, but only at first init  */
        if ((0 == loc_CanDevInstCnt[0]) && (0 == loc_CanDevInstCnt[1])) {
            err = (int)loc_ResetCanFd(&loc_CanFdDevInst);
            if (0 != err) {
                R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_Init CANFD reset failed \r\n");
                goto exit;
            }
            /* Interrupt setting */
            /* Controller in Global reset & Channel reset mode */
            loc_ConfigureCanFd(&loc_CanFdDevInst);
        }

        /* Configure per channel attributes */

        /* Configure Channel's Rx fifo */
        loc_ConfigureRx(inst);

        /* Configure Channel's Tx (Common) fifo */
        loc_ConfigureTx(inst);

        /* Configure receive rules */
        loc_ConfigureAflRules(inst);
    }
exit:
    if (0 == err) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_Init OK\r\n");
    } else {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_Init NG!\r\n");
    }

    return err;
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_DeInit
 */
/**
 * @brief  CANFD internal de-initialization
 *
 * De-initialize one CAN-FD driver instance and release the system resource.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] IrqNum     - Interrupt ID of CAN-FD global
 *
 * @return == 0 - OK
 * @return != 0 - NG
 */
static int R_CIO_CANFD_PRV_DeInit(size_t Addr)
{
    int x = 0;
    r_cio_canfd_prv_ChInst_t *inst;

    if (R_CIO_CANFD_NUM_CHANNELS <= Addr) {
        x = -1;
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_DeInit Channel NG! Channel(%d)\r\n", Addr);
    }

    if (0 == x) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_DeInit CIO %s driver deinit at Channel(%d)\n",
                    R_CIO_CANFD_Driver.Name, Addr);

        inst = R_CIO_CANFD_PRV_FindInstance(Addr);
        if (inst) {
            /* Interrupt setting */
            if ((1 == loc_CanDevInstCnt[Addr]) && (0 == loc_CanDevInstCnt[ANOTHER_CHAN(Addr)])) {
                x = (int)R_CIO_CANFD_PRV_IrqDeInit(&loc_CanFdDevInst);
                if (x == 0) {
                    x = (int)loc_ResetCanFd(&loc_CanFdDevInst);
                    if (0 != x) {
                        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_DeInit CANFD reset failed \r\n");
                    }
                }
            }
            if (x == 0) {
                R_CIO_CANFD_PRV_DisableChannelInterrupts(inst);
            }

            if (1 == loc_CanDevInstCnt[Addr]) {
                /* Enter channel stop mode */
                R_CIO_CANFD_PRV_SetBit(inst->BaseAddr, R_CIO_CANFD_CCTR(Addr), R_CIO_CANFD_CCTR_CSLPR);
                if (0 == loc_CanDevInstCnt[ANOTHER_CHAN(Addr)]) {
                    /* Enter global stop mode */
                    R_CIO_CANFD_PRV_SetBit(inst->BaseAddr, R_CIO_CANFD_GCTR, R_CIO_CANFD_GCTR_GSLPR);
                }
            }

            x = (int)R_CIO_CANFD_PRV_FreeInstance(inst);

            if ((0 == x) && (0 < loc_CanDevInstCnt[Addr])) {
                loc_CanDevInstCnt[Addr]--;
            }
        } else {
            R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_DeInit R_CIO_CANFD_PRV_FindInstance failed. return 0.\r\n");
            x = -1;
        }
    }

    if (x == 0) {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_DeInit OK\r\n");
    } else {
        R_PRINT_Log("[CioCanfdMain]: R_CIO_CANFD_PRV_DeInit NG\r\n");
    }
    return x;
}


/*******************************************************************************
   Section: Global functions
 */
void R_CIO_CANFD_PRV_SetTiming(r_cio_canfd_prv_ChInst_t *Inst)
{
    uint32_t cfg = 0;
    uint32_t ntseg1 = 0;
    uint32_t ntseg2 = 0;
    uint32_t nsjw = 0;
    uint32_t nbrp = 0;

    uint32_t dtseg1 = 0;
    uint32_t dtseg2 = 0;
    uint32_t dsjw = 0;
    uint32_t dbrp = 0;

    /* Nominal bit timing settings,
       For the moment, just one fixed setting */
    if (0 != Inst->FdMode) {
#if 0
        /* CAN-FD only mode
         * NBRP = 1/1
         * NTSEG1 = 29 Tq
         * NTSEG2 = 10 Tq
         * NSJW = 3 Tq
         * 80MHz/2/1/40Tq(NTSEG1=29,NTSEG2=10,SS=1)=Arbitration 1Mbps
         * (1+29)/40 = 75%
         */
        cfg = 0x091C1000;
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CCFG(Inst->Ch), cfg);

       /*
        * DBRP = 1/1
        * DTSEG1 = 5 Tq
        * DTSEG2 = 2 Tq
        * DSJW = 1 Tq/
        * 80MHz/2/1/8Tq(DTSEG1=5,DTSEG2=2,SS=1)=Payload 5Mbps
        * (1+5)/8 = 75%
        */
        cfg = 0x00140000;
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_F_DCFG(Inst->Ch), cfg);
#else
        /* CAN-FD only mode
         * NTSEG2 (bit 26:24) = 9 (= 10 Tq)
         * NTSEG1 (bit 22:16) = 0x1C = 28 ( = 29 Tq)
         * NSJW (bit 15:11) = 2 (= 3 Tq)
         * NBRP (bit 9:0) = 0 (= 1/1)
         * 80MHz/2/1/40Tq(NTSEG1=29,NTSEG2=10,SS=1)=Arbitration 1Mbps
         * Sample Point : (1+29)/40 = 75%
         */
        cfg = 0x091C1000;
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CCFG(Inst->Ch), cfg);

       /*
        * DSJW (bit 26:24) = 0 (= 1 Tq)
        * DTSEG2 (bit 22:20) = 4 (= 5 Tq)
        * DTSEG1 (bit 19:16) = 0xD = 13 (= 14 Tq)
        * DBRP (bit 7:0) = 1 (= 1/2)
        * 80MHz/2/2/20Tq(DTSEG1=14,DTSEG2=5,SS=1)=Payload 1Mbps
        * Sample Point : (1+14)/20 = 75%
        */
        cfg = 0x004D0001;
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_F_DCFG(Inst->Ch), cfg);
#endif
    } else {
        /* Classical CAN only mode
           20MHz/2 = 10MHz : SJW=1, TSEG2=3, TSEG1=6 , SS=1(fixed) -> Total 10 Tq : 1MHz=1Mbps
           => *((volatile uint32_t*)RSCFD0CmCFG(chNo)) = 0x00250001;
         */
        cfg = 0x00250001;
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CCFG(Inst->Ch), cfg);
    }
}


/*******************************************************************************
   Function: R_CIO_CANFD_PRV_FreeInstance
 */

int32_t R_CIO_CANFD_PRV_FreeInstance(r_cio_canfd_prv_ChInst_t *Inst)
{

    if (0 != Inst) {
        Inst->BaseAddr = 0;
        return 0;
    }
    return -1;
}


/*******************************************************************************
   Function: R_CIO_CANFD_PRV_FindInstance
 */

r_cio_canfd_prv_ChInst_t *R_CIO_CANFD_PRV_FindInstance(size_t Channel)
{
    r_cio_canfd_prv_ChInst_t *inst;
    int32_t i;

    i = 0;
    inst = 0;
    while (i < R_CIO_CANFD_NUM_CHANNELS) {
        if (loc_CanFdChInst[i].Ch == Channel) {
            inst = &loc_CanFdChInst[i];
            break;
        }
        i++;
    }
    return inst;
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_AllocInstance
 */

r_cio_canfd_prv_ChInst_t *R_CIO_CANFD_PRV_AllocInstance(size_t Channel)
{
    r_cio_canfd_prv_ChInst_t *inst;
    int32_t i;

    i = 0;
    inst = 0;
    while (i < R_CIO_CANFD_NUM_CHANNELS) {
        if (0 == loc_CanFdChInst[i].BaseAddr) {
            loc_CanFdChInst[i].BaseAddr = R_CIO_CANFD_BASE_ADDR;
            loc_CanFdChInst[i].Ch = Channel;
            inst = &loc_CanFdChInst[i];
            break;
        }
        i++;
    }
    return inst;
}

/*******************************************************************************
   Section: Global variables
 */

const r_cio_Driver_t R_CIO_CANFD_Driver = {
    "R-CAR_cio_canfd",
    R_CIO_CANFD_PRV_Init,
    R_CIO_CANFD_PRV_DeInit,
    R_CIO_CANFD_PRV_Open,
    R_CIO_CANFD_PRV_Close,
    R_CIO_CANFD_PRV_Read,
    R_CIO_CANFD_PRV_Write,
    R_CIO_CANFD_PRV_IoCtl,
    R_CIO_CANFD_PRV_Status
};


