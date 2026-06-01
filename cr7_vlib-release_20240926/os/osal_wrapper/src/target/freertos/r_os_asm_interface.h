/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_OS_ASM_INTERFACE_H
#define R_OS_ASM_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>

/**
  Invalidate instruction cache.

  Parameters:
  Start       - Start address of the memory block
  Size        - Size of the memory block
*/
void R_OS_ASMIF_InvICache(size_t Start, size_t Size);

/**
  Invalidate data cache.

  Parameters:
  Start       - Start address of the memory block
  Size        - Size of the memory block
*/
void R_OS_ASMIF_InvDCache(size_t Start, size_t Size);

/**
  Invalidate all cache.

  Parameters:
  Start       - Start address of the memory block
  Size        - Size of the memory block
*/
void R_OS_ASMIF_InvAllCache(size_t Start, size_t Size);

/**
  Flush the instruction cache.

  Parameters:
  Start       - Start address of the memory block
  Size        - Size of the memory block
*/
void R_OS_ASMIF_FlushICache(size_t Start, size_t Size);

/**
  Flush the data cache.

  Parameters:
  Start       - Start address of the memory block
  Size        - Size of the memory block
*/
void R_OS_ASMIF_FlushDCache(size_t Start, size_t Size);

/**
  Flush all cache.

  Parameters:
  Start       - Start address of the memory block
  Size        - Size of the memory block
*/
void R_OS_ASMIF_FlushAllCache(size_t Start, size_t Size);    


#ifdef __cplusplus
}
#endif

#endif /* R_OS_ASM_INTERFACE_H */
