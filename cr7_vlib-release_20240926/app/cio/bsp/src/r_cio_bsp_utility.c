/*************************************************************************************************************
* cio_bsp_utility_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include <string.h>

#include "r_cio_bridge.h"
#include "r_cio_bsp_utility.h"
#include "r_print_api.h"

/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Function: R_CIO_UTIL_FindDriver
 */

int R_CIO_BSP_UTIL_FindDriver(char *Name, const r_cio_DeviceEntry_t *DeviceList, int MaxNum)
{
    int i;
    int x;

    for (i = 0; i < R_CIO_MAX_DEVICENAME; i++) {
        if (Name[i] == 0) {
            break;
        }
    }
    if (i >= R_CIO_MAX_DEVICENAME) {
        R_PRINT_Log("[CioUtility]: R_CIO_BSP_UTIL_FindDriver Argument failed. Device name error.\n");
        return -1;
    }
    i = 0;
    do {
        x = strcmp(Name, DeviceList[i].Name);
        if (x == 0) {
            break;
        }
        i++;
    } while (i < MaxNum);
    if (i >= MaxNum) {
        R_PRINT_Log("[CioUtility]: R_CIO_BSP_UTIL_FindDriver Argument failed. Number of drivers in the list. MaxNum(%d)\n", MaxNum);
        i = -1;
    }
    return i;
}
