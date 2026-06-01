/*************************************************************************************************************
* cio_canfd
* Copyright (c) [2019-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/



/**@defgroup group1_canfd CIO CANFD Driver Declaration
 * @ingroup cio_canfd_driver
 *
 * @brief Bridge for the CIO server.
 *
 * Declaration of the CAN-FD driver constant. This is the only information that needs to be exposed to the CIO server

 * @{
 */

#ifndef R_CIO_CANFD_H
#define  R_CIO_CANFD_H

#ifdef __cplusplus
extern "C" {
#endif

/** R_CANFD_Driver
 * @brief This is the structure providing access to the CAN-FD driver for the CIO server.
 */
extern const r_cio_Driver_t R_CIO_CANFD_Driver;

#ifdef __cplusplus
}
#endif

#endif

/** @} */
