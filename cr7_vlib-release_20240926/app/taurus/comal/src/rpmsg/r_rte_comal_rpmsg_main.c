/*************************************************************************************************************
* r_rte_comal_rpmsg_main
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/* TAURUS side implementation of the data exchange with the guest

   This communication layer implements RPMSG on virtio.
 */


/*******************************************************************************
   Section: Includes
 */
#include "rcar-xos/osal/r_osal.h"
#include "r_taurus_bridge.h"
#include "r_rte_comal_api.h"
#include "r_taurus_platform_peri_ids.h"
#include "r_taurus_protocol_ids.h"

#include "taurus_rpmsg.h"
#include "virtio_config.h"
#include "resource_table.h"
#include "taurus_memory_barrier.h"

#include "r_print_api.h"

/*******************************************************************************
   Section: Local Defines
 */
/* Max number of guest that can be handled */
#define RTE_COMAL_GUEST_MAX_NUM         8

/* Max lenght of RPMSG service name */
#define RTE_COMAL_SERVICE_NAME_LEN      RPMSG_NAME_SIZE


/* MFIS interrupts */
#define IRQ_MFIS_ARIICR(x)      (256 + (x))

/* MFIS registers */
#define MFIS_BASE 0xe6260000

#define IICR(n) (0x0400 + n * 0x8)
#define EICR(n) (0x0404 + n * 0x8)
#define IMBR(n) (0x0440 + n * 0x4)
#define EMBR(n) (0x0460 + n * 0x4)

#define LOC_MFIS_REG_WRITE(REG, VAL)    LOC_WRITE32(MFIS_BASE + REG, VAL)
#define LOC_MFIS_REG_READ(REG)          LOC_READ32(MFIS_BASE + REG)

#define LOC_WRITE32(ADDR, VAL)  *((volatile unsigned int *)((size_t)ADDR)) = (VAL)
#define LOC_READ32(ADDR)        *((volatile unsigned int *)((size_t)ADDR))

/* Mutex create resouce ID */
#define LOC_TAURUS_RESOURCE_ID  0x8000U                        /* RESOURCE_ID_BSP         */
#define LOC_TAURUS_MUTEX_CREATE (LOC_TAURUS_RESOURCE_ID + 72U) /* OSAL_MUTEX_TAURUS_RPMSG */

#define LOC_RESOURCE_ID_2DG      0x9000U
#define LOC_TAURUS_MQ_CREATE  (LOC_RESOURCE_ID_2DG + 72U)  /* OSAL_MQ_TAURUS_MFIS */


/* timeout (s)  */
#define LOC_WAIT_TIME_MS    3000 /* 3 sec */

#ifndef RESOURCE_TABLE_BASE
#error "RESOURCE_TABLE_BASE not defined!"
#endif
static struct cr7_resource_table *cr7_res_table = (struct cr7_resource_table *)RESOURCE_TABLE_BASE;

#if defined(R_RPMSG_ENABLE_DEBUG)
#define R_RPMSG_DEBUG(...) R_PRINT_Log(__VA_ARGS__);
#else
#define R_RPMSG_DEBUG(...)
#endif


/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Section: Local Variables
 */

typedef struct {
    char name[RTE_COMAL_SERVICE_NAME_LEN];
    unsigned int service_port;
    unsigned int guest_port;                /* only one guest per service at the moment */
    R_TAURUS_Peripheral_Id_t peripheral_id;
} R_RTE_COMAL_Rpmsg_Service_t;

#define RPMSG_SERVICE_SENTINEL { .name = "", .service_port = 0, .guest_port = 0, .peripheral_id = R_TAURUS_INVALID_PERI_ID }

