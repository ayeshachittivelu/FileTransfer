/*************************************************************************************************************
* taurus_comch_protocol
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef R_TAURUS_COMCH_PROTOCOL_H
#define R_TAURUS_COMCH_PROTOCOL_H
#include "r_taurus_bridge.h"
#include "r_taurus_protocol_ids.h"

#ifndef __packed
#define __packed       __attribute__((__packed__))
#endif

#define COMCH_PROTOCOL_EVENT_PKT_AVAIL               ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0x000000)


#define COMCH_PROTOCOL_OPEN                              ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xE00000)

struct taurus_comch_init_in {
    uint64_t cookie;
    char AppName[64];
} __packed;
struct taurus_comch_init_out {
    uint64_t cookie;
    uint64_t res;
    int Id;
} __packed;

#define COMCH_PROTOCOL_IOC_MYFUNCTION1                    ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xF00000)

struct taurus_comch_myfunction1_in{
    uint64_t cookie;
    uint64_t parm1;
    uint64_t parm2;
} __packed;
struct taurus_comch_myfunction1_out {
    uint64_t cookie;
    uint64_t retvalue1;    
    uint64_t res;
} __packed;


#define COMCH_PROTOCOL_IOC_MYFUNCTION2                    ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xF00001)

struct taurus_comch_myfunction2_in{
    uint64_t cookie;
    uint64_t parm1;
    uint64_t parm2;
} __packed;
struct taurus_comch_myfunction2_out {
    uint64_t cookie;
    uint64_t retvalue1;    
    uint64_t res;
} __packed;




#define COMCH_PROTOCOL_CLOSE                             ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xE00001)

struct taurus_comch_close_in {
    uint64_t cookie;
} __packed;

struct taurus_comch_close_out {
    uint64_t cookie;
    uint64_t res;
} __packed;

#define COMCH_PROTOCOL_IOC_TRANSMIT                      ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xF0000B)
/* Write can transfer data via Data buffer pointer or internal buffer Message Buffer */

struct taurus_comch_transmit_in{
    uint64_t cookie;
    uint16_t type;
    uint16_t LenByte;
    uint32_t BufferAddress;
    char MessageBuffer[64];
} __packed;

struct taurus_comch_transmit_out{
    uint64_t cookie;
    uint64_t res;
} __packed;


#define COMCH_PROTOCOL_IOC_RECEIVE                       ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xF0000C)

struct taurus_comch_receive_in{
    uint64_t cookie;
    uint16_t type;    
} __packed;

struct taurus_comch_receive_out {
    uint64_t cookie;
    uint16_t LenByte;
    uint32_t BufferAddress;
    char MessageBuffer[64];    
    uint64_t res;
} __packed; 

#define COMCH_PROTOCOL_STATUS                            ((TAURUS_PROTOCOL_COMCH_ID << 24) | 0xF00009)

struct taurus_comch_get_comch_stats_in{
    uint64_t cookie;
    uint8_t CtrlIdx;
    /* uint32_t* comchStats; */
} __packed;

struct taurus_comch_get_comch_stats_out{
    uint64_t cookie;
    uint32_t comchStats;
    uint64_t res;
} __packed;


/*******************************************************/

struct taurus_comch_cmd_msg {
    R_TAURUS_CmdMsg_t hdr;
    uint32_t type;
    union {
        struct taurus_comch_init_in open;
        struct taurus_comch_myfunction1_in myfunction1;
	struct taurus_comch_myfunction2_in myfunction2;
        struct taurus_comch_close_in close;
        struct taurus_comch_transmit_in transmit;
        struct taurus_comch_receive_in receive;
        struct taurus_comch_get_comch_stats_in get_comch_stats;	
    } params;
};

struct taurus_comch_res_msg {
    R_TAURUS_ResultMsg_t hdr;
    uint32_t type;
    union {
        struct taurus_comch_init_out open;
        struct taurus_comch_myfunction1_out myfunction1;
        struct taurus_comch_myfunction2_out myfunction2;
        struct taurus_comch_close_out close;
        struct taurus_comch_transmit_out transmit;
        struct taurus_comch_receive_out receive;
        struct taurus_comch_get_comch_stats_out get_comch_stats;	
    } params;
};
#endif /* R_TAURUS_COMCH_PROTOCOL_H */
