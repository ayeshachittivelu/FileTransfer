

/*******************************************************************************
  Section: Includes
*/

#include "rcar-xos/osal/r_osal.h"
#include "r_print_api.h"
#include "r_cio_bridge.h"
#include "r_cio_bsp.h"

#include "r_cio_bsp_utility.h"
#include "r_cio_bsp_board.h"

#include "r_cio_dummy.h"
#ifdef R_USE_CAN
#include "r_cio_can.h"
#endif
#ifdef R_USE_CAN
    #include "r_cio_bsp_can_config.h"
#else
#ifdef R_USE_CANFD
    #include "r_cio_canfd.h"
    #include "r_cio_bsp_canfd_config.h"
#endif
#endif

#ifdef R_USE_WM
#include "r_cio_wm.h"
#include "r_5p49v6901_api.h"
#endif

#ifdef R_USE_VIN
#include "r_rtdmac_api.h"
#include "r_cio_vin.h"
#include "r_cio_bsp_vin_config.h"
#endif

#ifdef R_USE_ETHER
#include "Eth_Types.h"
#include "r_cio_ether.h"
#endif

#ifdef R_USE_AUDIO
#include "r_audmac_api.h"
#endif

const r_cio_DeviceEntry_t locSalvatorDeviceTable[] = {
    {"Dummy:0",         0,   0, &R_CIO_DummyDriver},
#ifdef R_USE_CAN
    {"Can:0", 0xe6C30000, 218, &R_CAR_CAN_Driver},
    {"Can:1", 0xe6C38000, 219, &R_CAR_CAN_Driver},
#endif
#ifdef R_USE_CANFD
    {"CanFD:0", 0, 62, &R_CIO_CANFD_Driver},
    {"CanFD:1", 1, 62, &R_CIO_CANFD_Driver},
#endif
#ifdef R_USE_ETHER
    {"RCar-Ether:0", 0, 0, &R_CIO_ETHER_Driver},
#endif
#ifdef R_USE_WM
    {"RCar-WM:0",       0, 0, &WmDriver},
    {"RCar-WM:1",       1, 0, &WmDriver},
    {"RCar-WM:2",       2, 0, &WmDriver},
    {"RCar-WM:3",       3, 0, &WmDriver},
#endif
#ifdef R_USE_VIN
/*
    {"RCar-VIN:0", 0, 220, &R_CIO_VIN_Driver},
    {"RCar-VIN:4", 4, 206, &R_CIO_VIN_Driver},
    {"RCar-VIN:5", 5, 207, &R_CIO_VIN_Driver},
*/
    {"RCar-VIN:0", 0, 220, &R_CIO_VIN_Driver},
    {"RCar-VIN:1", 1, 221, &R_CIO_VIN_Driver},
    {"RCar-VIN:2", 2, 222, &R_CIO_VIN_Driver},
    {"RCar-VIN:3", 3, 223, &R_CIO_VIN_Driver},
    {"RCar-VIN:4", 4, 206, &R_CIO_VIN_Driver},
    {"RCar-VIN:5", 5, 207, &R_CIO_VIN_Driver},
#endif
    { 0, 0, 0, 0}
};

const r_cio_DeviceEntry_t locEbisuDeviceTable[] = {
    {"Dummy:0",         0,   0, &R_CIO_DummyDriver},
#if defined(R_USE_CAN)
    { "Can:0", 0xe6C30000, 218, &R_CAR_CAN_Driver },
#endif
#if defined(R_USE_CANFD)
    { "CanFD:0", 0, 62, &R_CIO_CANFD_Driver },
#endif
#ifdef R_USE_WM
    { "RCar-WM:0",       0, 0, &WmDriver },
    { "RCar-WM:1",       1, 0, &WmDriver },
    /* TODO CIO can't cope if the app attempts to open a driver that doesn't exist */
    { "RCar-WM:2",       2, 0, &WmDriver },
    { "RCar-WM:3",       3, 0, &WmDriver },
#endif
#ifdef R_USE_VIN
    {"RCar-VIN:4", 4, 206, &R_CIO_VIN_Driver},
#endif
    { 0, 0, 0, 0 }
};

const r_cio_DeviceEntry_t *locDeviceTable;

