/*************************************************************************************************************
* cio_canfd_helper_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Section: Includes
 */

#include "rcar-xos/osal/r_osal.h"
#include "r_cio_canfd_main.h"
#include "r_cio_canfd_helper.h"


/*******************************************************************************
   Section: Global (private) helper functions
 */

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_UpdateReg
 */
void R_CIO_CANFD_PRV_UpdateReg(size_t Reg, uint32_t Mask, uint32_t Val)
{
    uint32_t data = R_CIO_CANFD_READ32(Reg);

    data &= ~Mask;
    data |= (Val & Mask);
    R_CIO_CANFD_WRITE32(Reg, data);
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_ReadReg
 */
uint32_t R_CIO_CANFD_PRV_ReadReg(size_t Base, uint32_t Offset)
{
    return R_CIO_CANFD_READ32(Base + Offset);
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_WriteReg
 */
void R_CIO_CANFD_PRV_WriteReg(size_t Base, uint32_t Offset, uint32_t Val)
{
    R_CIO_CANFD_WRITE32(Base + Offset, Val);
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_SetBit
 */
void R_CIO_CANFD_PRV_SetBit(size_t Base, uint32_t Reg, uint32_t Val)
{
    R_CIO_CANFD_PRV_UpdateReg(Base + Reg, Val, Val);
}

/*******************************************************************************
   Function: R_CIO_CANFD_PRV_ClearBit
 */
void R_CIO_CANFD_PRV_ClearBit(size_t Base, uint32_t Reg, uint32_t Val)
{
    R_CIO_CANFD_PRV_UpdateReg(Base + Reg, Val, 0);
}
