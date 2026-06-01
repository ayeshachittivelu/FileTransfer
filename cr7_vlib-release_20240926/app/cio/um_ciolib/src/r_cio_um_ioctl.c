/*************************************************************************************************************
* cio_um_ioctl
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
   Section: Global Functions
 */


/*******************************************************************************
   Function: R_CIO_IoCtl
 */

ssize_t R_CIO_IoCtl(int Id, void *Par1, void *Par2, void *Par3)
{
    R_CIO_UM_Channel_t *ch;
    r_cio_CmdMsg_t msg;
    r_cio_ResultMsg_t result;
    ssize_t ret;
    e_osal_return_t osal_ret;

    ch = R_CIO_PRV_FindChannel(Id);
    if (0 != ch) {
        /* send IOCTL command */
        msg.Id = ch->MsgId;
        ch->MsgId++;
        msg.Cmd = R_CIO_CMD_IOCTL;
        msg.Par1 = Par1;
        msg.Par2 = Par2;
        msg.Par3 = Par3;
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(ch->ChannelMutex, (osal_milli_sec_t)TIMEOUT_MS );
        if (OSAL_RETURN_OK != osal_ret){
            R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            goto clean;
        }
        osal_ret = R_OSAL_MqSendForTimePeriod(ch->MsgQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    &msg, CIO_MSG_MQ_MSG_SIZE);
        while(osal_ret != OSAL_RETURN_OK) {
            R_OSAL_ThreadSleepForTimePeriod(10);
            osal_ret = R_OSAL_MqSendForTimePeriod(ch->MsgQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    &msg, CIO_MSG_MQ_MSG_SIZE);
        }
        if (OSAL_RETURN_OK != osal_ret){
            R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_MqSendForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            goto clean;
        }

        /* check acknowledge */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (OSAL_RETURN_OK != osal_ret){
            R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl check acknowledge R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            goto clean;
        } else {
            if (result.Id != msg.Id) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl check acknowledge Wrong Id\r\n");
                ret = -1;
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                goto clean;
            }
            if (result.Result != R_CIO_RES_ACK) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl No ACK\r\n");
                ret = -1;
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                goto clean;
            }
        }

        /* check result */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (OSAL_RETURN_OK != osal_ret){
            R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl check result R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            goto clean;
        } else {
            if (result.Id != msg.Id) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl check result Wrong Id\r\n");
                ret = -1;
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                goto clean;
            }
            if (R_CIO_RES_COMPLETE != result.Result) {
                R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl No COMPLETE\r\n");
                ret = -1;
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                goto clean;
            }
        }

        ret = (ssize_t)result.Aux;
        osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
        if (OSAL_RETURN_OK != osal_ret){
            R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            ret = -1;
        }
    } else {
        R_PRINT_Log("[CioUmIoctl]: R_CIO_IoCtl R_CIO_PRV_FindChannel failed. Id(%d)\r\n", Id);
        ret = -1;
    }

clean:
    return (ssize_t)ret;
}