/* List of services announced to the guest by the TAURUS server. */
static R_RTE_COMAL_Rpmsg_Service_t guest1_services[] = {
#if defined(R_TAURUS_ENABLE_VIRTDEV)
    {
        .name = "taurus-virtdev",
        .service_port = TAURUS_PROTOCOL_VIRTDEV_ID,
        .guest_port = 0,
        .peripheral_id = R_TAURUS_VIRTDEV_PERI_ID
    },
#endif
#if defined(R_TAURUS_ENABLE_RVGC)
    {
        .name = "taurus-rvgc",
        .service_port = TAURUS_PROTOCOL_RVGC_ID,
        .guest_port = 0,
        .peripheral_id = R_TAURUS_RVGC_PERI_ID
    },
#endif
#if defined(R_TAURUS_ENABLE_CAMERA)
    {
        .name = "taurus-camera",
        .service_port = TAURUS_PROTOCOL_CAMERA_ID,
        .guest_port = 0,
        .peripheral_id = R_TAURUS_CAMERA_PERI_ID
    },
#endif
#if defined(R_TAURUS_ENABLE_CAN)
    {
        .name = "taurus-can",
        .service_port = TAURUS_PROTOCOL_CAN_ID,
        .guest_port = 0,
        .peripheral_id = R_TAURUS_CAN_PERI_ID
    },
#endif
#if defined(R_TAURUS_ENABLE_ETHER)
    {
        .name = "taurus-ether",
        .service_port = TAURUS_PROTOCOL_ETHER_ID,
        .guest_port = 0,
        .peripheral_id = R_TAURUS_ETHER_PERI_ID
    },
#endif
#if defined(R_TAURUS_ENABLE_COMCH)
    {
        .name = "taurus-comch",
        .service_port = TAURUS_PROTOCOL_COMCH_ID,
        .guest_port = 0,
        .peripheral_id = R_TAURUS_COMCH_PERI_ID
    },
#endif
    RPMSG_SERVICE_SENTINEL
};

/* R_RTE_COMAL_Rpmsg_Buf_Desc_t
 *
 * struct used to save all the info related to a virtqueue buffer
 * extracted by R_RTE_COMAL_ReadCmd(). These info are needed to
 * release the buffer in R_RTE_COMAL_SendResult() after we are done
 * processing it. */
typedef struct {
    int16_t head;
    struct taurus_rpmsg_hdr *    msg;
    uint32_t msg_len;
    struct taurus_virtqueue *    virtqueue;
} R_RTE_COMAL_Rpmsg_Buf_Desc_t;

#define COMAL_RPMSG_BUF_DESC_RX 0
#define COMAL_RPMSG_BUF_DESC_TX 1

typedef struct {
    int guest;
    unsigned int initialized;
    struct taurus_rpmsg_dev rpmsg_dev;
    R_RTE_COMAL_Rpmsg_Service_t         *services;
    R_RTE_COMAL_Rpmsg_Buf_Desc_t buf_in_use[2];
    osal_mutex_handle_t mutex;
    osal_mq_handle_t  mq_handle;
    osal_device_handle_t device_handle;
} R_RTE_COMAL_Rpmsg_State_t;

static R_RTE_COMAL_Rpmsg_State_t locRpmsgState[RTE_COMAL_GUEST_MAX_NUM];

static void *loc_mfis_irq(osal_device_handle_t device_handle, uint64_t irq_channel, void* Arg)
{
    R_RTE_COMAL_Rpmsg_State_t *rpmsg_state;
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int guest = (int)Arg;
    uint32_t val;
    uint32_t dummy_buf=0;
    (void)device_handle;
    (void)irq_channel;
    
    /* Clear the MFIS interrupt */
    val = LOC_MFIS_REG_READ(IICR(0));
    LOC_MFIS_REG_WRITE(IICR(0), val & ~0x1);

    rpmsg_state = &locRpmsgState[guest];
    osal_ret = R_OSAL_MqSendForTimePeriod(rpmsg_state->mq_handle, (osal_milli_sec_t)200u, &dummy_buf, sizeof(dummy_buf));
    if(OSAL_RETURN_OK != osal_ret) {
        /* Failed: R_OSAL_MqSendForTimePeriod() */
    }

    return 0;
}

static int loc_rpmsg_service_port_to_peripheral(int Guest, unsigned int ServicePort, R_RTE_COMAL_Rpmsg_Service_t * * service)
{
    R_RTE_COMAL_Rpmsg_State_t *rpmsg_state;
    R_RTE_COMAL_Rpmsg_Service_t *service_iter;
    int peri_id = -1;

    rpmsg_state = &locRpmsgState[Guest];
    service_iter = rpmsg_state->services;

    for (; service_iter->peripheral_id != R_TAURUS_INVALID_PERI_ID; service_iter++) {
        if (ServicePort == service_iter->service_port) {
            peri_id = service_iter->peripheral_id;
            if (service != NULL)
                *service = service_iter;
            break;
        }
    }
    return peri_id;
}

static void locFatalError(int Err)
{
    R_PRINT_Log("[ComalRpmsgMain]: NG\r\n");
    while (Err) {
    }
}

