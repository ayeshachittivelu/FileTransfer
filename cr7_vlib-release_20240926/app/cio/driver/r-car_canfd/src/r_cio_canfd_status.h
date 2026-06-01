/*************************************************************************************************************
* cio_canfd_status_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/


#ifndef R_CIO_CANFD_STATUS_H
#define R_CIO_CANFD_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************************************************
   section: Global Defines
 */


/*******************************************************************************
   Section: Global private functions
 */


/*******************************************************************************
   Function Name: R_CIO_CANFD_PRV_Status
 */
/**
 * @brief  Return the current (error)status for the given instance
 * After call this function, the error status will be cleared.
 *
 * @param[in] Addr       - Channel number of the given CANFD instance
 * @param[in] Par1       - Optional parameter (unused)
 * @param[in] Par2       - Optional parameter (unused)
 * @param[in] Par3       - optional parameter (unused)
 *
 * @return >=  0 - Current error status
 * @return != -1 - NG
 */
ssize_t R_CIO_CANFD_PRV_Status(size_t Addr, void *Par1, void *Par2, void *Par3);


#ifdef __cplusplus
}
#endif

#endif /* R_CIO_CANFD_STATUS_H */
