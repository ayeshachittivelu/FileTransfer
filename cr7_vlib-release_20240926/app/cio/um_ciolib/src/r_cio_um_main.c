/*************************************************************************************************************
* cio_um_main_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/


/*******************************************************************************
   Section: Includes
 */
#include "rcar-xos/osal/r_osal.h"
#include "r_cio_api.h"
#include "r_cio_um_main.h"
#include "r_print_api.h"

/*******************************************************************************
   Section: Local Defines
 */

/*******************************************************************************
   Define: LOC_CHANNEL_NUM

   Maximum number of I/O channels managed by the cio_um_library
   (consider to move the configuration out of the library)
 */

#define LOC_CHANNEL_NUM 10


/*******************************************************************************
   Define: LOC_INITIAL_MSGID

   Starting id for an I/O channel
 */

#define LOC_INITIAL_MSGID 1000


/*******************************************************************************
   Section: Local Variables
 */

/*******************************************************************************
   Variable: locChannelPool

   Buffer to store I/O channel specific data, e.g. message queues to the driver.

   (consider to move the pool itself out of the library)
 */

static R_CIO_UM_Channel_t locChannelPool[LOC_CHANNEL_NUM];


/*******************************************************************************
   Variable: locChannelId

   Unique id assigned to a channel during allocation from the pool
 */

static int locChannelId;


/*******************************************************************************
   Variable: locCioInitFlag

   Flag set to 0 if CIO has not been initialised in this process yet, 1 if it
   has been
 */

static int locCioInitFlag = 0;

/*******************************************************************************
   Variable: locCioCommonLock

   This mutex is utilized for access protection of common structures
   of the CIO UM library
*/

static osal_mutex_handle_t locCioUMCommonLock;


#define CIO_RESOURCE_ID_BSP      0x8000U
#define MUTEX_CIO_LIB       (CIO_RESOURCE_ID_BSP   + 0U)
#define MUTEX_CIO_LIB_CH    (CIO_RESOURCE_ID_BSP   + 58U)

/*******************************************************************************
   Section: Local Functions
 */


/*******************************************************************************
   Function: locAllocChannel

   Allocate channel from channel pool.

   Parameters:
   void

   Returns:
   ==0  - NG
   !=0  - Pointer to a channel
 */

static R_CIO_UM_Channel_t *locAllocChannel(void)
{
    R_CIO_UM_Channel_t *ch;
    int i;
    e_osal_return_t osal_ret;

    ch = 0;
    for (i = 0; i < LOC_CHANNEL_NUM; i++) {
        if (locChannelPool[i].ChannelId == 0) {
            ch = &locChannelPool[i];
            locChannelPool[i].ChannelId = locChannelId;
            locChannelId++;
            osal_ret = R_OSAL_ThsyncMutexCreate(MUTEX_CIO_LIB_CH+i, &locChannelPool[i].ChannelMutex);
            if (osal_ret != OSAL_RETURN_OK) {
                R_PRINT_Log("[CioUmMain]: locAllocChannel R_OSAL_ThsyncMutexCreate failed(%d)\r\n", osal_ret);
                locChannelPool[i].ChannelId = 0;
                ch = 0;
            }
            break;
        }
    }
    return ch;
}


/*******************************************************************************
   Function: locFreeChannel

   Mark a channel as free and make it available for allocation again.

   Parameters:
   ch         - Pointer to a channel

   Returns:
   ==0  - OK
   !=0  - NG
 */

static int locFreeChannel(R_CIO_UM_Channel_t *Ch)
{
    int x = 0;
    e_osal_return_t osal_ret;
    Ch->ChannelId = 0;
    Ch->MsgId = LOC_INITIAL_MSGID;

    osal_ret = R_OSAL_ThsyncMutexDestroy(Ch->ChannelMutex);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: locFreeChannel R_OSAL_ThsyncMutexDestroy failed(%d)\r\n", osal_ret);
        x = -1;
    }

    return x;
}


/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Function: R_CIO_PRV_FindChannel
 */

