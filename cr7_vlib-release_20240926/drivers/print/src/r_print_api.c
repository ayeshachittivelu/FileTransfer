/*************************************************************************************************************
* print_api_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include <stdarg.h>
#include "rcar-xos/osal/r_osal.h"

#define PRINT_RESOURCE_ID_BSP      0x8000U
#define MUTEX_PRINT       (PRINT_RESOURCE_ID_BSP   + 98U)
#define LOC_PRINT_MUTEX_TIMEOUT   (3000U)


#define R_USE_LOG 1 /* Set here 1 when use log */

#if (R_USE_LOG)
static osal_mutex_handle_t loc_printfMutex = NULL;
#endif

void R_PRINT_Log(const char *fmt, ...)
{
#if (R_USE_LOG)
    va_list args;
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    bool isISR = true;

    osal_ret = R_OSAL_InterruptIsISRContext(&isISR);
    if ((OSAL_RETURN_OK == osal_ret) && (false == isISR))
    {
        if (NULL == loc_printfMutex)
        {
            osal_ret = R_OSAL_ThsyncMutexCreate(MUTEX_PRINT, &loc_printfMutex);
            if (OSAL_RETURN_OK != osal_ret)
            {
                printf("R_PRINT_Log MutexCreate Error osal_ret = %d\n", osal_ret);
            }
        }
        va_start(args, fmt);
        osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(loc_printfMutex, LOC_PRINT_MUTEX_TIMEOUT);
        if (OSAL_RETURN_OK == osal_ret)
        {
            vprintf(fmt, args);
            osal_ret = R_OSAL_ThsyncMutexUnlock(loc_printfMutex);
            if (OSAL_RETURN_OK != osal_ret)
            {
                printf("R_PRINT_Log MutexUnlock Error osal_ret = %d\n", osal_ret);
            }
        }
        else
        {
            printf("R_PRINT_Log MutexLock Error osal_ret = %d\n", osal_ret);
        }
        va_end(args);
    }
#endif
}
