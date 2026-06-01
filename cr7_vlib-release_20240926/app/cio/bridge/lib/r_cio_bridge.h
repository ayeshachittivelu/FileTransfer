/*************************************************************************************************************
* cio_bridge
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/


#ifndef R_CIO_BRIDGE_H
#define R_CIO_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rcar-xos/osal/r_osal.h"

/**@addtogroup cio_bridge CIO Bridge
 * @ingroup cio
 * @{
 * @}
 */

/**
 * \defgroup cio_bridge_constants   Constants
 * @ingroup cio_bridge
 * @{
 */

/**
   @def R_CIO_QUEUE_MSG_NAME
 */
#define R_CIO_QUEUE_MSG_NAME "/CIO_MSG"
/**
   @def R_CIO_QUEUE_RET_NAME
 */
#define R_CIO_QUEUE_RET_NAME "/CIO_RET"
/**
   @def R_CIO_MAX_DEVICENAME
 */
#define R_CIO_MAX_DEVICENAME 0x100

/** @} Constants */
/**
 * \defgroup cio_bridge_types   Data Types
 * @ingroup cio_bridge
 * @{
 */

/*
 * List of the CIO commands
 */
typedef enum {
    R_CIO_CMD_NOP,      /**< No operation */
    R_CIO_CMD_OPEN,     /**< Open */
    R_CIO_CMD_CLOSE,    /**< Close */
    R_CIO_CMD_READ,     /**< Read */
    R_CIO_CMD_WRITE,    /**< Write */
    R_CIO_CMD_IOCTL,    /**< I/O Control */
    R_CIO_CMD_STATUS,   /**< Status */
    R_CIO_CMD_EXIT      /**< Exit */
} r_cio_Cmd_t;


/*******************************************************************************
   @enum r_cio_Result_t
   @brief CIO command.
 */

typedef enum {
    R_CIO_RES_ACK,
    R_CIO_RES_NACK,
    R_CIO_RES_COMPLETE,
    R_CIO_RES_ERROR
} r_cio_Result_t;


/**
   @brief CIO command message.
 */
typedef struct {
    int Id;               /**< Transaction Id */
    r_cio_Cmd_t Cmd;       /**< Command (Open, Read, Write, Close, IoCtl) */
    void             *Par1; /**< Auxiliary parameter, typically buffer*/
    void             *Par2; /**< Auxiliary parameter, typically size */
    void             *Par3; /**< Auxiliary parameter */
} r_cio_CmdMsg_t;


/**
   @struct r_cio_ResultMsg_t
   @brief CIO command message.
 */
typedef struct {
    int Id;             /**< Transaction Id */
    r_cio_Result_t Result;  /**< Result (ACK, NAK, ERR) */
    void           *Aux; /**< Auxiliary result parameter (e.g. written data lentgh) */
} r_cio_ResultMsg_t;


/**@struct r_cio_Driver_t
   @brief CIO driver structure.
 */
typedef struct {
    char *Name; /**< Driver name */
    int (*Init)(size_t Addr, int IrqNum); /**< Init function called during CIO server start */
    int (*DeInit)(size_t Addr); /**< DeInit function called during CIO server ending */
    int (*Open)(size_t Addr, void *Par1, void *Par2, void *Par3); /**< Driver command */
    int (*Close)(size_t Addr, void *Par1, void *Par2, void *Par3); /**< Driver command */
    ssize_t (*Read)(size_t Addr, void *Par1, void *Par2, void *Par3); /**< Driver command */
    ssize_t (*Write)(size_t Addr, void *Par1, void *Par2, void *Par3); /**< Driver command */
    ssize_t (*IoCtl)(size_t Addr, void *Par1, void *Par2, void *Par3); /**< Driver command */
    ssize_t (*Status)(size_t Addr, void *Par1, void *Par2, void *Par3); /**< Driver command */
} r_cio_Driver_t;


typedef struct {
    char                  *Name;
    size_t BaseAddr;
    int IrqNum;
    const r_cio_Driver_t  *Driver;
} r_cio_DeviceEntry_t;

/** @} Data Types*/

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_BRIDGE_H */
