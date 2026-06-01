/*************************************************************************************************************
* cio_bsp_r-carx3_cr7_vin
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
  Section: Includes
*/
#include "rcar-xos/osal/r_osal.h"

#ifdef R_USE_VIN
#include "r_cio_bsp_vin_config.h"
//#include "r_i2c_api.h"
//#include "r_adv7482_api.h"
#include "r_csi2_api.h"
#include "r_vin_api.h"
#endif

/*******************************************************************************
  Section: Local functions
*/
#ifdef R_USE_VIN
void prvBspVinInitRoute(void)
{
}

void prvBspVinDeInitRoute(void)
{
}
#endif
