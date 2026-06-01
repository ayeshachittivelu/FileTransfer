/*
============================================================================
DESCRIPTION
Tick timer API
============================================================================
                            C O P Y R I G H T
============================================================================
                      Copyright (c) 2019 - 2020
                                  by
                       Renesas Electronics (Europe) GmbH.
                           Arcadiastrasse 10
                          D-40472 Duesseldorf
                               Germany
                          All rights reserved.
============================================================================
Purpose: to be used as sample S/W

DISCLAIMER
This software is supplied by Renesas Electronics Corporation and is only
intended for use with Renesas products. No other uses are authorized. This
software is owned by Renesas Electronics Corporation and is protected under
all applicable laws, including copyright laws.
THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
Renesas reserves the right, without notice, to make changes to this software
and to discontinue the availability of this software. By using this software,
you agree to the additional terms and conditions found by accessing the
following link:
http://www.renesas.com/disclaimer *
Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
*/

#ifndef _R_TICK_API_H_
#define _R_TICK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup  Tickgroup  Tick driver
 * @ingroup drivers
 * @brief    Tick driver interface
 *
 * This driver is OS dependent and provide an interface to the timer
 */

/**
 * \defgroup TickApi   Tick user API
 * \ingroup   Tickgroup
 * @brief tick user functions
 * @{
 */

/**
 * Return the current time in ms
 *
 * This function depends on the device and the OS used.
 *
 * @retval   - time in ms
 */
uint32_t R_TICK_GetTimeMs(void);

/**
 * Return the current time in us
 * 
 * This function depends on the device and the OS used.
 *
 * @retval   - time in us
 */
uint32_t R_TICK_GetTimeUs(void);

/**
 * Delay that does not put the CPU into low power states
 *
 * @param[in] us - microseconds to delay
 */
void R_TICK_DelayUs(uint32_t us);

/** @} */ /* end of group TickApi */

#ifdef __cplusplus
}
#endif

#endif /* _R_TICK_API_H_ */

