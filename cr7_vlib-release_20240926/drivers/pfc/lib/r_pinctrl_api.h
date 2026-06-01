/*************************************************************************************************************
* pfc_api_h
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_PINCTRL_API_H_
#define R_PINCTRL_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

typedef struct RcarPin {
    const char *name;
	uint8_t	ipsr;
	uint8_t	ipsrVal;
	uint8_t	ipsrPos;
	uint8_t	gpsr;
	uint8_t	gpsrPos;
    uint8_t		modsel;
	uint32_t	modselVal;
	uint32_t	modselMask;
} RcarPin_t;

void pfcConfig(const RcarPin_t *pin);
void gpioSetOutput(uint32_t block, uint32_t bit, uint32_t lvl);
void gpioSetInput(uint32_t block, uint32_t bit);
uint32_t gpioGetInput(uint32_t block, uint32_t bit);
void pfcSetGPIO(uint8_t block, uint8_t pos, uint8_t output);



#ifdef __cplusplus
}
#endif

#endif /* _R_PINCTRL_H_ */