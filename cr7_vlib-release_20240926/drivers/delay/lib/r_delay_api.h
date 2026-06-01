/*************************************************************************************************************
* delay_api_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef _R_DELAY_API_H_
#define _R_DELAY_API_H_

#include <stdint.h>

extern void soft_delay(const uint32_t Loop);

/* delay_us(x) is currently implemented with a NOP loop.
 * The value of 400 of the coefficient was calculated based on the result
 * of measurment in H3-CR7.  */
#define delay_us(X) soft_delay(400 * X)

/* 1 RCLK cycle wait */
#define R_DELAY_ONE_RCLK_CYCLE_IN_US (31) /* 31us */

#endif /* _R_DELAY_API_H_ */
