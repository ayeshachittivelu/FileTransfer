/*************************************************************************************************************
* cio_bsp_utility_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
   Title: Server Module Interface

 */

/*******************************************************************************
   Section: Global Defines

   Definition of all global defines.
 */

#undef EXTERN
#ifndef UTILITY_GLOBALS
#define EXTERN extern
#else
#define EXTERN
#endif

/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Function: R_CIO_UTIL_FindDriver

   Find a driver in a driver list

   Parameters:
   Name       - Name of driver to be found
   Devicelist - List of device entries
   MaxNum     - Number of drivers in the list

   Returns:
   => 0       - Index in the list
   <  0       - Failed
 */

int R_CIO_BSP_UTIL_FindDriver(char *Name, const r_cio_DeviceEntry_t *DeviceList, int MaxNum);


#ifdef __cplusplus
}
#endif

#endif /* UTILITY_H */