R_CIO_UM_Channel_t *R_CIO_PRV_FindChannel(int Id)
{
    R_CIO_UM_Channel_t *ch;
    int i;

    ch = 0;
    if (1 == locCioInitFlag) {
        /* this function is called by all IO functions (read, write, ioctl, status...)
         * it is easier to make the init check here and return a null pointer
         * in case of error. Shall never come to that, as user shall get an error
         * already by opening a channel
         */
        for (i = 0; i < LOC_CHANNEL_NUM; i++) {
            if (locChannelPool[i].ChannelId == Id) {
                ch = &locChannelPool[i];
                break;
            }
        }
    }

    return ch;
}

/*******************************************************************************
   Function: R_CIO_Init
 */

int R_CIO_Init(void)
{
    int i;
    int ret = 0;
    e_osal_return_t osal_ret;
    osal_mutex_handle_t  temp_mutex_handle;

    osal_ret = R_OSAL_ThsyncMutexCreate(MUTEX_CIO_LIB, &temp_mutex_handle);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Init R_OSAL_ThsyncMutexCreate failed(%d)\r\n", osal_ret);
        ret = -1;
    } else {
        locCioUMCommonLock = temp_mutex_handle;
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(locCioUMCommonLock, (osal_milli_sec_t)TIMEOUT_MS);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Init R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
        }
    }

    if ((0 == locCioInitFlag) && (-1 != ret)) {
        locChannelId = 1;
        for (i = 0; i < LOC_CHANNEL_NUM; i++) {
            locChannelPool[i].MsgQueue = 0;
            locChannelPool[i].RetQueue = 0;
            locChannelPool[i].MsgId = LOC_INITIAL_MSGID;
            locChannelPool[i].ChannelId = 0;
        }
        locCioInitFlag = 1;
    } else {
        R_PRINT_Log("[CioUmMain]: R_CIO_Init Mutex error(ret:%d) or Already R_CIO_Init(locCioInitFlag:%d).\r\n",
                    ret, locCioInitFlag);
        ret = -1;
    }

    if (-1 != ret) {
        osal_ret = R_OSAL_ThsyncMutexUnlock(locCioUMCommonLock);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Init R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            ret = -1;
        }
    }
    return ret;
}


/*******************************************************************************
   Function: R_CIO_DeInit
 */

int R_CIO_DeInit(void)
{
    int i;
    e_osal_return_t osal_ret;

    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(locCioUMCommonLock, (osal_milli_sec_t)TIMEOUT_MS);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioUmMain]: R_CIO_DeInit R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
        return -1;
    }
    for (i = 0; i < LOC_CHANNEL_NUM; i++) {
        locChannelPool[i].MsgQueue = 0;
        locChannelPool[i].RetQueue = 0;
        locChannelPool[i].MsgId = LOC_INITIAL_MSGID;
        locChannelPool[i].ChannelId = 0;
    }
    locCioInitFlag = 0;
    osal_ret = R_OSAL_ThsyncMutexUnlock(locCioUMCommonLock);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioUmMain]: CIO_DeInit R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
        return -1;
    }
    osal_ret = R_OSAL_ThsyncMutexDestroy(locCioUMCommonLock);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioUmMain]: CIO_DeInit R_OSAL_ThsyncMutexDestroy failed(%d)\r\n", osal_ret);
        return -1;
    }
    return 0;
}


/*******************************************************************************
   Function: R_CIO_Open
 */

