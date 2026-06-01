/*
 * Copyright (c) 2017-2020, Renesas Electronics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * \defgroup  gpiogroup GPIO driver
 * @ingroup drivers
 * @{
 * @}
 */

/**
 * \defgroup GPIOapi   API
 * @ingroup gpiogroup
 * @{
 */

/**
 * Set a level of a pin that is set as a GPIO output with R_GPIO_SetGpio()
 * @param[in] Block     - General Output Register (OUTDTn) index.
 * @param[in] Bit       - Bit within OUTDTn.
 * @param[in] Level     - 1=high, 0=low.
 */
void R_GPIO_SetOutput(uint8_t Block, uint8_t Bit, uint8_t Level);

/**
 * Get a level of a pin that is set as a GPIO input with R_GPIO_SetGpio()
 * @param[in] Block     - General Output Register (INDTn) index.
 * @param[in] Bit       - Bit within INDTn.
 * @retval 0 if pin is high, 1 if pin is low
 */
uint8_t R_GPIO_GetInput(uint8_t Block, uint8_t Bit);

/**
 * Set a pin to be a GPIO input or output
 * @param[in] Block     - General Input/Output Switching Register (INOUTSELn) index.
 * @param[in] Bit       - Bit within INOUTSELn.
 * @param[in] Output    - 1=output, 0=input.
 */
void R_GPIO_SetGpio(uint8_t Block, uint8_t Bit, uint8_t Output);

/** @} api */

#endif /* __GPIO_H__ */
