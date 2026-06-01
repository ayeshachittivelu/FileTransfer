/*************************************************************************************************************
* taurus_rpmsg.c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "taurus_rpmsg.h"
#include "taurus_memory_barrier.h"
#include "r_print_api.h"

/* Address 53 is reserved for advertising remote services */
#define RPMSG_NS_ADDR           (53)

struct taurus_rpmsg_ns_msg {
    char name[RPMSG_NAME_SIZE];
    uint32_t addr;
    uint32_t flags;
};

static void *my_memcpy(void *dst, const void *src, unsigned long n)
{
    uint8_t *_dst, *_src;

    _dst = (uint8_t *)dst;
    _src = (uint8_t *)src;

    while (n--)
        *(_dst++) = *(_src++);

    return dst;
}

int16_t taurus_rpmsg_init(struct taurus_rpmsg_dev *dev,
                          struct fw_rsc_vdev_vring *tx_vring,
                          struct fw_rsc_vdev_vring *rx_vring,
                          int (*notify)(struct taurus_virtqueue *vq))
{
    struct taurus_vring_alloc_info vr_info = { 0 };

    vr_info.pages = (void *)tx_vring->da;
    vr_info.align = tx_vring->align;
    vr_info.num_descs = tx_vring->num;
    taurus_virtqueue_init(&dev->svq, tx_vring->notifyid, &vr_info, notify);

    vr_info.pages = (void *)rx_vring->da;
    vr_info.align = rx_vring->align;
    vr_info.num_descs = rx_vring->num;
    taurus_virtqueue_init(&dev->rvq, rx_vring->notifyid, &vr_info, notify);

    return 0;
}

int16_t taurus_rpmsg_send(struct taurus_rpmsg_dev *dev,
                          uint32_t src,
                          uint32_t dst,
                          const void *data,
                          uint16_t len)
{
    struct taurus_rpmsg_hdr *msg;
    uint32_t msg_len;
    int16_t head;
    struct taurus_virtqueue *virtqueue;

    /* Check the length of the payload */
    if (len > (RPMSG_BUFFER_SIZE - sizeof(struct taurus_rpmsg_hdr))) {
        R_PRINT_Log("[TaurusRpmsg]: taurus_rpmsg_send length of the payload(%d) failed.\r\n", len);
        return -1;
    }

    virtqueue = &dev->svq;

    /* Get an available buffer */
    msg = taurus_virtqueue_get_avail_buf(virtqueue, &head, &msg_len);
    if (!msg) {
        /* No available buffer */
        R_PRINT_Log("[TaurusRpmsg]: taurus_virtqueue_get_avail_buf failed.\r\n");
        return -2;
    }

    /* Copy local data buffer to the descriptor buffer address */
    (void)my_memcpy(msg->data, data, len);
    msg->len = len;
    msg->dst = dst;
    msg->src = src;
    msg->flags = 0;
    msg->reserved = 0;
    /* Clean message header */
    taurus_memory_cache_flush((void *)msg, sizeof(*msg));
    /* Clean message body */
    taurus_memory_cache_flush((void *)msg->data, len);


    /* Return the used buffer */
    if (taurus_virtqueue_add_used_buf(virtqueue, head, msg_len) < 0) {
        R_PRINT_Log("[TaurusRpmsg]: taurus_virtqueue_add_used_buf failed.\r\n");
        return -3;
    }

    /* Notify the other side */
    if (virtqueue->notify)
        virtqueue->notify(virtqueue);

    return 0;
}

int16_t taurus_rpmsg_receive(struct taurus_rpmsg_dev *dev,
                             uint16_t *src,
                             uint16_t *dst,
                             void *data,
                             uint16_t *len)
{
    int16_t head;
    struct taurus_rpmsg_hdr *msg;
    uint32_t msg_len;
    struct taurus_virtqueue *virtqueue;

    virtqueue = &dev->rvq;

    /* Get an available buffer */
    msg = taurus_virtqueue_get_avail_buf(virtqueue, &head, &msg_len);
    if (!msg) {
        R_PRINT_Log("[TaurusRpmsg]: taurus_virtqueue_get_avail_buf failed.\r\n");
        return -2;
    }

    taurus_memory_cache_invalidate(msg, sizeof(*msg));
    taurus_memory_cache_invalidate((void *)msg->data, msg_len);

    /* Copy the message payload to the local data buffer provided */
    (void)my_memcpy(data, msg->data, msg->len);
    *src = msg->src;
    *dst = msg->dst;
    *len = msg->len;

    /* Add the used buffer */
    if (taurus_virtqueue_add_used_buf(virtqueue, head, msg_len) < 0) {
        R_PRINT_Log("[TaurusRpmsg]: taurus_virtqueue_add_used_buf failed.\r\n");
        return -3;
    }

    /* Notify the other side */
    if (virtqueue->notify)
        virtqueue->notify(virtqueue);

    return 0;
}

int16_t taurus_rpmsg_channel(enum taurus_rpmsg_ns_flags flags,
                             struct taurus_rpmsg_dev *dev,
                             const char *name,
                             int32_t port)
{
    struct taurus_rpmsg_ns_msg ns_msg;
    uint8_t i;

    for (i = 0; i < RPMSG_NAME_SIZE; i++) {
        ns_msg.name[i] = name[i];
    }
    ns_msg.addr = port;
    ns_msg.flags = flags;

    return taurus_rpmsg_send(dev, port, RPMSG_NS_ADDR, &ns_msg, sizeof(ns_msg));
}
