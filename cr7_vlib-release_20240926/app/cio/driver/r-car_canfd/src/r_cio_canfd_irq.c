/*************************************************************************************************************
* cio_canfd_irq_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Section: Includes
 */

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_bridge.h"
#include "r_cio_canfd_main.h"
#include "r_cio_canfd_helper.h"
#include "r_cio_canfd_irq.h"
#include "r_cio_canfd_write.h"
#include "r_cio_can.h"
#include "r_print_api.h"

/*******************************************************************************
   Section: Local Defines
 */

/* timeout (ms) */
#define TIMEOUT_MS 1000 /* 1000 milisecond */

#define CIO_CANFD_GLOBAL 1
#define CIO_CANFD_CHANNEL 0
#define CIO_CANFD_DEVICE_CHANNEL 2

#define CIO_CANFD_MQ_MSG_SIZE      sizeof(uint32_t)

/*******************************************************************************
   Section: Local Variables
 */
/* Datal lenght "translation table" */
static uint8_t loc_CanFdDlc[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64
};


/*******************************************************************************
   Section: Local Functions
 */
extern osal_device_handle_t osal_devhandle_canfd;

/*******************************************************************************
   Function: loc_EnableGlobalInterrupts
 */
static void loc_EnableGlobalInterrupts(r_cio_canfd_prv_DevInst_t *Inst)
{
    uint32_t ctr;

    /* Clear any stray error interrupt flags */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GERFL, 0);

    /* Global interrupts setup */
    ctr = R_CIO_CANFD_GCTR_MEIE;
    if (0 != Inst->FdMode) {
        ctr |= R_CIO_CANFD_GCTR_CFMPOFIE;
    }
    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_GCTR, ctr);
}

/*******************************************************************************
   Function: loc_DisableGlobalInterrupts
 */
static void loc_DisableGlobalInterrupts(r_cio_canfd_prv_DevInst_t *Inst)
{
    /* Disable all interrupts */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GCTR, 0);

    /* Clear any stray error interrupt flags */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_GERFL, 0);
}



/*******************************************************************************
   Function: loc_TxIrq
 */
static int32_t loc_TxIrq(r_cio_canfd_prv_ChInst_t *Inst)
{
    r_cio_can_Msg_t *nxt;

    /* check if we still have data to send */
    if (Inst->TxStrt == Inst->TxStop) {
        return 0;
    }
    nxt = Inst->TxStrt + 1;
    if (nxt > Inst->TxEob) {
        nxt = Inst->TxBuf;
    }

    /* send the pending message out */
    R_CIO_CANFD_PRV_TransmitFrame(Inst);

    /* Move DataQue pointer */
    Inst->TxStrt = nxt;

    if (Inst->TxStrt != Inst->TxStop) {
        return 1;
    } else {
        return 0;
    }

}


/*******************************************************************************
   Function: loc_RxIrq
 */