#ifndef DU_COMP_HDMI
#define DU_COMP_HDMI (1)
#endif

/*******************************************************************************
  Function: R_CIO_BSP_Init
*/

int R_CIO_BSP_Init(void) 
{
    r_cio_board_family_t board_family = prvGetBoardFamilyId();
    int x;
    int i;
    uint32_t ret = 0;

#if defined(R_USE_CAN) || defined(R_USE_CANFD)
    /* BSP init for CAN  */
    prvCanfdDeviceOpen();
    prvCanClkEnable();
//    prvCanEnableTransceiver();
    prvCanPFCInit();
    prvCanSoftReset(0);
#ifdef R_USE_CAN
    prvCanSoftReset(1);
#endif
#endif

#ifdef R_USE_VIN
    prvBspVinInitRoute();
    ret = R_RTDMAC_Initialize();
    if (0 != ret) {
        R_PRINT_Log("[CioCr7]: R_CIO_BSP_Init R_RTDMAC_Initialize failed(%ld).\n", ret);
    }
#endif

#ifdef R_USE_AUDIO
    ret = R_AUDMAC_Initialize();
    if (0 != ret) {
        R_PRINT_Log("[CioCr7]: R_CIO_BSP_Init R_AUDMAC_Initialize failed(%ld).\n", ret);
    }
#endif


#ifdef R_USE_WM
 #if DU_COMP_HDMI
    if (board_family == R_BOARD_SALVATOR_XS) {
        R_PRINT_Log("R_5P49V6901_Init\n");
        R_5P49V6901_Init(1);
        if (R_5P49V6901_ERR_SUCCESS != ret) {
        	R_PRINT_Log("R_5P49V6901_Init 1 error %d\n", ret);
        }
       // R_5P49V6901_Init(2);
        ret = R_5P49V6901_Init(3);
        if (R_5P49V6901_ERR_SUCCESS != ret) {
        	R_PRINT_Log("R_5P49V6901_Init 3 error %d\n", ret);
        }
    }
 #endif
#endif

    if (board_family == R_BOARD_SALVATOR_XS) {
        locDeviceTable = locSalvatorDeviceTable;
    } else if (board_family == R_BOARD_EBISU) {
        locDeviceTable = locEbisuDeviceTable;
    }

    i = 0;
    while (locDeviceTable[i].Driver != 0) {
        x = locDeviceTable[i].Driver->Init(locDeviceTable[i].BaseAddr, locDeviceTable[i].IrqNum);
        if (x != 0) {
            break;
        }
        i++;
    }
    return x;
}


/*******************************************************************************
  Function: R_CIO_BSP_DeInit
*/

int R_CIO_BSP_DeInit(void) 
{
    int x;
    int y;
    int i;

    i = 0;
    y = 0;
    while (locDeviceTable[i].Driver != 0) {
        x = locDeviceTable[i].Driver->DeInit(locDeviceTable[i].BaseAddr);
        if (x != 0) {
            y = x;
        }
        i++;
    }

#ifdef R_USE_VIN
    R_RTDMAC_DeInitialize();
#endif

    return y;
}


/*******************************************************************************
  Function: R_CIO_BSP_GetDriver
*/

int R_CIO_BSP_GetDriver(char *Name, r_cio_Driver_t **Driver, size_t *Addr) 
{
    r_cio_board_family_t board_family = prvGetBoardFamilyId();
    size_t nr_drivers = 0;
    int idx;

    if (board_family == R_BOARD_SALVATOR_XS) {
        nr_drivers = sizeof(locSalvatorDeviceTable) / sizeof(r_cio_DeviceEntry_t);
    } else if (board_family == R_BOARD_EBISU) {
        nr_drivers = sizeof(locEbisuDeviceTable) / sizeof(r_cio_DeviceEntry_t);
    }

    idx = R_CIO_BSP_UTIL_FindDriver(Name, locDeviceTable, nr_drivers);

    if (idx < 0) {
        *Driver = 0;
        *Addr   = 0;
        return -1;
    }
    *Driver = (r_cio_Driver_t *)locDeviceTable[idx].Driver;
    *Addr   = locDeviceTable[idx].BaseAddr;

    return 0;
}
