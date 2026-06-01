/*************************************************************************************************************
* cio_canfd_read_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup canfdread_canfd CAN-FD Driver data read
 * @ingroup cio_canfd_driver
 *
 * @brief Read from the CAN-FD drivers's receive buffer
 *
 * @{
 */



#ifndef R_CIO_CANFD_READ_H
#define  R_CIO_CANFD_READ_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
   Section: Global private functions
 */


/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_Read
 */
/**
 * @brief  Read from the message buffer assigned to the given instance
 *
 * Read from the CAN-FD driver's receive buffer.
 * The data will stored in the buffer given by Par1.
 * The lenghth of the data to read must be a multiple of the the defined
 * can message length <r_cio_can_Msg_t>
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] Par1       - Buffer address (destination)
 * @param[in] Par2       - length of data to read.
                           If 0 is specified, it is not an error.
                           Also, the read data is returned as 0.
 * @param[in] Par3       - optional parameter (unused)
 *
 * @return >= 0 - length of the read data
 * @return <  0 - NG
 */
ssize_t R_CIO_CANFD_PRV_Read(size_t Addr, void *Par1, void *Par2, void *Par3);


#ifdef __cplusplus
}
#endif

#endif

/** @} */
