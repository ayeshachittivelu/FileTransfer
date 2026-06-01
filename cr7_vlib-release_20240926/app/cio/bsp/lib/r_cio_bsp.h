/*************************************************************************************************************
* cio_bsp
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_CIO_BSP_H
#define R_CIO_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/**@addtogroup cio_bsp CIO BSP User Interaction
 * @ingroup cio
 * @{
 */

/*******************************************************************************
   Section: Global Functions
 */
#include <stdint.h>
#include "r_cio_bridge.h"

/*******************************************************************************
   Function: R_CIO_BSP_Init

   Init the BSP hardware for CIO.

   Parameters:
   void

   Returns:
   ==0 - init was successfull
   !=0 - init failed
 */

int R_CIO_BSP_Init(void);


/*******************************************************************************
   Function: R_CIO_BSP_DeInit

   DeInit the BSP hardware for CIO.

   Parameters:
   void

   Returns:
   ==0 - init was successfull
   !=0 - init failed
 */

int R_CIO_BSP_DeInit(void);


/*******************************************************************************
   Function: R_CIO_BSP_GetDriver

   Get driver info and base address of the assigned peripheral from the BSP

   Parameters:
   Name       - Info for the BSP to identify driver & baseaddress
   , Driver     - Pointer to a driver structure to be filled by the BSP
   Addr       - Pointer to a baseaddress to be filled by the BSP

   Returns:
   ==0 - driver found
   !=0 - NG
 */
int R_CIO_BSP_GetDriver(char *Name, r_cio_Driver_t **Driver, size_t *Addr);

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_BSP_H */
