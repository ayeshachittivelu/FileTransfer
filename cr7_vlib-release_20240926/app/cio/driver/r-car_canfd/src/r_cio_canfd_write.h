/*************************************************************************************************************
* cio_canfd_write_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup canfdwrite CAN-FD Driver data transmit
 * @ingroup cio_canfd_driver
 *
 * @brief Write data to the CAN-FD drivers's trasmit buffer
 *
 * @{
 */


#ifndef R_CIO_CANFD_WRITE_H
#define  R_CIO_CANFD_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
   Section: Global private functions
 */

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_Write
 */
/**
 * @brief Write to the message buffer assigned to the given instance
 * and initiate transmission
 *
 * R_CIO_CANFD_PRV_Write runs, and if the return value is OK, the channel interrupt occurs asynchronously.
 * If run R_CIO_CANFD_PRV_Close before receive the channel interrupt, the transmission will not be completed.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] Par1       - Buffer address (source)
 * @param[in] Par2       - length of data to write.
                           If 0 is specified, it is not an error.
                           Also, the write data is returned as 0.
 * @param[in] Par3       - optional parameter (unused)
 *
 * @return >= 0 - length of write (handled) data
 * @return <  0 - NG
 */
ssize_t R_CIO_CANFD_PRV_Write(size_t Addr, void *Par1, void *Par2, void *Par3);

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_PutData
 */
/**
 * @brief Write CAN Frame data to the CAN-FD (FIFO) register. Used by the
 * R_CIO_CANFD_PRV_TransmitFrame function.
 *
 * @param[in] Inst       - Pointer to the CANFD channel instance
 * @param[in] Frame      - The CAN frame holding the data
 * @param[in] Offset     - offset for the appropriate TX register(s)
 *
 * @return void
 */
void R_CIO_CANFD_PRV_PutData(r_cio_canfd_prv_ChInst_t *Inst, r_cio_can_Msg_t *Frame,
                             uint32_t Offset);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_TransmitFrame
 */
/**
 * @brief  Prepare and Write frame data from from the instance's TX buffer to the CAN-FD registes.
 *
 * The data is transformed into CAN conform format and written to the the corresponding CAN-FD registers.
 * The can trasnmission is initiated by this function.
 *
 * @param[in] Inst       - Pointer to the CANFD Channel instance
 *
 * @return void
 *
 */
void R_CIO_CANFD_PRV_TransmitFrame(r_cio_canfd_prv_ChInst_t *Inst);


#ifdef __cplusplus
}
#endif

#endif

/** @} */