static int virtqueue_kick(struct taurus_virtqueue *vq)
{
    uint32_t regval;

    /* If requested, do not kick the ARM host */
    taurus_memory_cache_invalidate(vq->vring.avail, sizeof(*vq->vring.avail));
    if (vq->vring.avail->flags & VRING_AVAIL_F_NO_INTERRUPT) {
        return -1;
    }

    regval = ((vq->id << 1) | 0x1);

    /* Generate a system event to kick the ARM */
    LOC_MFIS_REG_WRITE(EICR(0), regval);

    return 0;
}

void locWaitForResTableInit(struct cr7_resource_table *res_table)
{
    /* Workaround:
     * The CR7 is supposed to start before the other cores, but the
     * virtqueues are set up by Integrity running on the CA5x cores.
     * As soon as we start, invalidate the contents of the resource
     * table. Then check when the same memory location change value,
     * meaning that the other side has completed the initialization.
     */
    volatile uint32_t *p = (volatile uint32_t *)res_table;

    /* *p = 0xBADDCAFE; */

    /* If enabled, data cache should be flushed here */
    taurus_memory_cache_flush((void *)p, sizeof(*p));

    do {
        R_OSAL_ThreadSleepForTimePeriod(100);
    } while (*p != 0x1);
}

/*******************************************************************************
   Function: R_RTE_COMAL_Init
 */


static int loc_MfisDeviceOpen(char *device_type, int device_channel, osal_device_handle_t *device_handle)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    size_t numOfDevice = 0;
    size_t numOfByte = 0;
    size_t i = 0;
    size_t j = 0;
    osal_device_handle_t local_handle = OSAL_DEVICE_HANDLE_INVALID;
    char devicelist[400];

    osal_ret = R_OSAL_IoGetNumOfDevices(device_type, &numOfDevice);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        R_PRINT_Log("[MfisIrq]: loc_MfisDeviceOpen R_OSAL_IoGetNumOfDevices failed(%d)(%s))\r\n", osal_ret, device_type);
    }
    else
    {
        osal_ret = R_OSAL_IoGetDeviceList(device_type, &devicelist[0], sizeof(devicelist), &numOfByte);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            R_PRINT_Log("[MfisIrq]: loc_MfisDeviceOpen R_OSAL_IoGetDeviceList failed(%d)\r\n", osal_ret);
        }
        else
        {
            char *deviceList[numOfDevice + 1];
            deviceList[0] = &devicelist[0];
            for (i = 0; i < numOfByte; i++)
            {
                if (devicelist[i] == '\n')
                {
                    j++;
                    devicelist[i] = '\0';
                    deviceList[j] = &devicelist[i+1];
                }
            }

            if(j > numOfDevice)
            {
                app_ret = -1;
                R_PRINT_Log("[MfisIrq]: loc_MfisDeviceOpen j(%d)>numOfDevice(%d)\r\n", j, numOfDevice);
            }
            else
            {
                osal_ret = R_OSAL_IoDeviceOpen(deviceList[device_channel], &local_handle);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    R_PRINT_Log("[MfisIrq]: loc_MfisDeviceOpen R_OSAL_IoDeviceOpen failed(%d)\r\n", osal_ret);
                }
                else
                {
                    *device_handle = local_handle;
                }
            }
        }
    }

    if(0 != app_ret)
    {
        *device_handle = OSAL_DEVICE_HANDLE_INVALID;
    }

    return app_ret;
}

