/*************************************************************************************************************
* cio_canfd_connect_c
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
#include "r_cio_canfd.h"
#include "r_cio_canfd_connect.h"
#include "r_print_api.h"


/*******************************************************************************
   Section: Global (private) functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Open
 */

int R_CIO_CANFD_PRV_Open(size_t Addr, void *Par1, void *Par2, void *Par3)
{
    r_cio_canfd_prv_ChInst_t *inst;
    int x;

    x = -1;
    inst = R_CIO_CANFD_PRV_FindInstance(Addr);
    if (inst == 0) {
        R_PRINT_Log("[CioCanfdConnect]: R_CIO_CANFD_PRV_Open R_CIO_CANFD_PRV_FindInstance failed\n");
        return x;
    }
    if (R_CIO_CANFD_PRV_MAX_OPEN > inst->InUseCnt) {
        inst->InUseCnt++;
        x = (int)R_CIO_CANFD_PRV_Start(inst);
        if (0 != x) {
            R_PRINT_Log("[CioCanfdConnect]: R_CIO_CANFD_PRV_Open R_CIO_CANFD_PRV_FindInstance Start failed(%d)\n", x);
        }
    }

    return x;
}


/*******************************************************************************
   Function: R_CIO_CANFD_PRV_Close
 */

int R_CIO_CANFD_PRV_Close(size_t Addr, void *Par1, void *Par2, void *Par3)
{
    r_cio_canfd_prv_ChInst_t *inst;
    int x;

    x = -1;
    inst = R_CIO_CANFD_PRV_FindInstance(Addr);
    if (inst == 0) {
        R_PRINT_Log("[CioCanfdConnect]: R_CIO_CANFD_PRV_Close R_CIO_CANFD_PRV_FindInstance failed\n");
        return x;
    }
    if (inst->InUseCnt > 0) {
        inst->InUseCnt--;
        x = (int)R_CIO_CANFD_PRV_Stop(inst);
        if (0 != x) {
            R_PRINT_Log("[CioCanfdConnect]: R_CIO_CANFD_PRV_Close R_CIO_CANFD_PRV_Stop failed(%d)\n", x);
        }
    }
    return x;
}

