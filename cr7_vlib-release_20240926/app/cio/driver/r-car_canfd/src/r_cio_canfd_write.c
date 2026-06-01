/*************************************************************************************************************
* cio_canfd_write_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Section: Includes
 */

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_bridge.h"
#include "r_cio_can.h"
#include "r_cio_canfd_main.h"
#include "r_cio_canfd_write.h"
#include "r_cio_canfd_helper.h"
#include "r_print_api.h"
#include <string.h>

/*******************************************************************************
   Section: Local Constants
 */

static const uint8_t loc_Len2DlcTbl[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8,     /* 0 - 8 */
                                          9, 9, 9, 9, /* 9 - 12 */
                                          10, 10, 10, 10, /* 13 - 16 */
                                          11, 11, 11, 11, /* 17 - 20 */
                                          12, 12, 12, 12, /* 21 - 24 */
                                          13, 13, 13, 13, 13, 13, 13, 13, /* 25 - 32 */
                                          14, 14, 14, 14, 14, 14, 14, 14, /* 33 - 40 */
                                          14, 14, 14, 14, 14, 14, 14, 14, /* 41 - 48 */
                                          15, 15, 15, 15, 15, 15, 15, 15, /* 49 - 56 */
                                          15, 15, 15, 15, 15, 15, 15, 15 }; /* 57 - 64 */

/*******************************************************************************
   Section: Local functions
 */

/* map the sanitized data length to an appropriate data length code */
uint8_t loc_Len2Dlc(uint8_t Len)
{
    /* sanity check */
    if (64 < Len) {
        return 0xF;
    }
    return loc_Len2DlcTbl[Len];
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_TransmitFrame
 */

void R_CIO_CANFD_PRV_TransmitFrame(r_cio_canfd_prv_ChInst_t *Inst)
{
    r_cio_can_Msg_t *frame = Inst->TxStrt;
    uint32_t status = 0;
    uint32_t id;
    uint32_t dlc;

    if (0 != (frame->Id & R_CIO_CANFD_EFF_FLAG)) {
        id = frame->Id & R_CIO_CANFD_EFF_MASK;
        id |= R_CIO_CANFD_CFID_CFIDE;
    } else {
        id = frame->Id & R_CIO_CANFD_SFF_MASK;
    }

    if (0 != (frame->Id & R_CIO_CANFD_RTR_FLAG)) {
        id |= R_CIO_CANFD_CFID_CFRTR;
    }
    dlc = R_CIO_CANFD_CFPTR_CFDLC(loc_Len2Dlc(frame->DataLen));

    if (1 == Inst->FdMode) {
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                 R_CIO_CANFD_F_CFID(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), id);
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                 R_CIO_CANFD_F_CFPTR(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), dlc);

        if (0 != (frame->Flags & R_CIO_CANFD_FDF)) {
            /* CAN-FD frame format */
            status |= R_CIO_CANFD_CFFDCSTS_CFFDF;
            if (0 != (frame->Flags & R_CIO_CANFD_BRS)) {
                status |= R_CIO_CANFD_CFFDCSTS_CFBRS;
            }
            if (Inst->State == R_CIO_CANFD_STATE_ERROR_PASSIVE) {
                status |= R_CIO_CANFD_CFFDCSTS_CFESI;
            }
        }

        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                 R_CIO_CANFD_F_CFFDCSTS(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), status);

        R_CIO_CANFD_PRV_PutData(Inst, frame,
                                R_CIO_CANFD_F_CFDF(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX, 0));
    } else {
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                 R_CIO_CANFD_C_CFID(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), id);
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                                 R_CIO_CANFD_C_CFPTR(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), dlc);
        R_CIO_CANFD_PRV_PutData(Inst, frame,
                                R_CIO_CANFD_C_CFDF(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX, 0));
    }

    /* Start Tx: Write 0xff to CFPC to increment the CPU-side
     * pointer for the Common FIFO
     */
    R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr,
                             R_CIO_CANFD_CFPCTR(Inst->Ch, R_CIO_CANFD_CFFIFO_IDX), 0xff);
}

/*******************************************************************************
   Section: Global private functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_PutData
 */
void R_CIO_CANFD_PRV_PutData(r_cio_canfd_prv_ChInst_t *Inst, r_cio_can_Msg_t *Frame,
                             uint32_t Offset)
{
    uint32_t i;
    uint32_t lwords;

    lwords = R_CIO_CANFD_PRV_ROUND_UP(Frame->DataLen, sizeof(uint32_t));
    for (i = 0; i < lwords; i++) {
        R_CIO_CANFD_PRV_WriteReg(Inst->BaseAddr, Offset + (i *  sizeof(uint32_t)),
                                 *((uint32_t *)Frame->Data + i));
    }
}


/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Write
 */

ssize_t R_CIO_CANFD_PRV_Write(size_t Addr, void *Par1, void *Par2, void *Par3)
{
    r_cio_canfd_prv_ChInst_t *inst;
    r_cio_can_Msg_t        *buf;
    r_cio_can_Msg_t        *nxt;
    ssize_t len;

    inst = R_CIO_CANFD_PRV_FindInstance(Addr);
    if (0 == inst) {
        R_PRINT_Log("[CioCanfdWrite]: R_CIO_CANFD_PRV_Write R_CIO_CANFD_PRV_FindInstance failed\n");
        return -1;
    }

    buf = Par1;
    /* Check the correct length */
    if (0 != ((ssize_t)Par2 % sizeof(r_cio_can_Msg_t))) {
        R_PRINT_Log("[CioCanfdWrite]: R_CIO_CANFD_PRV_Write Message length failed(%d)\n", (ssize_t)Par2);
        return -2;
    }
    len = (ssize_t)Par2 / sizeof(r_cio_can_Msg_t);

    /* copy buffer to transfer buffer */
    while (len > 0) {
        nxt = inst->TxStop + 1;
        if (nxt > inst->TxEob) {
            nxt = inst->TxBuf;
        }
        if (inst->TxStrt != nxt) {
            /*     *inst->TxStop = *buf */
            (void)memcpy((void *)inst->TxStop, (void *)buf, sizeof(r_cio_can_Msg_t));

            inst->TxStop = nxt;
            buf++;
            len--;

        } else {
            /* hit the end */
            break;
        }
    }
    /* Start transmition */
    if (inst->TxStrt != inst->TxStop) {
        nxt = inst->TxStrt + 1;
        if (nxt > inst->TxEob) {
            nxt = inst->TxBuf;
        }
        /* Now, send the first message out */
        R_CIO_CANFD_PRV_TransmitFrame(inst);

        /* Move DataQue pointer */
        inst->TxStrt = nxt;

        /* Some (all) data was processed */
        len = (ssize_t)Par2 - (len * sizeof(r_cio_can_Msg_t));
    }

    return len;
}
