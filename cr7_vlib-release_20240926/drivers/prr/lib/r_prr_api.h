/*************************************************************************************************************
* prrr_api_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef _R_PRR_API_H_
#define _R_PRR_API_H_

#include <stdint.h>

/**
 * \defgroup  PRRgroup   PRR driver
 * @ingroup drivers
 * @brief Product Read Register (PRR) driver interface
 * @{
 * @}
 */

/**
 * \defgroup PRRtypes   Data Types
 * @ingroup PRRgroup
 * @{
 */

/**
 * List of the supported R-Car devices
 */
typedef enum {
    R_PRR_RCARH3,       /**< RCar H3 */
    R_PRR_RCARM3W,      /**< RCar M3-W */
    R_PRR_RCARM3N,      /**< RCar M3-N */
    R_PRR_RCARD3,       /**< RCar D3 */
    R_PRR_RCARE3,       /**< Rcar E3 */
    R_PRR_RCARV3M,      /**< Rcar V3M */
    R_PRR_RCARV3H,      /**< Rcar V3H */
    R_PRR_RCARLAST      /**< Delimiter */
} r_prr_DevName_t;

typedef enum {
    R_PRR_REV10,        /*   Ver.1.0  */
    R_PRR_REV11,        /*   Ver.1.1  */
    R_PRR_REV13,        /*   Ver.1.3  */
    R_PRR_REV20,        /*   Ver.2.0  */
    R_PRR_REV21,        /*   Ver.2.1  */
    R_PRR_REV30,        /*   Ver.3.0  */
    R_PRR_REVLAST       /**< Delimiter */
} r_prr_Rev_t;

/** @} types */

/**
 * \defgroup PRRapi   API
 * @ingroup PRRgroup
 * @{
 */

/**
 * Return the R-Car device type currently in use.
 *
 * @retval      See @ref r_prr_DevName_t
 */
r_prr_DevName_t R_PRR_GetDevice(void);

/**
 * Return the R-Car device revision currently in use.
 *
 * @retval      See @ref r_prr_Rev_t
 */
r_prr_Rev_t R_PRR_GetRevision(void);

/** @} api */

#endif /* _R_PRR_API_H_ */
