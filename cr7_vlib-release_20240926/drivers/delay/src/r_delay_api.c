/*************************************************************************************************************
* delay_api_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include "r_delay_api.h"

void soft_delay(const uint32_t Loop)
{
    uint32_t i;
    for (i = 0; i < Loop; i++) {
        __asm("nop");
    }
}

