/*************************************************************************************************************
* cio_canfd_ioctl_c
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
#include "r_cio_canfd_ioctl.h"
#include "r_print_api.h"


/*******************************************************************************
   Section: Global functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_IoCtl
 */

ssize_t R_CIO_CANFD_PRV_IoCtl(size_t Addr, void *Par1, void *Par2, void *Par3)
{
    r_cio_canfd_prv_ChInst_t *inst;
    ssize_t result = 0;
    r_cio_can_Ctl_t ctl = (r_cio_can_Ctl_t)Par1;

    inst = R_CIO_CANFD_PRV_FindInstance(Addr);
    if (inst == 0) {
        R_PRINT_Log("[CioCanfdIoctl]: R_CIO_CANFD_PRV_IoCtl FindInstance failed\n");
        return -1;
    }
    switch (ctl) {
    case R_CIO_CAN_REGISTER_RX_EVENT:
    {
        if (NULL != Par2) {
            inst->MqHandle = Par2;
        } else {
            R_PRINT_Log("[CioCanfdIoctl]: R_CIO_CANFD_PRV_IoCtl Par2 is NULL\n");
            result = -1;
        }

        R_CIO_CANFD_PRV_IrqSetup(Addr);

        break;
    }
    default:
        /* unknown Event */
        R_PRINT_Log("[CioCanfdIoctl]: R_CIO_CANFD_PRV_IoCtl unknowm Evnet(%d)\n", ctl);
        result = -1;
        break;
    }
    return result;
}
