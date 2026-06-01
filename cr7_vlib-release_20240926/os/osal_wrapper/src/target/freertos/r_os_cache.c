/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include "r_os_asm_interface.h"
#include "target/freertos/r_osal_common.h"

int32_t R_OSAL_CacheFlush(size_t Start, size_t Size, size_t Cache)
{
    if (Cache == R_OSAL_CACHE_I) {
        R_OS_ASMIF_FlushICache(Start, Size);
    } else if (Cache == R_OSAL_CACHE_D) {
        R_OS_ASMIF_FlushDCache(Start, Size);
    } else if (Cache == R_OSAL_CACHE_ALL) {
        R_OS_ASMIF_FlushAllCache(Start, Size);
    } else {
        return -1;
    }
    return 0;
}

int32_t R_OSAL_CacheInvalidate(size_t Start, size_t Size, size_t Cache)
{
    if (Cache == R_OSAL_CACHE_I) {
        R_OS_ASMIF_InvICache(Start, Size);
    } else if (Cache == R_OSAL_CACHE_D) {
        R_OS_ASMIF_InvDCache(Start, Size);
    } else if (Cache == R_OSAL_CACHE_ALL) {
        R_OS_ASMIF_InvAllCache(Start, Size);
    } else {
        return -1;
    }
    return 0;
}