static int32_t loc_RxIrq(r_cio_canfd_prv_ChInst_t *Inst)
{
    r_cio_can_Msg_t *nxt;
    size_t addr;
    uint32_t val;
    uint32_t i;
    uint32_t lwords;
    uint32_t offset;
    uint32_t sts = 0;
    uint32_t rfid;
    uint32_t id;
    uint32_t dlc;
    uint32_t flags;
    uint32_t ridx = Inst->Ch + R_CIO_CANFD_RFFIFO_IDX;
    uint32_t returndata = 0;
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /* Has a new message been received? */
    addr = Inst->BaseAddr + R_CIO_CANFD_RFSTS(Inst->Ch);
    val = R_CIO_CANFD_READ32(addr);
    flags = 0;
    /* Check FIFO empty condition */
    if (0 == (val & R_CIO_CANFD_RFSTS_RFIF)) {
        return 0;
    }
    /* CAN-FD ? */
    if (0 != Inst->FdMode) {
        rfid = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_F_RFID(ridx));
        dlc = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_F_RFPTR(ridx));
        sts = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_F_RFFDSTS(ridx));
    } else {
        rfid = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_C_RFID(ridx));
        dlc = R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, R_CIO_CANFD_C_RFPTR(ridx));
    }
    if (0 != (rfid & R_CIO_CANFD_RFID_RFIDE)) {
        id = (rfid & R_CIO_CANFD_EFF_MASK) | R_CIO_CANFD_EFF_FLAG;
    } else {
        id = rfid & R_CIO_CANFD_SFF_MASK;
    }
    dlc = R_CIO_CANFD_RFPTR_RFDLC(dlc);
    if (0 != Inst->FdMode) {
        dlc = loc_CanFdDlc[dlc];
        if (0 == (sts & R_CIO_CANFD_RFFDSTS_RFFDF)) {
            /* check/ensure proper limits in case of non canFD frame */
            if (R_CIO_CANFD_CAN_MAX_DLC < dlc) {
                dlc =  R_CIO_CANFD_CAN_MAX_DLC;
            }
        }
        if (0 != (sts & R_CIO_CANFD_RFFDSTS_RFESI)) {
            flags |= R_CIO_CANFD_ESI;
        }
        if ((0 == (sts & R_CIO_CANFD_RFFDSTS_RFFDF)) && (0 != (rfid & R_CIO_CANFD_RFID_RFRTR))) {
            id |= R_CIO_CANFD_RTR_FLAG;
        } else {
            if (sts & R_CIO_CANFD_RFFDSTS_RFBRS) {
                flags |= R_CIO_CANFD_BRS;
            }
            if (sts & R_CIO_CANFD_RFFDSTS_RFFDF) {
                flags |= R_CIO_CANFD_FDF;
            }
            nxt = Inst->RxStop + 1;
            if (nxt > Inst->RxEob) {
                nxt = Inst->RxBuf;
            }
            if (Inst->RxStrt != nxt) {
                Inst->RxStop->DataLen = dlc;
                Inst->RxStop->Id = id;
                Inst->RxStop->Flags = flags;
                lwords = R_CIO_CANFD_PRV_ROUND_UP(dlc, sizeof(uint32_t));
                offset = R_CIO_CANFD_F_RFDF(ridx, 0);
                for (i = 0; i < lwords; i++) {
                    *((uint32_t *)Inst->RxStop->Data + i) =
                        R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, offset + (i * sizeof(uint32_t)));
                }
                /* Inform about the event */
                if (NULL != Inst->MqHandle) {
                    osal_ret = R_OSAL_MqSendForTimePeriod(Inst->MqHandle, TIMEOUT_MS, &returndata, CIO_CANFD_MQ_MSG_SIZE);
                    if (OSAL_RETURN_OK != osal_ret) {
                        R_PRINT_Log("[CioCnafdIrq]: loc_RxIrq CANFDMode R_OSAL_MqSendForTimePeriod failed(%d)\r\n",
                                    osal_ret);
                    }
                }
                Inst->RxStop = nxt;
            } else {
                R_PRINT_Log("[CioCnafdIrq]: loc_RxIrq CANFD incorrect nxt\n");
                Inst->RxStop = nxt; /* We have to drop packet otherwise we lock-up ...*/
                return -1;
            }
        }
    } else { /* ... if CAN-FD  mode */
        if (R_CIO_CANFD_CAN_MAX_DLC < dlc) {
            /* incorrect size */
            R_PRINT_Log("[CioCnafdIrq]: loc_RxIrq CAN incorrect DLC(%d)\n", dlc);
            return -1;
        }
        dlc = loc_CanFdDlc[dlc];
        if (0 != (id & R_CIO_CANFD_RFID_RFRTR)) {
            id |= R_CIO_CANFD_RTR_FLAG;
        } else {
            nxt = Inst->RxStop + 1;
            if (nxt > Inst->RxEob) {
                nxt = Inst->RxBuf;
            }
            if (Inst->RxStrt != nxt) {
                Inst->RxStop->DataLen = dlc;
                Inst->RxStop->Id = id;
                Inst->RxStop->Flags = flags;
                lwords = R_CIO_CANFD_PRV_ROUND_UP(dlc, sizeof(uint32_t));
                offset = R_CIO_CANFD_F_RFDF(ridx, 0);
                for (i = 0; i < lwords; i++) {
                    *((uint32_t *)Inst->RxStop->Data + i) =
                        R_CIO_CANFD_PRV_ReadReg(Inst->BaseAddr, offset + (i * sizeof(uint32_t)));
                }
                Inst->RxStop = nxt;
                /* Inform about the event */
                if (NULL != Inst->MqHandle) {
                    osal_ret = R_OSAL_MqSendForTimePeriod(Inst->MqHandle, TIMEOUT_MS, &returndata, CIO_CANFD_MQ_MSG_SIZE);
                    if (OSAL_RETURN_OK != osal_ret) {
                        R_PRINT_Log("[CioCnafdIrq]: loc_RxIrq Not CANFDMode R_OSAL_MqSendForTimePeriod failed(%d)\r\n",
                                    osal_ret);
                    }
                }

            } else {
                /* no space */
                R_PRINT_Log("[CioCnafdIrq]: loc_RxIrq CAN incorrect nxt\n");
                return -1;
            }
        }
    }
    /* Write 0xff to RFPC to increment the CPU-side
     * pointer of the Rx FIFO
     */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_RFPCTR(ridx), 0xff);

    return 0;
}

