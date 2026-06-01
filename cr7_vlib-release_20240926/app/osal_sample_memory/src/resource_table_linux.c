/*
****************************************************************************
  Copyright (c) [2021-2022] Renesas Electronics Corporation
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
****************************************************************************
*/
/**********************************************************************************************************************
 * File Name    : resource_table_linux.c
 * Version      : 1.0
 * Description  : List remote processor firmware information that will be accessed by Linux kernel.
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "resource_table.h"
#include "virtio_ids.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#define NUM_RES_ENTRIES (1)

#define CR7_RPMSG1_ID (0)

#define CR7_RPMSG1_VRING0_DA   (0)
#define CR7_RPMSG1_VRING0_SIZE (512)
#define CR7_RPMSG1_VRING0_ID   (1)
#define CR7_RPMSG1_VRING0_PA   (0)

#define CR7_RPMSG1_VRING1_DA   (0)
#define CR7_RPMSG1_VRING1_SIZE (512)
#define CR7_RPMSG1_VRING1_ID   (2)
#define CR7_RPMSG1_VRING1_PA   (0)

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/
extern unsigned int _LDS_START_ADDR; /* set in the linker script */
extern unsigned int _LDS_PROG_SIZE;  /* set in the linker script */

/* clang-format off */
__attribute__((section(".resource_table")))
__attribute__((aligned(4096)))
/* clang-format on */

#ifdef INTEGRITY_GUEST
struct cr7_resource_table resource_table;
#else
struct cr7_resource_table resource_table= {
    /* struct resource_table base */
    {
        1,
        NUM_RES_ENTRIES,
        {0, 0},
    },
    /* u32 offsets[NUM_RES_ENTRIES] */
    {
        offsetof(struct cr7_resource_table, rpmsg1_header),
    },
    /* struct fw_rsc_hdr rpmsg1_header */
    {
        RSC_VDEV,
    },
    /* struct fw_rsc_vdev rpmsg1 */
    {
        VIRTIO_ID_RPMSG,
        CR7_RPMSG1_ID, /* notifyid */
        1,             /* fw features, VIRTIO_RPMSG_F_NS */
        0,             /* negotiated features place holder */
        0,             /* config_len */
        0,             /* status place holder */
        2,             /* num_of_vrings */
        {0},           /* reserved */
    },
    /* struct fw_rsc_vdev_vring rpmsg1_vring0 RX */
    {
        CR7_RPMSG1_VRING0_DA,
        4096, /* align */
        CR7_RPMSG1_VRING0_SIZE,
        CR7_RPMSG1_VRING0_ID,
        CR7_RPMSG1_VRING0_PA,
    },
    /* struct fw_rsc_vdev_vring rpmsg1_vring1 TX */
    {
        CR7_RPMSG1_VRING1_DA,
        4096, /* align */
        CR7_RPMSG1_VRING1_SIZE,
        CR7_RPMSG1_VRING1_ID,
        CR7_RPMSG1_VRING1_PA,
    },
};
#endif
