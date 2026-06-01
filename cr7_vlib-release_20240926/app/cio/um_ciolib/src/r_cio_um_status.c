/*************************************************************************************************************
* cio_um_status
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
   Function: R_CIO_Status
 */

ssize_t R_CIO_Status(int Id, void *Par1, void *Par2, void *Par3)
{
    R_CIO_UM_Channel_t *ch;
    r_cio_CmdMsg_t msg;
    r_cio_ResultMsg_t result;
    ssize_t ret = 0;
    e_osal_return_t osal_ret;

    ch = R_CIO_PRV_FindChannel(Id);
    if (0 != ch)
    {
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(ch->ChannelMutex, (osal_milli_sec_t)TIMEOUT_MS);
        if (OSAL_RETURN_OK == osal_ret)
        {
            /* send STATUS command */
            msg.Id = ch->MsgId;
            ch->MsgId++;
            msg.Cmd = R_CIO_CMD_STATUS;
            msg.Par1 = Par1;
            msg.Par2 = Par2;
            msg.Par3 = Par3;
            osal_ret = R_OSAL_MqSendForTimePeriod(ch->MsgQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                        &msg, CIO_MSG_MQ_MSG_SIZE);
            if (OSAL_RETURN_OK != osal_ret)
            {
                R_PRINT_Log("[CioUmStatus]: R_CIO_Status R_OSAL_MqSendForTimePeriod failed(%d)\r\n", osal_ret);
                ret = -1;
            }

            if (0 == ret)
            {
                /* check acknowledge */
                do
                {
                    osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                            (void*)&result, CIO_RET_MQ_MSG_SIZE);
                } while (OSAL_RETURN_TIME == osal_ret);
                if (OSAL_RETURN_OK != osal_ret)
                {
                    R_PRINT_Log("[CioUmStatus]: R_CIO_Status check acknowledge R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
                    ret = -1;
                }
                else
                {
                    if (result.Id != msg.Id)
                    {
                        R_PRINT_Log("[CioUmStatus]: R_CIO_Status check acknowledge Wrong Id\r\n");
                        ret = -1;
                    }
                    if (result.Result != R_CIO_RES_ACK)
                    {
                        R_PRINT_Log("[CioUmStatus]: R_CIO_Status check acknowledge No ACK\r\n");
                        ret = -1;
                    }
                }
            }

            if (0 == ret)
            {
                /* check result */
                do
                {
                    osal_ret = R_OSAL_MqReceiveForTimePeriod(ch->RetQueue, (osal_milli_sec_t)TIMEOUT_MS,
                                            (void*)&result, CIO_RET_MQ_MSG_SIZE);
                } while (OSAL_RETURN_TIME == osal_ret);
                if (OSAL_RETURN_OK != osal_ret)
                {
                    R_PRINT_Log("[CioUmStatus]: R_CIO_Status check result R_OSAL_MqReceiveForTimePeriod failed(%d)\r\n", osal_ret);
                    ret = -1;
                }
                else
                {
                    if (result.Id != msg.Id)
                    {
                        R_PRINT_Log("[CioUmStatus]: R_CIO_Status check result Wrong Id\r\n");
                        ret = -1;
                    }
                    if (R_CIO_RES_COMPLETE != result.Result)
                    {
                        R_PRINT_Log("[CioUmStatus]: R_CIO_Status check result No COMPLETE\r\n");
                        ret = -1;
                    }
                    else
                    {
                        /* Success */
                        ret = (ssize_t)result.Aux;
                    }
                }
            }

            osal_ret = R_OSAL_ThsyncMutexUnlock(ch->ChannelMutex);
            if (OSAL_RETURN_OK != osal_ret)
            {
                R_PRINT_Log("[CioUmStatus]: R_CIO_Status R_OSAL_ThsyncMutexUnlock failed(%d)\r\n", osal_ret);
                ret = -1;
            }
        }
        else
        {
            R_PRINT_Log("[CioUmStatus]: R_CIO_Status R_OSAL_ThsyncMutexLockForTimePeriod failed(%d)\r\n", osal_ret);
            ret = -1;
        }
    }
    else
    {
        R_PRINT_Log("[CioUmStatus]: R_CIO_Status R_CIO_PRV_FindChannel returns (%d)\r\n", ch);
        ret = -1;
    }

    return (ssize_t)ret;
}
