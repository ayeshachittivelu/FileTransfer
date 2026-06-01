/*************************************************************************************************************
* resource_table_linux.h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef _RESOURCE_TABLE_H_
#define _RESOURCE_TABLE_H_

#include "remoteproc.h"

#define MAX_NUM_RES_ENTRIES 16

struct cr7_resource_table {
    struct resource_table base;
    uint32_t offsets[MAX_NUM_RES_ENTRIES];

    struct fw_rsc_hdr rpmsg1_header;
    struct fw_rsc_vdev rpmsg1;
    struct fw_rsc_vdev_vring rpmsg1_vring0;
    struct fw_rsc_vdev_vring rpmsg1_vring1;
};

#endif /* _RESOURCE_TABLE_H_ */
