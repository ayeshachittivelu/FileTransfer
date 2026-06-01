/*************************************************************************************************************
* cio_bsp_canfd
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef CIO_BSP_CANFD_CONFIG_H
#define CIO_BSP_CANFD_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
   Section: private defines
 */


#define PRIV_RCAR_CAN_TIMEOUT           (0x1000000)


void prvCanEnableTransceiver(void);
void prvCanPFCInit(void);
void prvCanClkEnable(void);
void prvCanSoftReset(uint32_t Unit);
void prvCanfdDeviceOpen(void);
void prvCanfdDeviceClose(void);

#ifdef __cplusplus
}
#endif

#endif /* CIO_BSP_CANFD_CONFIG_H */