int R_RTE_COMAL_Init(int Guest)
{

    R_RTE_COMAL_Rpmsg_State_t *rpmsg_state;
    volatile uint8_t *status;
    struct fw_rsc_vdev *rpmsg_dev;
    struct fw_rsc_vdev_vring *vring0;
    struct fw_rsc_vdev_vring *vring1;
    R_RTE_COMAL_Rpmsg_Service_t *service;
    st_osal_mq_config_t mq_config;
    e_osal_return_t osal_ret = OSAL_RETURN_FAIL;
    int16_t ret = -1;
    static int waiting_ticks=1;
    static int first=1;

    R_RPMSG_DEBUG("[ComalRpmsgMain]: COMAL_Init+ 1\n");
    switch (Guest) {
    case 1:
        taurus_memory_cache_invalidate((void *)cr7_res_table, sizeof(*cr7_res_table));
        rpmsg_dev = &cr7_res_table->rpmsg1;
        vring0 = &cr7_res_table->rpmsg1_vring0;
        vring1 = &cr7_res_table->rpmsg1_vring1;
        service = &guest1_services[0];
        break;
    default:
        /* Currently only Guest=1 is supported */
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_Init Guest(%d) failed\r\n", Guest);
        return -1;
    }
    R_RPMSG_DEBUG("[ComalRpmsgMain]: COMAL_Init+ 2\n");

    rpmsg_state = &locRpmsgState[Guest];
    rpmsg_state->guest = Guest;
    rpmsg_state->services = service;

    osal_ret = R_OSAL_MqInitializeMqConfigSt(&mq_config);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: MqInitializeMqConfigSt failed %d", osal_ret);
        return -1;
    }
    mq_config.max_num_msg = 256;
    mq_config.msg_size    = sizeof(uint32_t);
    osal_ret    = R_OSAL_MqCreate(&mq_config, LOC_TAURUS_MQ_CREATE, &rpmsg_state->mq_handle);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: R_OSAL_MqCreate failed %d", osal_ret);
        return -1;
    }

    R_RPMSG_DEBUG("[ComalRpmsgMain]: MfisDeviceOpen+\n");
    loc_MfisDeviceOpen("mfis", 0, &rpmsg_state->device_handle);
    R_RPMSG_DEBUG("[ComalRpmsgMain]: InterruptRegisterIsr+\n");
    R_OSAL_InterruptRegisterIsr(rpmsg_state->device_handle, 0 /*irqChannel*/, 0, (p_osal_isr_func_t)loc_mfis_irq, (void *)Guest);
    R_RPMSG_DEBUG("[ComalRpmsgMain]: InterruptEnableIsr+\n");
    R_OSAL_InterruptEnableIsr(rpmsg_state->device_handle, 0 /*irqChannel*/);
    R_RPMSG_DEBUG("[ComalRpmsgMain]: InterruptEnableIsr-\n");

    osal_ret = R_OSAL_ThsyncMutexCreate(LOC_TAURUS_MUTEX_CREATE, &rpmsg_state->mutex);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: R_OSAL_ThsyncMutexCreate failed %d", osal_ret);
        return osal_ret;
    }

    /* Make sure the Guest drivers are ready for RPMsg communication */
    R_RPMSG_DEBUG("[ComalRpmsgMain]: Waiting for Guest to connect\n");
    R_RPMSG_DEBUG("[ComalRpmsgMain]: Added print during waiting time\n");
    status = &rpmsg_dev->status;
    *status = 0;
    taurus_memory_cache_flush((void *)status,sizeof(*status));
    R_PRINT_Log("[ComalRpmsgMain]: resource table address %p 0x%08x\n",cr7_res_table, *status);


    do {
        waiting_ticks++;
        if (first) R_RPMSG_DEBUG("[ComalRpmsgMain]: Wait loop 1\n");
        R_OSAL_ThreadSleepForTimePeriod(50);
        if (first) R_RPMSG_DEBUG("[ComalRpmsgMain]: Wait loop 2\n");
        taurus_memory_cache_invalidate((void *)status, sizeof(*status));
        if (first) R_RPMSG_DEBUG("[ComalRpmsgMain]: Wait loop 3\n");
        if(waiting_ticks%20 == 0) {
            R_RPMSG_DEBUG("[ComalRpmsgMain]: Waiting for connection %d\n",waiting_ticks%100);
        }
        if (first) R_RPMSG_DEBUG("[ComalRpmsgMain]: Wait loop 4\n");
        first = 0;
    } while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));
    R_RPMSG_DEBUG("[ComalRpmsgMain]: Connection to Guest established\n");

    /* Initialize the RPMsg rpmsg_dev structure */
    ret = taurus_rpmsg_init(&rpmsg_state->rpmsg_dev, vring0, vring1, &virtqueue_kick);
    if (0 != ret) {
        R_PRINT_Log("[ComalRpmsgMain]: taurus_rpmsg_init failed\n");
        return -1;
    }

    R_RPMSG_DEBUG("[ComalRpmsgMain]: Announce supported services to the guest\n");
    /* Announce supported services to the guest */
    for (; service->peripheral_id != R_TAURUS_INVALID_PERI_ID; service++) {
        R_RPMSG_DEBUG("[ComalRpmsgMain]: Announcing service %s\n", service->name);
        ret = taurus_rpmsg_channel(RPMSG_NS_CREATE,
                                   &rpmsg_state->rpmsg_dev,
                                   service->name,
                                   service->service_port);
        if (0 != ret) {
            R_PRINT_Log("[ComalRpmsgMain]: taurus_rpmsg_init failed(%d)\n", ret);
            return -1;
        }
    }

    rpmsg_state->initialized = 1;
    R_RPMSG_DEBUG("[ComalRpmsgMain]: Init complete\n");

    if(OSAL_RETURN_OK != osal_ret) {
        ret = -1;
    }
    return ret;
}