/*******************************************************************************
   Function: loc_GlobalIrqThread

   CANFD  global IRQ thread.

   Parameters:
   Arg        - Thread parameter

   Returns:
   void*
 */

static void *loc_GlobalIrqThread(osal_device_handle_t device_handle, uint64_t irq_channel, void* Arg)
{
    int32_t x;
    uint32_t sts;
    uint32_t ch;
    uint32_t gerfl;
    r_cio_canfd_prv_DevInst_t   *dev_inst;
    r_cio_canfd_prv_ChInst_t    *ch_inst;
    uint32_t ridx;

    dev_inst = (r_cio_canfd_prv_DevInst_t *)Arg;

    /* Global error interrupts */
    gerfl = R_CIO_CANFD_PRV_ReadReg(dev_inst->BaseAddr, R_CIO_CANFD_GERFL);

    if (0 != R_CIO_CANFD_GERFL_ERR(dev_inst, gerfl)) {
        R_CIO_CANFD_PRV_GlobalErr(dev_inst, gerfl);
        dev_inst->Error = gerfl;
        /* Clear all global error interrupts. Only affected channels bits
         * get cleared
         */
        R_CIO_CANFD_PRV_WriteReg(dev_inst->BaseAddr, R_CIO_CANFD_GERFL, 0);

    }

    R_CIO_CANFD_PRV_FOR_EACH_SET_BIT(ch, dev_inst->ChannelMask, R_CIO_CANFD_NUM_CHANNELS) {

        ridx = ch + R_CIO_CANFD_RFFIFO_IDX;
        ch_inst = R_CIO_CANFD_PRV_FindInstance(ch);
        /* check FIFO in any case */
        x = loc_RxIrq(ch_inst);
        if (0 != x) {
            dev_inst->Error = -1;
        }
        /* Handle Rx interrupt flags  */
        sts = R_CIO_CANFD_PRV_ReadReg(dev_inst->BaseAddr, R_CIO_CANFD_RFSTS(ridx));
        if (0 != (sts & R_CIO_CANFD_RFSTS_RFIF)) {
            while (0 != (sts & R_CIO_CANFD_RFSTS_RFMC)) {
                x = loc_RxIrq(ch_inst);
                sts = R_CIO_CANFD_PRV_ReadReg(dev_inst->BaseAddr, R_CIO_CANFD_RFSTS(ridx)) & R_CIO_CANFD_RFSTS_RFMC;
            }

            /* Clear interrupt bit */
            R_CIO_CANFD_PRV_WriteReg(dev_inst->BaseAddr, R_CIO_CANFD_RFSTS(ridx),
                                        sts & ~R_CIO_CANFD_RFSTS_RFIF);

        }
        /* Enable Rx FIFO interrupts */
        R_CIO_CANFD_PRV_SetBit(dev_inst->BaseAddr, R_CIO_CANFD_RFCC(ridx),
                               R_CIO_CANFD_RFCC_RFIE);

    }
    return 0;
}

/*******************************************************************************
   Function: loc_ChannelIrqThread

   CANFD Channek IRQ thread.

   Parameters:
   Arg        - Thread parameter

   Returns:
   void*
 */

