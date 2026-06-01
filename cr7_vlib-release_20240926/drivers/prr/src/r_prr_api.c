/*************************************************************************************************************
* prr_api_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include "r_prr_api.h"

/* Product Register */
#define PRR                 (0xFFF00044U)
#define PRR_PRODUCT_MASK    (0x00007F00U)
#define PRR_CUT_MASK        (0x000000FFU)
#define PRR_PRODUCT_H3      (0x00004F00U)           /* R-Car H3   */
#define PRR_PRODUCT_M3      (0x00005200U)           /* R-Car M3-W */
#define PRR_PRODUCT_V3M     (0x00005400U)           /* R-Car V3M  */
#define PRR_PRODUCT_M3N     (0x00005500U)           /* R-Car M3-N */
#define PRR_PRODUCT_V3H     (0x00005600U)           /* R-Car V3H  */
#define PRR_PRODUCT_E3      (0x00005700U)           /* R-Car E3  */
#define PRR_PRODUCT_D3      (0x00005800U)           /* R-Car D3  */
#define PRR_PRODUCT_10      (0x00U)         /*   Ver.1.0  */
#define PRR_PRODUCT_11      (0x01U)         /*   Ver.1.1  */
#define PRR_PRODUCT_13      (0x03U)         /*   Ver.1.3  */
#define PRR_PRODUCT_20      (0x10U)         /*   Ver.2.0  */
#define PRR_PRODUCT_21      (0x11U)         /*   Ver.2.1  */
#define PRR_PRODUCT_30      (0x20U)         /*   Ver.3.0  */

static uint32_t loc_RegRead32(uintptr_t addr)
{
    return *((volatile uint32_t *)addr);
}

r_prr_DevName_t R_PRR_GetDevice(void)
{
    r_prr_DevName_t dev;
    uint32_t        reg_val;

    reg_val = loc_RegRead32(PRR);
    reg_val = reg_val & PRR_PRODUCT_MASK;

    switch (reg_val) {
    case PRR_PRODUCT_H3:
        dev = R_PRR_RCARH3;
        break;
    case PRR_PRODUCT_M3:
        dev = R_PRR_RCARM3W;
        break;
    case PRR_PRODUCT_M3N:
        dev = R_PRR_RCARM3N;
        break;
    case PRR_PRODUCT_E3:
        dev = R_PRR_RCARE3;
        break;
    case PRR_PRODUCT_D3:
        dev = R_PRR_RCARD3;
        break;
    case PRR_PRODUCT_V3M:
        dev = R_PRR_RCARV3M;
        break;
    case PRR_PRODUCT_V3H:
        dev = R_PRR_RCARV3H;
        break;
    default:
        dev = R_PRR_RCARLAST;
        break;
    }

    return dev;
}

r_prr_Rev_t R_PRR_GetRevision(void)
{
    r_prr_Rev_t rev;
    uint32_t    reg_val;

    reg_val = loc_RegRead32(PRR);
    reg_val = reg_val & PRR_CUT_MASK;

    switch (reg_val) {
    case PRR_PRODUCT_10:
        rev = R_PRR_REV10;
        break;
    case PRR_PRODUCT_11:
        rev = R_PRR_REV11;
        break;
    case PRR_PRODUCT_13:
        rev = R_PRR_REV13;
        break;
    case PRR_PRODUCT_20:
        rev = R_PRR_REV20;
        break;
    case PRR_PRODUCT_21:
        rev = R_PRR_REV21;
        break;
    case PRR_PRODUCT_30:
        rev = R_PRR_REV30;
        break;
    default:
        rev = R_PRR_RCARLAST;
        break;
    }

    return rev;
}