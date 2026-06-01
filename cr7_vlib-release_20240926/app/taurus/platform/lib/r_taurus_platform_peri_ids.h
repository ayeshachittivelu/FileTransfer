/*************************************************************************************************************
* taurus_platform_peri_ids
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef R_TAURUS_PLATFORM_PERI_ID_H
#define R_TAURUS_PLATFORM_PERI_ID_H

typedef enum {
    R_TAURUS_INVALID_PERI_ID = 0,
#if defined(R_TAURUS_ENABLE_VIRTDEV)
    R_TAURUS_VIRTDEV_PERI_ID,
#endif
#if defined(R_TAURUS_ENABLE_RVGC)
    R_TAURUS_RVGC_PERI_ID,
#endif
#if defined(R_TAURUS_ENABLE_CAMERA)
    R_TAURUS_CAMERA_PERI_ID,
#endif
#if defined(R_TAURUS_ENABLE_CAN)
    R_TAURUS_CAN_PERI_ID,
#endif
#if defined(R_TAURUS_ENABLE_ETHER)
    R_TAURUS_ETHER_PERI_ID,
#endif
#if defined(R_TAURUS_ENABLE_COMCH)
    R_TAURUS_COMCH_PERI_ID,
#endif
    R_TAURUS_LAST_PERI_ID
} R_TAURUS_Peripheral_Id_t;

#endif