int R_CIO_Open(const char *Name, const char *Mode)
{
    char qname[R_CIO_MAX_DEVICENAME];
    Cio_MqHandle_t *mq;
    e_osal_return_t osal_ret;
    r_cio_CmdMsg_t msg;
    r_cio_ResultMsg_t result;
    int ret = -1;
    int id;
    R_CIO_UM_Channel_t *ch;
    int channel;
    int destroy = 0;

    if (0 == locCioInitFlag) {
        /* CIO not initialised */
        R_PRINT_Log("[CioUmMain]: R_CIO_Open CIO not initialised. InitFlag(%d)\r\n", locCioInitFlag);
        goto end_no_mutex;
    }
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(locCioUMCommonLock, (osal_milli_sec_t)TIMEOUT_MS);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_ThsyncMutexLockForTimePeriod(locCioUMCommonLock) failed(%d)\r\n", osal_ret);
        ret = -2;
        goto end_no_mutex;
    }

    ch = locAllocChannel();
    if (ch) {
        id = ch->ChannelId;
    } else {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open locAllocChannel failed.\r\n");
        goto end_no_ch_mutex;
    }

    R_OSAL_ThreadSleepForTimePeriod(1);

    /* send open command to CIO msg queue */
    mq = GetMqHandl(R_CIO_QUEUE_MSG_NAME);
    if (mq == NULL) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open GetMqHandl(R_CIO_QUEUE_MSG_NAME) failed.\r\n");
        (void)locFreeChannel(ch);
        ret = -1;
        goto end_no_ch_mutex;
    }

    msg.Id = LOC_INITIAL_MSGID;
    msg.Cmd = R_CIO_CMD_OPEN;
    msg.Par1 = (void *)Name;
    msg.Par2 = (void *)Mode;
    msg.Par3 = 0;
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(ch->ChannelMutex, (osal_milli_sec_t)TIMEOUT_MS);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_ThsyncMutexLockForTimePeriod(ch->ChannelMutex) failed(%d)\r\n", osal_ret);
        (void)locFreeChannel(ch);
        ret = -2;
        goto end_no_ch_mutex;
    }

    osal_ret = R_OSAL_MqSendForTimePeriod(mq->mq_handle, (osal_milli_sec_t)TIMEOUT_MS,
                            &msg, CIO_MSG_MQ_MSG_SIZE);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_MqSendForTimePeriod failed(%d)\r\n", osal_ret);
        destroy = 1;
        goto end_unlock_ch_mutex;
    }

    R_OSAL_ThreadSleepForTimePeriod(1);

    /* Receive result of open command */
    mq = GetMqHandl(R_CIO_QUEUE_RET_NAME);
    if (mq == NULL) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open GetMqHandl(R_CIO_QUEUE_RET_NAME) failed.\r\n");
        destroy = 1;
        goto end_unlock_ch_mutex;
    }


    do {
        osal_ret = R_OSAL_MqReceiveForTimePeriod(mq->mq_handle, (osal_milli_sec_t)TIMEOUT_MS,
                            (void*)&result, CIO_RET_MQ_MSG_SIZE);
    } while(OSAL_RETURN_TIME == osal_ret);
    if (osal_ret == OSAL_RETURN_OK) {
        if ((result.Id != msg.Id) || (result.Result != R_CIO_RES_ACK)) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_MqReceiveForTimePeriod Argument failed. Id(%d), Result(%d)\r\n",
                        result.Id, result.Result);
            osal_ret = -1;
        }
    }

    if (osal_ret == OSAL_RETURN_OK) {
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(mq->mq_handle, (osal_milli_sec_t)TIMEOUT_MS,
                            (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (osal_ret == OSAL_RETURN_OK) {
            if ((result.Id != msg.Id) || (result.Result != R_CIO_RES_COMPLETE)) {
                R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_MqReceiveForTimePeriod Argument failed. Id(%d), Result(%d)\r\n",
                    result.Id , result.Result);
                osal_ret = -1;
            }
        }
    }

    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_MqReceiveForTimePeriod Return failed(%d), Id(%d), Result(%d)\r\n",
                    osal_ret, result.Id, result.Result);
        destroy = 1;
        goto end_unlock_ch_mutex;
    }

    R_OSAL_ThreadSleepForTimePeriod(10);

    /* open msg & result queue for the driver */
    channel = (int)(size_t)result.Aux;
    ch->QId = channel;
    (void)snprintf(qname, R_CIO_MAX_DEVICENAME, "%s%d", R_CIO_QUEUE_MSG_NAME, channel);
    mq = GetMqHandl(qname);
    if (mq == NULL) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_CIO_QUEUE_MSG_NAME GetMqHandl failed.\r\n");
        destroy = 1;
        goto end_unlock_ch_mutex;
    }
    ch->MsgQueue = mq->mq_handle;

    R_OSAL_ThreadSleepForTimePeriod(1);

    (void)snprintf(qname, R_CIO_MAX_DEVICENAME, "%s%d", R_CIO_QUEUE_RET_NAME, channel);
    mq = GetMqHandl(qname);
    if (mq == NULL) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_CIO_QUEUE_RET_NAME GetMqHandl failed.\r\n");
        destroy = 1;
        goto end_unlock_ch_mutex;
    }
    ch->RetQueue = mq->mq_handle;

    ret = id;

 end_unlock_ch_mutex:
    osal_ret =R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_ThsyncMutexUnlock(ChannelMutex) failed(%d)\r\n", osal_ret);
        ret = -1;
    }

    /* Destroy do after unlock */
    if (0 != destroy) {
        (void)locFreeChannel(ch);
        ret = -1;
    }

 end_no_ch_mutex:
    osal_ret =R_OSAL_ThsyncMutexUnlock(locCioUMCommonLock);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Open R_OSAL_ThsyncMutexUnlock(locCioUMCommonLock) failed(%d)\r\n", osal_ret);
        ret = -1;
    }
 end_no_mutex:
    return ret;
}



