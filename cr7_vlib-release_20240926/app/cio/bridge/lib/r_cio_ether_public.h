/*************************************************************************************************************
* cio_ether_public
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@addtogroup etherif CIO ETHER User interaction
 * @ingroup cio_bridge
 *
 * @brief Type defintions neccessary to for the user application to interact with the CIO ETHER driver.
 *
 * Common structure (for the application and CIO) defining a ETHER message and a
 * list of commands for the CIO ETHER IoCtl function.
 *
 *
 * @{
 */


#ifndef R_CIO_ETHER_PUBLIC_H
#define R_CIO_ETHER_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "rcar-xos/osal/r_osal.h"
#include "Eth_Types.h"

/**
 * @def R_ETHER_MSG_MAX_BYTES
 * Maximum allowed length (in bytes) for the ETHER message data
 */
#define R_ETHER_MSG_MAX_BYTES        64

/**@struct r_cio_ether_Msg_t
 * @brief ETHER Message definition
 */
typedef struct {
    uint8 CtrlIdx;
    Eth_ModeType CtrlMode;
}r_cio_ether_mode;

typedef struct {
    uint8 CtrlIdx;
    uint8 TrcvIdx;
    uint8 RegIdx;
    uint16 RegVal;
}r_cio_ether_mii;

typedef struct {
    uint8 CtrlIdx;
    uint8 Priority;
    Eth_BufIdxType* BufIdxPtr;
    uint8** BufPtr;
    uint16* LenBytePtr;
}r_cio_ether_tx_buff;

typedef struct {
    uint8 CtrlIdx;
    uint8* PhysAddrPtr;
}r_cio_ether_phys_addr;

typedef struct {
    uint8 CtrlIdx;
    uint8* PhysAddrPtr;
    Eth_FilterActionType Action;
}r_cio_ether_phys_addr_filter;

typedef struct {
    Std_VersionInfoType versionInfo;
} r_cio_ether_version_info;

typedef struct {
    uint8 CtrlIdx;
    uint8 BufIdx;
    Eth_TimeStampQualType timeQualPtr;
}r_cio_ether_current_time;

typedef struct {
    uint8 CtrlIdx;
    uint8 BufIdx;
}r_cio_enable_egress_time_stamp;

typedef struct {
    uint8 CtrlIdx;
    uint8 BufIdx;
    Eth_TimeStampQualType* timeQualPtr;
    Eth_TimeStampType* timeStampPtr;
}r_cio_ether_get_egress_time_stamp;

typedef struct {
    uint8 CtrlIdx;
    Eth_DataType* DataPtr;
    Eth_TimeStampQualType* timeQualPtr;
    Eth_TimeStampType* timeStampPtr;
}r_cio_ether_get_ingress_time_stamp;

typedef struct {
    uint8 CtrlIdx;
    Eth_TimeStampType* timeStampPtr;
}r_cio_ether_set_global_time;

typedef struct {
    uint8 CtrlIdx;
    Eth_TimeIntDiffType* timeOffsetPtr;
    Eth_RateRatioType* rateRatioPtr;
}r_cio_ether_set_correction_time;

typedef struct {
    uint8 CtrlIdx;
    uint8 CountValues;
    uint32 *DropCount;
}r_cio_ether_get_drop_count;

typedef struct {
    uint8 CtrlIdx;
    uint32 etherStats;
}r_cio_ether_get_ether_stats;

typedef struct {
    uint8 CtrlIdx;
    Eth_RxStatusType RxStatus;
}r_cio_ether_receive;

typedef struct {
    uint8 CtrlIdx;
    Eth_BufIdxType BufIdx;
    Eth_FrameType FrameType;
    boolean TxConfirmation;
    uint16 LenByte;
    uint8 *PhysAddrPtr;
}r_cio_ether_transmit;

/**@enum r_cio_ether_Ctl_t
 * @brief CIO ETHER Control elements
 *
 * The list of functions and settings available for IOCTL
 */
typedef enum {
    R_CIO_ETHER_IOC_SET_MODE,
    R_CIO_ETHER_IOC_GET_MODE,
    R_CIO_ETHER_IOC_WRITE_MII,
    R_CIO_ETHER_IOC_READ_MII,
    R_CIO_ETHER_PROVIDE_TX_BUFF,
    R_CIO_ETHER_TX_CONFIRMATION,
    R_CIO_ETHER_GET_VERSION_INFO,
    R_CIO_ETHER_GET_PHYS_ADDR,
    R_CIO_ETHER_SET_PHYS_ADDR,
    R_CIO_ETHER_UPDATE_PHYS_ADDR_FILTER,
    R_CIO_ETHER_GET_CURRENT_TIME,
    R_CIO_ETHER_ENABLE_EGRESS_TIME_STAMP,
    R_CIO_ETHER_GET_EGRESS_TIME_STAMP,
    R_CIO_ETHER_GET_INGRESS_TIME_STAMP,
    R_CIO_ETHER_SET_GLOBAL_TIME,
    R_CIO_ETHER_SET_CORRECTION_TIME,
    R_CIO_ETHER_GET_DROP_COUNT,
    R_CIO_ETHER_GET_ETHER_STATS,
    R_CIO_ETHER_RECEIVE,
    R_CIO_ETHER_TRANSMIT,
    R_CIO_ETHER_REGISTER_RX_EVENT
} r_cio_ether_Ctl_t;


osal_mq_handle_t Ether_mq_handle_received;

typedef struct {
    /* uint8 CtrlIdx;           */
    /* Eth_FrameType FrameType; */
    /* boolean IsBroadcast;     */
    /* uint8* PhysAddrPtr;      */
    Eth_DataType* DataPtr;
    uint16 LenByte;
} r_ether_mq_receive_t;

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_ETHER_PUBLIC_H */

/** @} */