/*******************************************************************************
   Function: R_RTE_COMAL_DeInit
 */

int R_RTE_COMAL_DeInit(int Guest)
{
    R_RTE_COMAL_Rpmsg_State_t *rpmsg_state;
    e_osal_return_t osal_ret = OSAL_RETURN_FAIL;

    rpmsg_state = &locRpmsgState[Guest];

    osal_ret = R_OSAL_ThsyncMutexDestroy(rpmsg_state->mutex);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_DeInit Mutex destroy failed(%d)\n", osal_ret);
    }

    osal_ret = R_OSAL_MqDelete(rpmsg_state->mq_handle);
    if(OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_DeInit R_OSAL_MqDelete failed\n");
    }

    return 0;
}


/*******************************************************************************
   Function: R_RTE_COMAL_ReadCmd
 */

int R_RTE_COMAL_ReadCmd(int Guest, R_TAURUS_CmdMsg_t *Cmd)
{
    R_RTE_COMAL_Rpmsg_State_t *          rpmsg_state;
    R_RTE_COMAL_Rpmsg_Buf_Desc_t *       buf_in_use;
    R_TAURUS_CmdMsg_t *                  rpmsg_cmd_hdr;
    int16_t head;
    struct taurus_rpmsg_hdr *            msg = NULL;
    uint32_t msg_len;
    struct taurus_virtqueue *            virtqueue;
    uint32_t src, dst;
    int32_t Per;
    R_RTE_COMAL_Rpmsg_Service_t *        service;
    e_osal_return_t                      osal_ret = OSAL_RETURN_OK;
    uint32_t dummy_buf;

    msg = NULL;

    if ((Guest < 0) || (Guest >= RTE_COMAL_GUEST_MAX_NUM)) {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_ReadCmd Guest(%d) error\n", Guest);
        return -1;
    }

    rpmsg_state = &locRpmsgState[Guest];

    if (!rpmsg_state->initialized) {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_ReadCmd Not initialized\n");
        return -1;
    }

    /* In order to avoid copying buffers around, R_RTE_COMAL_ReadCmd()
     * grabs two buffers from the virtqueues which are directly passed
     * to the peripheral driver via Par1 and Par2 fields in the
     * R_TAURUS_CmdMsg_t struct. The first buffer holds the received message with
     * the input data and the other is an empty buffer where the
     * peripheral driver put the output data.
     *
     * We make a copy of the buffers' descriptors so that we can later
     * return the two buffers to the virtqueues once the peripheral
     * has finished processing them. This takes place in
     * R_RTE_COMAL_SendResult().
     *
     * Until a different solution is found, this approach basically
     * makes R_RTE_COMAL_ReadCmd() and R_RTE_COMAL_SendResult() not
     * reentrant (They are thread-safe though, as long as each thread
     * uses a different Guest id).
     */
    if (rpmsg_state->buf_in_use[0].msg || rpmsg_state->buf_in_use[1].msg) {
        /* This should not happen */
        locFatalError(-1);
    }

    /* Get an available buffer from the RX queue */
    virtqueue = &rpmsg_state->rpmsg_dev.rvq;

    do {
#if defined(USE_AUTOSAR)
        R_OSAL_ThreadSleepForTimePeriod(1);
#endif
#if (defined(INTEGRITY_GUEST)|| defined(LINUX_GUEST))
        osal_ret = R_OSAL_MqReceiveForTimePeriod(rpmsg_state->mq_handle,(osal_milli_sec_t)1000u,&dummy_buf,sizeof(dummy_buf));
#endif
        if (OSAL_RETURN_TIME == osal_ret) {
            continue;
        } else if (OSAL_RETURN_OK == osal_ret ) {
            osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(rpmsg_state->mutex, LOC_WAIT_TIME_MS);
            if(OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_ReadCmd R_OSAL_ThsyncMutexLockForTimePeriod failed\n");
            } else {
                msg = taurus_virtqueue_get_avail_buf(virtqueue, &head, &msg_len);
                osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_ReadCmd R_OSAL_ThsyncMutexUnlock failed\n");
                }
            }
        }
    } while (!msg);
    
    /* invalidate message header */
    taurus_memory_cache_invalidate(msg, sizeof(*msg));
        
    /* TEMPORARY store source and destination of the message */
    src = msg->src;
    dst = msg->dst;

    /* Save the info into rpmsg_state struct so we are able to release
     * this buffer later */
    buf_in_use = &rpmsg_state->buf_in_use[COMAL_RPMSG_BUF_DESC_RX];
    buf_in_use->head = head;
    buf_in_use->msg = msg;
    buf_in_use->msg_len = msg_len;
    buf_in_use->virtqueue = virtqueue;

    /* invalidate message body */
    taurus_memory_cache_invalidate((void *)msg->data, msg_len);
    rpmsg_cmd_hdr = (R_TAURUS_CmdMsg_t *)msg->data;

    /* copy the header into the TAURUS Cmd struct */
    Cmd->Id = rpmsg_cmd_hdr->Id;
    Cmd->Channel = rpmsg_cmd_hdr->Channel;
    Cmd->Cmd = rpmsg_cmd_hdr->Cmd;
    /* Par1 is the cmd id for the peripheral */
    Cmd->Par1 = rpmsg_cmd_hdr->Par1;
    /* Par2 is the address in memory of the input message */
    Cmd->Par2 = (intptr_t)msg->data;

    /* Now we need to find which peripheral this command is for */
    service = 0;
    Per = loc_rpmsg_service_port_to_peripheral(Guest, dst, &service);
    if (Per == -1) {
        /* We received a command for a peripheral that we don't
         * know... just set the TAURUS Cmd to an invalid value and return so
         * that the command parser will reply with a NACK */
        Cmd->Cmd = ~0x0;
        Cmd->Per = ~0x0;
    } else {
        Cmd->Per = Per;
    }
    /* Save the guest port for this service */
    if (service->guest_port == 0)
        service->guest_port = src;

    /* We need a buffer where the pheriperal can write the output.
     * Get an available buffer from the TX queue */
    virtqueue = &rpmsg_state->rpmsg_dev.svq;

    do {
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(rpmsg_state->mutex, LOC_WAIT_TIME_MS);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_ReadCmd R_OSAL_ThsyncMutexLockForTimePeriod failed\n");
        }
        /* FIXME: should have a sleep in here if buffer cannot be aquired or wait for a notify (Integrity
           does notify about putting buffers back in the queue, but not so important as messages do not queue
           up as they wait for a reply */
        msg = taurus_virtqueue_get_avail_buf(virtqueue, &head, &msg_len);
        osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_ReadCmd R_OSAL_ThsyncMutexLockForTimePeriod failed\n");
        }
    } while (!msg);

    /* set source and destination for the reply */
    msg->src = dst;
    msg->dst = src;

    /* Save the info into rpmsg_state struct so we are able to release
     * this buffer later */
    buf_in_use = &rpmsg_state->buf_in_use[COMAL_RPMSG_BUF_DESC_TX];
    buf_in_use->head = head;
    buf_in_use->msg = msg;
    buf_in_use->msg_len = msg_len;
    buf_in_use->virtqueue = virtqueue;

    /* Par3 is the address in memory of the output message */
    Cmd->Par3 = (intptr_t)msg->data;

    return osal_ret;
}


