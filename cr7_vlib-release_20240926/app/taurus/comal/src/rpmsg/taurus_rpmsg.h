/*************************************************************************************************************
* taurus_rpmsg.h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef _TAURUS_RPMSG_H_
#define _TAURUS_RPMSG_H_

#include "taurus_virtqueue.h"
#include "virtio_ring.h"
#include "remoteproc.h"

/* Maximum size of remote service name */
#define RPMSG_NAME_SIZE         32

/* Maximum size of the buffers allocated by the host.
 * Note: the actual usable size for a rpmsg message is
 * (RPMSG_BUFFER_SIZE - sizeof(struct taurus_rpmsg_hdr))
 */
#define RPMSG_BUFFER_SIZE       512

enum taurus_rpmsg_ns_flags {
    RPMSG_NS_CREATE  = 0,
    RPMSG_NS_DESTROY = 1
};

/* Every message sent(/received) on the rpmsg bus begins with this
 * header */
struct taurus_rpmsg_hdr {
    uint32_t src;
    uint32_t dst;
    uint32_t reserved;
    uint16_t len;
    uint16_t flags;
    uint8_t data[0];
};

struct taurus_rpmsg_dev {
    struct taurus_virtqueue svq;
    struct taurus_virtqueue rvq;
};

int16_t taurus_rpmsg_init(struct taurus_rpmsg_dev *dev,
                          struct fw_rsc_vdev_vring *tx_vring,
                          struct fw_rsc_vdev_vring *rx_vring,
                          int (*notify)(struct taurus_virtqueue *vq));

int16_t taurus_rpmsg_receive(struct taurus_rpmsg_dev *dev,
                             uint16_t *src,
                             uint16_t *dst,
                             void *data,
                             uint16_t *len);

int16_t taurus_rpmsg_send(struct taurus_rpmsg_dev *dev,
                          uint32_t src,
                          uint32_t dst,
                          const void *data,
                          uint16_t len);

int16_t taurus_rpmsg_channel(enum taurus_rpmsg_ns_flags flags,
                             struct taurus_rpmsg_dev *dev,
                             const char *name,
                             int32_t port);

#endif /* _TAURUS_RPMSG_H_ */
