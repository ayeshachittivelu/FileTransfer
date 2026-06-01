/*************************************************************************************************************
* cio_canfd_ioctl_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup group4_canfd CAN-FD Driver Configuration Function
 * @ingroup cio_canfd_driver
 *
 * @brief Provide a CIO interface to pass different configuartion parameter to the driver
 *
 * @{
 */



#ifndef R_CIO_CANFD_IOCTL_H
#define  R_CIO_CANFD_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_IoCtl
 */
/**
 * @brief CANFD settings function.
 *
 * Provide a CIO interface to pass different configuration parameter to the driver.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param [in] Par1       - Set settings/functions. See @ref r_cio_can_Ctl_t.
 * @param [in] Par2       - Set message queue handle. See @ref osal_mq_handle_t.
 * @param [in] Par3       - Optional parameter (unused)
 *
 * @return == 0 - OK
 * @return != 0 - NG
 */
ssize_t R_CIO_CANFD_PRV_IoCtl(size_t Addr, void *Par1, void *Par2, void *Par3);


#ifdef __cplusplus
}
#endif

#endif

/** @} */