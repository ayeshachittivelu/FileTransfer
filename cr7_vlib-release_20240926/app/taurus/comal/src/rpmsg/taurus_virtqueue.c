/*************************************************************************************************************
* taurus_virtqueue.c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#include "taurus_virtqueue.h"
#include "taurus_memory_barrier.h"
#include "virtio_ring.h"
#include "r_print_api.h"

void taurus_virtqueue_init(struct taurus_virtqueue *vq,
                           uint32_t id,
                           const struct taurus_vring_alloc_info *vr_alloc_info,
                           int (*notify)(struct taurus_virtqueue *vq))
{
    vq->id = id;
    vq->last_avail_idx = 0;
    vq->notify = notify;

    vring_init(&vq->vring, vr_alloc_info->num_descs, vr_alloc_info->pages, vr_alloc_info->align);
}

void * taurus_virtqueue_get_avail_buf(struct taurus_virtqueue *vq, int16_t *avail_idx, uint32_t *len)
{
    int16_t head;
    void *buffer;
    struct vring_desc *desc_p;

    /* invalidate cache */
    taurus_memory_cache_invalidate(vq->vring.avail, sizeof(vq->vring.avail));


    if (vq->last_avail_idx == vq->vring.avail->idx) {
        /* no new buffers are available */
        return 0;
    }

    head = vq->last_avail_idx++ & (vq->vring.num - 1);
    taurus_memory_cache_invalidate(&vq->vring.avail->ring[head], sizeof(vq->vring.avail->ring[head]));
    *avail_idx = vq->vring.avail->ring[head];

    desc_p = vq->vring.desc + *avail_idx;
    taurus_memory_cache_invalidate(desc_p, sizeof(*desc_p));
    buffer = (void *)(uint32_t)(desc_p->addr);
    *len = desc_p->len;
   
    return buffer;
}

int16_t taurus_virtqueue_add_used_buf(struct taurus_virtqueue *vq, int16_t head, uint32_t len)
{
    struct vring_used_elem *used_elem;
    uint16_t used_idx;

    if (head > vq->vring.num) {
        /* invalid head */
        R_PRINT_Log("taurus_virtqueue_add_used_buf invalid head.\r\n");
        return -1;
    }

    taurus_memory_cache_invalidate(vq->vring.used, sizeof(*vq->vring.used));
    used_idx = vq->vring.used->idx & (vq->vring.num - 1);
    used_elem = &vq->vring.used->ring[used_idx];
    used_elem->id = head;
    used_elem->len = len;

    /* cache clean used_element */
    taurus_memory_cache_flush(used_elem, sizeof(*used_elem));

    vq->vring.used->idx++;

    /* cache clean idx */
    taurus_memory_cache_flush(&vq->vring.used->idx, sizeof(vq->vring.used->idx));

    return 0;
}