/*******************************************************************************
   Function: R_CIO_Close
 */

int R_CIO_Close(int Id)
{
    R_CIO_UM_Channel_t *ch;
    r_cio_CmdMsg_t msg;
    r_cio_ResultMsg_t result;
    int x;
    int ret = 0;
    e_osal_return_t osal_ret;

    ch = R_CIO_PRV_FindChannel(Id);
    if (ch) {
        /* send close command */
        msg.Id = ch->MsgId;
        ch->MsgId++;
        msg.Cmd = R_CIO_CMD_CLOSE;
        msg.Par1 = 0;
        msg.Par2 = 0;
        msg.Par3 = 0;
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(ch->ChannelMutex, (osal_milli_sec_t)TIMEOUT_MS);
        if (osal_ret != OSAL_RETURN_OK) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            goto end_mutex_destroy;
        }
        osal_ret = R_OSAL_MqSendForTimePeriod(ch->MsgQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                &msg, CIO_MSG_MQ_MSG_SIZE);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_MqSendForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            goto end;
        }

        /* check acknowledge */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Close check acknowledge R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            goto end;
        } else {
            if ((result.Id != msg.Id) || (result.Result != R_CIO_RES_ACK)) {
                R_PRINT_Log("[CioUmMain]: R_CIO_Close check acknowledge R_OSAL_MqReceiveForTimePeriod Argument failed.");
                R_PRINT_Log(" Id(%d), Result(%d)\r\n", result.Id, result.Result);
                ret = -1;
                goto end;
            }
        }

        /* check result */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Close check result R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            goto end;
        } else {
            if ((result.Id != msg.Id) || (result.Result != R_CIO_RES_COMPLETE)) {
                R_PRINT_Log("[CioUmMain]: R_CIO_Close check result R_OSAL_MqReceiveForTimePeriod Argument failed.");
                R_PRINT_Log(" Id(%d), Result(%d)\r\n", result.Id, result.Result);
                ret = -1;
                goto end;
            }
        }

        if (result.Aux != 0) {
            /* Stop server */
            osal_ret = R_OSAL_ThreadJoin((osal_thread_handle_t)result.Aux, 0);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_ThreadJoin failed(%d)\r\n", osal_ret);
                ret = -1;
                goto end;
            }
        }

        /* close queues */
        osal_ret = R_OSAL_MqDelete(ch->MsgQueue);
        InitMqHandl(ch->MsgQueue);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_MqDelete(MsgQueue) failed(%d)\r\n", osal_ret);
            ret = -1;
            goto end;
        }
        osal_ret = R_OSAL_MqDelete(ch->RetQueue);
        InitMqHandl(ch->RetQueue);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_MqDelete(RetQueue) failed(%d)\r\n", osal_ret);
            ret = -1;
            goto end;
        }

    } else {
        R_PRINT_Log("[CioUmMain]: R_CIO_Close R_CIO_PRV_FindChannel failed.\r\n");
        ret = -1;
        goto end_no_ch_mutex;
    }

end:
    osal_ret =R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_ThsyncMutexUnlock(ChannelMutex) failed(%d)\r\n", osal_ret);
        ret = -1;
    }

end_mutex_destroy:
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(locCioUMCommonLock, (osal_milli_sec_t)TIMEOUT_MS);
    if (osal_ret != OSAL_RETURN_OK) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
        ret = -1;
    }
    x = locFreeChannel(ch);
    if (x != 0) {
        ret = -1;
    }
    osal_ret =R_OSAL_ThsyncMutexUnlock(locCioUMCommonLock);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[CioUmMain]: R_CIO_Close R_OSAL_ThsyncMutexUnlock(locCioUMCommonLock) failed(%d)\r\n", osal_ret);
        ret = -1;
    }

end_no_ch_mutex:
    return ret;
}