/*******************************************************************************
   Function: R_RTE_COMAL_SendAck
 */

int R_RTE_COMAL_SendAck(int Guest, const R_TAURUS_ResultMsg_t *Result)
{
    R_RTE_COMAL_Rpmsg_State_t *          rpmsg_state;
    R_RTE_COMAL_Rpmsg_Buf_Desc_t *       buf_in_use;
    R_TAURUS_ResultMsg_t                 rpmsg_ack;
    int                                  ret;
    e_osal_return_t                      osal_ret = OSAL_RETURN_OK;

    if (Guest < 0 || Guest >= RTE_COMAL_GUEST_MAX_NUM) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Guest(%d) failed\n", Guest);
        return -1;
    }
    rpmsg_state = &locRpmsgState[Guest];

    if (!rpmsg_state->initialized) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult rpmsg_state not initialized\n");
        return -1;
    }

    do {
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(rpmsg_state->mutex, LOC_WAIT_TIME_MS);
    } while (OSAL_RETURN_TIME == osal_ret);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex lock failed\n");
    }
    
    if (!(rpmsg_state->buf_in_use[0].msg) || !(rpmsg_state->buf_in_use[1].msg)) {
        /* This should not happen */
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult msg NULL\n");
        locFatalError(-1);
    }

    if (Result->Result != R_TAURUS_RES_ACK) {
        osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex unlock failed\n");
        }
        return -1;
    }
        
    /* We are just sending back the ACK, so it's not time yet to
    * release the buffers. We will release the buffers next time
    * R_RTE_COMAL_SendResult() is called, when we have the result
    * from the peripheral.*/

    rpmsg_ack.Id = Result->Id;
    rpmsg_ack.Channel = Result->Channel;
    rpmsg_ack.Result = Result->Result;
    rpmsg_ack.Aux = 0;

    buf_in_use = &rpmsg_state->buf_in_use[COMAL_RPMSG_BUF_DESC_TX];

    ret = taurus_rpmsg_send(&rpmsg_state->rpmsg_dev,
                            buf_in_use->msg->src,
                            buf_in_use->msg->dst,
                            (void *)&rpmsg_ack,
                            sizeof(rpmsg_ack));
    osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex unlock failed\n");
    }

    if (0 != ret) osal_ret = ret;

    return osal_ret;
}

