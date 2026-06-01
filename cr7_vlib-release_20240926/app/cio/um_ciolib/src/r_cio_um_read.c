/*************************************************************************************************************
* cio_um_read
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
   Function: R_CIO_Read
 */


ssize_t R_CIO_Read(int Id, char *Buffer, size_t Len)
{
    R_CIO_UM_Channel_t *ch;
    r_cio_CmdMsg_t msg;
    r_cio_ResultMsg_t result;
    ssize_t len;
    ssize_t total_len;
    ssize_t trans_len;
    e_osal_return_t osal_ret;

    len = -1;
    total_len =  0;
    ch = R_CIO_PRV_FindChannel(Id);
    if (ch) {

        trans_len = Len - total_len;
        /* send read command */
        msg.Id = ch->MsgId;
        ch->MsgId++;
        msg.Cmd = R_CIO_CMD_READ;
        msg.Par1 = Buffer + total_len;
        msg.Par2 = (void *)trans_len;
        msg.Par3 = 0;
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(ch->ChannelMutex, (osal_milli_sec_t)TIMEOUT_MS );
        if (osal_ret != OSAL_RETURN_OK) {
            R_PRINT_Log("[CioUmRead]: R_CIO_Read R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (osal_ret != OSAL_RETURN_OK) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            return -1;
        }

        osal_ret = R_OSAL_MqSendForTimePeriod(ch->MsgQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    &msg, CIO_MSG_MQ_MSG_SIZE);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmRead]: R_CIO_Read R_OSAL_MqSendForTimePeriod failed(%d)\r\n", osal_ret);
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            return -1;
        }

        /* check acknowledge */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmRead]: R_CIO_Read check acknowledge R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read check acknowledge R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            return -1;
        } else {
            if (result.Id != msg.Id) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read check acknowledge Wrong Id\r\n");
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmRead]: R_CIO_Read check acknowledge R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                return -1;
            }
            if (result.Result != R_CIO_RES_ACK) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read check acknowledge No ACK\r\n");
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmRead]: R_CIO_Read check acknowledge R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                return -1;
            }
        }

        /* check result */
        do {
            osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                    (void*)&result, CIO_RET_MQ_MSG_SIZE);
        } while(OSAL_RETURN_TIME == osal_ret);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmRead]: R_CIO_Read check result R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read check result R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            }
            return -1;
        } else {
            if (result.Id != msg.Id) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read check result Wrong Id\r\n");
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmRead]: R_CIO_Read check result R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                return -1;
            }
            if (result.Result != R_CIO_RES_COMPLETE) {
                R_PRINT_Log("[CioUmRead]: R_CIO_Read check result No COMPLETE\r\n");
                osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
                if (OSAL_RETURN_OK != osal_ret) {
                    R_PRINT_Log("[CioUmRead]: R_CIO_Read check result R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                }
                return -1;
            }
        }

        len = (ssize_t)result.Aux;
        total_len += len;

        len = total_len;
        osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[CioUmRead]: R_CIO_Read R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
            return -1;
        }

    } else {
        R_PRINT_Log("[CioUmRead]: R_CIO_Read R_CIO_PRV_FindChannel failed(%d)\r\n", ch);
    }
    return len;
}
