/*************************************************************************************************************
* cio_canfd_read_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Section: Includes
 */

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_bridge.h"
#include "r_cio_canfd_main.h"
#include "r_cio_canfd_read.h"
#include "r_print_api.h"
#include <string.h>

/*******************************************************************************
   Section: Global private functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Read
 */

ssize_t R_CIO_CANFD_PRV_Read(size_t Addr, void *Par1, void *Par2, void *Par3)
{
    r_cio_canfd_prv_ChInst_t *inst;
    r_cio_can_Msg_t          *buf;
    ssize_t len;


    inst = R_CIO_CANFD_PRV_FindInstance(Addr);
    if (inst == 0) {
        R_PRINT_Log("[CioCanfdRead]: R_CIO_CANFD_PRV_Read FindInstance failed\n");
        return -1;
    }

    buf = (r_cio_can_Msg_t *)Par1;
    /* Check the correct message length */
    if (0 != ((ssize_t)Par2 % sizeof(r_cio_can_Msg_t))) {
        R_PRINT_Log("[CioCanfdRead]: R_CIO_CANFD_PRV_Read Message length failed(%d)\n", (ssize_t)Par2);
        return -2;
    }
    len = (ssize_t)Par2 / sizeof(r_cio_can_Msg_t);

    while (len > 0) {
        if (inst->RxStrt != inst->RxStop) {

            /* *buf = *inst->RxStrt */
            (void)memcpy((void *)buf, (void *)inst->RxStrt, sizeof(r_cio_can_Msg_t));
            inst->RxStrt++;
            if (inst->RxStrt > inst->RxEob) {
                inst->RxStrt = inst->RxBuf;
            }
            buf++;
            len--;
        } else {
            break;
        }
    }

    len = (ssize_t)Par2 - (len * sizeof(r_cio_can_Msg_t));
    return len;
}