int R_RTE_COMAL_SendResult(int Guest, const R_TAURUS_ResultMsg_t *Result)
{
    R_RTE_COMAL_Rpmsg_State_t *          rpmsg_state;
    R_RTE_COMAL_Rpmsg_Buf_Desc_t *       buf_in_use;
    R_TAURUS_ResultMsg_t *               rpmsg_res_hdr;
    struct taurus_rpmsg_hdr *            msg;
    uint32_t                             msg_len;
    int                                  ret = 0;
    e_osal_return_t                      osal_ret = OSAL_RETURN_OK;

    if (Guest < 0 || Guest >= RTE_COMAL_GUEST_MAX_NUM) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Guest(%d) failed\n", Guest);
        return -1;
    }

    rpmsg_state = &locRpmsgState[Guest];

    if (!rpmsg_state->initialized) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult rpmsg_state not initialized\n");
        return -1;
    }

    do {
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(rpmsg_state->mutex, LOC_WAIT_TIME_MS);
    } while (OSAL_RETURN_TIME == osal_ret);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex lock failed\n");
    }
    
    if (!(rpmsg_state->buf_in_use[0].msg) || !(rpmsg_state->buf_in_use[1].msg)) {
        /* This should not happen */
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult msg NULL\n");
        locFatalError(-1);
    }

    if (Result->Result == R_TAURUS_RES_ACK) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Bad result\n");
        osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex unlock failed\n");
        }
        return -1;
    }

    /* Return the used inbound buffer */
    buf_in_use = &rpmsg_state->buf_in_use[COMAL_RPMSG_BUF_DESC_RX];
    /* TODO: This is not nice, why mix taurus_virtqueue_add_used_buf and taurus_rpmsg_send  ? */
    /* TODO: Shouldn't taurus_rpmsg_send be the only one to call virtqueue functions ? */
    /* TODO: It make adding of the caching stuff more complicated because inteface is not clear */
    msg = buf_in_use->msg;
    msg_len = buf_in_use->msg_len;
    /* Clean message header */
    taurus_memory_cache_flush((void *)msg, sizeof(*msg));
    /* Clean message body */
    taurus_memory_cache_flush((void *)msg->data, msg_len);
    
    ret = taurus_virtqueue_add_used_buf(buf_in_use->virtqueue, buf_in_use->head, buf_in_use->msg_len);
    if (ret < 0) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult taurus_virtqueue_add_used_buf failed\n");
        osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex unlock failed\n");
        }
        return -1;
    }

    /* Kick the ARM host */
    if (buf_in_use->virtqueue->notify)
        buf_in_use->virtqueue->notify(buf_in_use->virtqueue);

    buf_in_use->msg = NULL;


    /* Return the used inbound buffer */
    buf_in_use = &rpmsg_state->buf_in_use[COMAL_RPMSG_BUF_DESC_TX];

    rpmsg_res_hdr = (R_TAURUS_ResultMsg_t *)buf_in_use->msg->data;
    rpmsg_res_hdr->Id = Result->Id;
    rpmsg_res_hdr->Per = Result->Per;
    rpmsg_res_hdr->Channel = Result->Channel;
    rpmsg_res_hdr->Result = Result->Result;

    /* Result.Aux holds the size of the answer written by the
     * peripheral */
    buf_in_use->msg->len = rpmsg_res_hdr->Aux;

    msg = buf_in_use->msg;
    msg_len = buf_in_use->msg_len;
    
    /* Clean message header */
    taurus_memory_cache_flush((void *)msg, sizeof(*msg));
    /* Clean message body */
    taurus_memory_cache_flush((void *)msg->data, msg_len);

    /* Add the used buffer */
    ret = taurus_virtqueue_add_used_buf(buf_in_use->virtqueue, buf_in_use->head, buf_in_use->msg_len);
    if (ret < 0) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult taurus_virtqueue_add_used_buf failed\n");
        osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex unlock failed\n");
        }
        return -1;
    }

    /* Kick the ARM host */
    if (buf_in_use->virtqueue->notify)
        buf_in_use->virtqueue->notify(buf_in_use->virtqueue);

    buf_in_use->msg = NULL;

    osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[ComalRpmsgMain]: COMAL_SendResult Mutex unlock failed\n");
    }

    return osal_ret;
}


