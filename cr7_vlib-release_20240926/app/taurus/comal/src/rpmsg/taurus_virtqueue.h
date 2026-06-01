/*************************************************************************************************************
* taurus_virtqueue.h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef _TAURUS_VIRTQUEUE_H_
#define _TAURUS_VIRTQUEUE_H_

#include "virtio_ring.h"

struct taurus_virtqueue {
    /* virtqueue id */
    uint32_t id;

    /* last consumed descriptor in the available table */
    uint16_t last_avail_idx;

    /* function used to notify the other side when buffers have been
     * used */
    int (*notify)(struct taurus_virtqueue *vq);

    /* Actual memory layout for this queue */
    struct vring vring;
};

struct taurus_vring_alloc_info {
    /* address of the memory reserved for the vring */
    void *pages;

    /* alignment for struct vring_used in the memory layout */
    uint32_t align;

    /* number of descriptors in the vring */
    uint16_t num_descs;
};

void taurus_virtqueue_init(struct taurus_virtqueue *vq,
                           uint32_t id,
                           const struct taurus_vring_alloc_info *vr_alloc_info,
                           int (*notify)(struct taurus_virtqueue *vq));

void * taurus_virtqueue_get_avail_buf(struct taurus_virtqueue *vq, int16_t *avail_idx, uint32_t *len);

int16_t taurus_virtqueue_add_used_buf(struct taurus_virtqueue *vq, int16_t head, uint32_t len);

#endif /* _TAURUS_VIRTQUEUE_H_ */