static void *loc_ChannelIrqThread(osal_device_handle_t device_handle, uint64_t irq_channel, void* Arg)
{
    int32_t x;
    uint32_t sts;
    uint32_t channel;
    uint32_t cerfl;
    uint32_t txerr;
    uint32_t rxerr;
    r_cio_canfd_prv_DevInst_t   *inst;
    r_cio_canfd_prv_ChInst_t    *ch_inst;

    inst = (r_cio_canfd_prv_DevInst_t *)Arg;

    R_CIO_CANFD_PRV_FOR_EACH_SET_BIT(channel, inst->ChannelMask, R_CIO_CANFD_NUM_CHANNELS) {

        ch_inst = R_CIO_CANFD_PRV_FindInstance(channel);
        /* Channel error interrupts */
        cerfl = R_CIO_CANFD_PRV_ReadReg(inst->BaseAddr, R_CIO_CANFD_CERFL(channel));
        sts = R_CIO_CANFD_PRV_ReadReg(inst->BaseAddr, R_CIO_CANFD_CSTS(channel));
        txerr = R_CIO_CANFD_CSTS_TECCNT(sts);
        rxerr = R_CIO_CANFD_CSTS_RECCNT(sts);
        if (0 != R_CIO_CANFD_CERFL_ERR(cerfl)) {
            /* handle Channel errors */
            R_CIO_CANFD_PRV_ChannelErr(ch_inst, cerfl);
            ch_inst->Error++;
        }
        /* Handle transition from error to normal states */
        if ((R_CIO_CANFD_MAX_ERR_ACTIVE > txerr)  && (R_CIO_CANFD_MAX_ERR_ACTIVE > rxerr)) {
            ch_inst->State = R_CIO_CANFD_STATE_ERROR_ACTIVE;
        } else if ((R_CIO_CANFD_MAX_ERR_WARNING > txerr) && (R_CIO_CANFD_MAX_ERR_WARNING > rxerr)) {
            ch_inst->State = R_CIO_CANFD_STATE_ERROR_WARNING;
        }
        /* Handle Tx interrupts */
        sts = R_CIO_CANFD_PRV_ReadReg(inst->BaseAddr, R_CIO_CANFD_CFSTS(channel, R_CIO_CANFD_CFFIFO_IDX));
        if (0 != (sts & R_CIO_CANFD_CFSTS_CFTXIF)) {
            x = loc_TxIrq(ch_inst);
            if (0 != x) {
                ch_inst->Error |= 0x80000000;
            }
            /* Clear interrupt bit */
            R_CIO_CANFD_PRV_WriteReg(inst->BaseAddr,
                                     R_CIO_CANFD_CFSTS(channel, R_CIO_CANFD_CFFIFO_IDX),
                                     sts & ~R_CIO_CANFD_CFSTS_CFTXIF);
        }
    }
    return 0;
}

/*******************************************************************************
   Section: Global (private)  Functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_EnableChannelInterrupts
 */
void R_CIO_CANFD_PRV_EnableChannelInterrupts(r_cio_canfd_prv_ChInst_t *Inst)
{
    uint32_t ctr;

    /* Clear any stray error flags */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CERFL(Inst->Ch), 0);

    /* Channel interrupts setup */
    ctr = (R_CIO_CANFD_CCTR_TAIE |
           R_CIO_CANFD_CCTR_ALIE | R_CIO_CANFD_CCTR_BLIE |
           R_CIO_CANFD_CCTR_OLIE | R_CIO_CANFD_CCTR_BORIE |
           R_CIO_CANFD_CCTR_BOEIE | R_CIO_CANFD_CCTR_EPIE |
           R_CIO_CANFD_CCTR_EWIE | R_CIO_CANFD_CCTR_BEIE);

    R_CIO_CANFD_PRV_SetBit(Inst->BaseAddr, R_CIO_CANFD_CCTR(Inst->Ch), ctr);
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_DisableChannelInterrupts
 */