static R_RTE_COMAL_Rpmsg_Service_t * loc_signalid_to_service(int Guest, R_TAURUS_SignalId_t Signal)
{
    R_RTE_COMAL_Rpmsg_State_t *rpmsg_state;
    R_RTE_COMAL_Rpmsg_Service_t *service_iter;
    R_RTE_COMAL_Rpmsg_Service_t *ret = NULL;
    uint8_t service_id;

    rpmsg_state = &locRpmsgState[Guest];
    service_iter = rpmsg_state->services;

    /* The service's id that wants to notify the guest is
     * encoded in the most significant byte of R_TAURUS_SignalId_t
     */
    service_id = (uint8_t)(Signal >> ((sizeof(Signal) - 1) * 8));

    for (; service_iter->peripheral_id != R_TAURUS_INVALID_PERI_ID; service_iter++) {
        if (service_iter->service_port == service_id) {
            ret = service_iter;
            break;
        }
    }
    return ret;
}

/*******************************************************************************
   Function: R_RTE_COMAL_SendSignal
 */

int R_RTE_COMAL_SendSignal(int Guest, R_TAURUS_SignalId_t Signal, void* Option)
{
    R_RTE_COMAL_Rpmsg_State_t   * rpmsg_state;
    R_TAURUS_ResultMsg_t          signal_msg;
    R_RTE_COMAL_Rpmsg_Service_t * service;
    int                           ret;
    e_osal_return_t               osal_ret = OSAL_RETURN_FAIL;    
#if defined(R_TAURUS_ENABLE_ETHER)
    R_TAURUS_Eth_Data_t           receive_data;
#endif
    (void )Option;

    if (Guest < 0 || Guest >= RTE_COMAL_GUEST_MAX_NUM) {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_SendSignal Guest(%d) failed\n", Guest);
        return -1;
    }

    rpmsg_state = &locRpmsgState[Guest];

    if (!rpmsg_state->initialized) {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_SendSignal rpmsg_state not initialized\n");
        return -1;
    }

    service =  loc_signalid_to_service(Guest, Signal);
    if (0 == service) {
        R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_SendSignal service NULL failed\n");
        return -1;
    }

    /* To signal a guest, a R_TAURUS_ResultMsg_t message is sent with
     * transaction Id=0 and Result=R_TAURUS_CMD_NOP. The
     * R_TAURUS_SignalId_t is copied into the Aux field. */
    signal_msg.Id = 0;
    signal_msg.Per = service->service_port;
    signal_msg.Channel = 0;
    signal_msg.Result = R_TAURUS_CMD_NOP;
#if defined(R_TAURUS_ENABLE_ETHER)
    if (((TAURUS_PROTOCOL_ETHER_ID << 24) | 0x000000) == Signal) /* ETHER_PROTOCOL_EVENT_PKT_AVAIL_CH0 */
    {
        if (NULL != Option)
        {
            receive_data = *(R_TAURUS_Eth_Data_t*)Option;
            signal_msg.Eth_Aux.Addr = receive_data.Addr;
            signal_msg.Eth_Aux.Size = receive_data.Size;
        }
        else
        {
            R_PRINT_Log("[ComalRpmsgMain]: R_RTE_COMAL_SendSignal 3rd argument Option is NULL in Ether event\n");
            return -1;
        }
    }
    else
#endif
    {
        signal_msg.Aux = Signal;
    }
    do {
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(rpmsg_state->mutex, LOC_WAIT_TIME_MS);
    } while (OSAL_RETURN_TIME == osal_ret);    
    if (OSAL_RETURN_OK != osal_ret) {
        return -1;
    }

    ret = taurus_rpmsg_send(&rpmsg_state->rpmsg_dev,
                            service->service_port,
                            service->guest_port,
                            (void *)&signal_msg,
                            sizeof(signal_msg));
    osal_ret = R_OSAL_ThsyncMutexUnlock(rpmsg_state->mutex);
    if (OSAL_RETURN_OK != osal_ret) {
        return -1;
    }
    
    if (0 != ret) osal_ret = ret;

    return osal_ret;
}

