/*************************************************************************************************************
* taurus_pf_ros_cr7_r-carx3
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#include "r_taurus_bridge.h"
#include "r_perserv_api.h"
#include "r_taurus_platform.h"
#include "r_taurus_platform_peri_ids.h"

#if defined(R_TAURUS_ENABLE_VIRTDEV)
#include "r_virtdev_sv_api.h"
#endif

#if defined(R_TAURUS_ENABLE_RVGC)
#include "r_rvgc_sv_api.h"
#endif

#if defined(R_TAURUS_ENABLE_CAMERA)
#include "r_camera_sv_api.h"
#endif

#if defined(R_TAURUS_ENABLE_CAN)
#include "r_can_sv_api.h"
#endif

#if defined(R_TAURUS_ENABLE_ETHER)
#include "r_ether_sv_api.h"
#endif

#if defined(R_TAURUS_ENABLE_COMCH)
#include "r_comch_sv_api.h"
#endif

R_TAURUS_PeripheralServer_t *R_TAURUS_PF_Devtable[] = {

#if defined(R_TAURUS_ENABLE_VIRTDEV)
    [R_TAURUS_VIRTDEV_PERI_ID] = &R_Virtdev_Server,
#endif

#if defined(R_TAURUS_ENABLE_RVGC)
    [R_TAURUS_RVGC_PERI_ID] = &R_Rvgc_Server,
#endif

#if defined(R_TAURUS_ENABLE_CAMERA)
    [R_TAURUS_CAMERA_PERI_ID] = &R_Camera_Server,
#endif

#if defined(R_TAURUS_ENABLE_CAN)
    [R_TAURUS_CAN_PERI_ID] = &R_Can_Server,
#endif

#if defined(R_TAURUS_ENABLE_ETHER)
    [R_TAURUS_ETHER_PERI_ID] = &R_Ether_Server,
#endif

#if defined(R_TAURUS_ENABLE_COMCH)
    [R_TAURUS_COMCH_PERI_ID] = &R_Comch_Server,
#endif
    0
};


R_TAURUS_PeripheralServer_t * *R_PF_GetDeviceTable(int Guest)
{
    return R_TAURUS_PF_Devtable;
}
