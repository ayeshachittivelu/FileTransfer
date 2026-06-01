/*************************************************************************************************************
* cio_can
* Copyright (c) [2019-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@addtogroup cio_can CIO CAN User interaction
 * @ingroup cio_bridge
 *
 * @brief Type defintions neccessary to for the user application to interact with the CIO CAN driver.
 *
 * Common structure (for the application and CIO) defining a CAN message and a
 * list of commands for the CIO CAN IoCtl function.
 *
 *
 * @{
 */


#ifndef R_CIO_CAN_H
#define R_CIO_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @def R_CAN_MSG_MAX_BYTES
 * Maximum allowed length (in bytes) for the CAN message data
 */
#define R_CAN_MSG_MAX_BYTES        64

/**@struct r_cio_can_Msg_t
 * @brief CAN Message definition
 */
typedef struct {
    uint32_t Flags;                     /**< Frame flags */
    uint32_t Id;                        /**< Message ID */
    uint16_t DataLen;                   /**< Actual message data length (in bytes) */
    uint8_t Data[R_CAN_MSG_MAX_BYTES];  /**< Message data */
} r_cio_can_Msg_t;

/**@enum r_cio_can_Ctl_t
 * @brief CIO CAN Controll elements
 *
 * The list of functions and settings available for IOCTL
 */
typedef enum {
    R_CIO_CAN_CTL_NULL,                 /**< NULL element */
    R_CIO_CAN_REGISTER_RX_EVENT,        /**< Set a RX "interrupt callback" (semaphore) */
    R_CIO_CAN_CTL_LAST                  /**< Delimiter, never change */
} r_cio_can_Ctl_t;

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_API_H */

/** @} */