void R_CIO_CANFD_PRV_DisableChannelInterrupts(r_cio_canfd_prv_ChInst_t *Inst)
{
    uint32_t ctr;

    ctr = (R_CIO_CANFD_CCTR_TAIE |
           R_CIO_CANFD_CCTR_ALIE | R_CIO_CANFD_CCTR_BLIE |
           R_CIO_CANFD_CCTR_OLIE | R_CIO_CANFD_CCTR_BORIE |
           R_CIO_CANFD_CCTR_BOEIE | R_CIO_CANFD_CCTR_EPIE |
           R_CIO_CANFD_CCTR_EWIE | R_CIO_CANFD_CCTR_BEIE);
    R_CIO_CANFD_PRV_ClearBit(Inst->BaseAddr, R_CIO_CANFD_CCTR(Inst->Ch), ctr);

    /* Clear any stray error flags */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, R_CIO_CANFD_CERFL(Inst->Ch), 0);
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_IrqInit
 */

int32_t R_CIO_CANFD_PRV_IrqInit(r_cio_canfd_prv_DevInst_t *Inst)
{
    size_t irqChannel;
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    Inst->device_handle = osal_devhandle_canfd;
    osal_ret = R_OSAL_InterruptGetNumOfIrqChannels(Inst->device_handle, &irqChannel);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqInit R_OSAL_InterruptGetNumOfIrqChannels");
        R_PRINT_Log(" failed(%d), irqChannel(%d)\r\n", osal_ret, irqChannel);
    }
    if(irqChannel==CIO_CANFD_DEVICE_CHANNEL){
        osal_ret = R_OSAL_InterruptRegisterIsr(Inst->device_handle, CIO_CANFD_GLOBAL, 0, (p_osal_isr_func_t)loc_GlobalIrqThread, (void*)Inst);
        if(OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqInit R_OSAL_InterruptRegisterIsr GLOBAL failed(%d)\r\n",
                        osal_ret);
        }
        osal_ret = R_OSAL_InterruptEnableIsr(Inst->device_handle, CIO_CANFD_GLOBAL);
        if(OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqInit R_OSAL_InterruptEnableIsr GLOBAL failed(%d)\r\n",
                        osal_ret);
        }

        osal_ret = R_OSAL_InterruptRegisterIsr(Inst->device_handle, CIO_CANFD_CHANNEL, 0, (p_osal_isr_func_t)loc_ChannelIrqThread, (void*)Inst);
        if(OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqInit R_OSAL_InterruptRegisterIsr CHANNEL failed(%d)\r\n",
                        osal_ret);
        }
        osal_ret = R_OSAL_InterruptEnableIsr(Inst->device_handle, CIO_CANFD_CHANNEL);
        if(OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqInit R_OSAL_InterruptEnableIsr CHANNEL failed(%d)\r\n",
                        osal_ret);
        }
    }else{
        R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqInit irqChannel not DEVICE_CHANNEL(%d)\r\n", irqChannel);
        return -1;
    }

    loc_EnableGlobalInterrupts(Inst);

    return 0;
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_IrqDeInit
 */

int32_t R_CIO_CANFD_PRV_IrqDeInit(r_cio_canfd_prv_DevInst_t *Inst)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    osal_ret = R_OSAL_InterruptDisableIsr(Inst->device_handle, CIO_CANFD_GLOBAL);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqDeInit R_OSAL_InterruptDisableIsr GLOBAL failed(%d)\r\n",
                    osal_ret);
    }
    osal_ret = R_OSAL_InterruptDisableIsr(Inst->device_handle, CIO_CANFD_CHANNEL);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqDeInit R_OSAL_InterruptDisableIsr CHANNEL failed(%d)\r\n",
                    osal_ret);
    }

    loc_DisableGlobalInterrupts(Inst);

    osal_ret = R_OSAL_InterruptUnregisterIsr(Inst->device_handle, CIO_CANFD_GLOBAL, (p_osal_isr_func_t)loc_GlobalIrqThread);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqDeInit R_OSAL_InterruptUnregisterIsr GLOBAL failed(%d)\r\n",
                    osal_ret);
    }
    osal_ret = R_OSAL_InterruptUnregisterIsr(Inst->device_handle, CIO_CANFD_CHANNEL, (p_osal_isr_func_t)loc_ChannelIrqThread);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioCnafdIrq]: R_CIO_CANFD_PRV_IrqDeInit R_OSAL_InterruptUnregisterIsr CHANNEL failed(%d)\r\n",
                    osal_ret);
    }

    return 0;
}
