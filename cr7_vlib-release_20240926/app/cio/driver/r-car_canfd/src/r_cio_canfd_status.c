/*************************************************************************************************************
* cio_canfd_status_c
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
#include "r_cio_canfd_status.h"
#include "r_print_api.h"


/*******************************************************************************
   Section: Global functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Status
 */

ssize_t R_CIO_CANFD_PRV_Status(size_t Addr, void *Par1, void *Par2, void *Par3)
{
    r_cio_canfd_prv_ChInst_t *inst;
    ssize_t result;

    inst = R_CIO_CANFD_PRV_FindInstance(Addr);
    if (inst == 0) {
        R_PRINT_Log("[CioCanfdStatus]: R_CIO_CANFD_PRV_Status R_CIO_CANFD_PRV_FindInstance failed\n");
        return -1;
    }
    result = (ssize_t)inst->Error;
    inst->Error = 0;
    return result;
}
