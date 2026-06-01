/*************************************************************************************************************
* cio_canfd_connect_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup group2_canfd CAN-FD Driver Connect Functions
 * @ingroup cio_canfd_driver
 *
 * @brief Methods used by the CIO Server to initiate or finish communicatrion with the
 * CAN-FD module
 *
 * Functions which a the CIO Server will call in order to initiate a connection
 * between a user applcation and the CAN-FD macro.
 *
 * @{
 */

#ifndef R_CIO_CANFD_CONNECT_H
#define  R_CIO_CANFD_CONNECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
   Global (private) functions
 */

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_Open
 */
/**
 * @brief CANFD internal Open function.
 *
 * Starts one CANFD channel operation.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] Par1       - Optional parameter (unused)
 * @param[in] Par2       - Optional parameter (unused)
 * @param[in] Par3       - optional parameter (unused)
 *
 * @return == 0 - OK
 * @return != 0 - NG
 */
int R_CIO_CANFD_PRV_Open(size_t Addr, void *Par1, void *Par2, void *Par3);

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_Close
 */
/**
 * @brief CANFD internal Close function.
 *
 * Stops one CANFD channel operation.
 *
 * When R_CIO_CANFD_PRV_Close is called, the processing will be interrupted
 * if there is a processing request immediately before.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] Par1       - Optional parameter (unused)
 * @param[in] Par2       - Optional parameter (unused)
 * @param[in] Par3       - optional parameter (unused)
 *
 * @return != -1 - OK
 * @return == -1 - NG
 */
int R_CIO_CANFD_PRV_Close(size_t Addr, void *Par1, void *Par2, void *Par3);


#ifdef __cplusplus
}
#endif

#endif
/** @} */
