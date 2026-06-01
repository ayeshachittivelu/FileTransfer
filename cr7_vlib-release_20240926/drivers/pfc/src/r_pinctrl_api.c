/*************************************************************************************************************
* pfc_api_c
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include "r_pinctrl_api.h"

/* PFC */
#define	PFC_PMMR			0xE6060000	/* R/W	32	LSI Multiplexed Pin Setting Mask Register */
#define PFC_IPSR(x)			(0xE6060200 + ((x) * 4))
#define PFC_GPSR(x)			(0xE6060100 + ((x) * 4))
#define	PFC_DRVCTRL(x)		(0xE6060300 + ((x) * 4))	/* DRV control registers */
#define	PFC_POCCTRL0		0xE6060380	/* R/W	32	POC control register0 */
#define	PFC_TDSELCTRL0		0xE60603C0	/* R/W	32	TDSEL control register0 */
#define	PFC_IOCTRL			0xE60603E0	/* R/W	32	IO cell control for IICDVFS */
#define	PFC_FUSEMON			0xE60603E4	/* R	32	Fuse Monitor register0 */
#define	PFC_PUEN(x)			(0xE6060400 + ((x) * 4))	/* Pin pull-enable register 0 */
#define	PFC_PUD(x)			(0xE6060440 + ((x) * 4))	/* Pin pull-up/down control register 0 */
#define PFC_MODSEL(x)		(0xE6060500 + ((x) * 4))

#define BIT(nr)         (1UL << (nr))

/*********************** RCAR_GPIO *************************/
#define	RCAR_GPIO_INOUTSEL0	0xE6050004	/* R/W	32	General input/output switching register 0 */
#define	RCAR_GPIO_INOUTSEL1	0xE6051004	/* R/W	32	General input/output switching register 1 */
#define	RCAR_GPIO_INOUTSEL2	0xE6052004	/* R/W	32	General input/output switching register 2 */
#define	RCAR_GPIO_INOUTSEL3	0xE6053004	/* R/W	32	General input/output switching register 3 */
#define	RCAR_GPIO_INOUTSEL4	0xE6054004	/* R/W	32	General input/output switching register 4 */
#define	RCAR_GPIO_INOUTSEL5	0xE6055004	/* R/W	32	General input/output switching register 5 */
#define	RCAR_GPIO_INOUTSEL6	0xE6055404	/* R/W	32	General input/output switching register 6 */
#define	RCAR_GPIO_INOUTSEL7	0xE6055804	/* R/W	32	General input/output switching register 7 */

#define	RCAR_GPIO_INDT0		0xE605000C	/* R	32	General input register 0 */
#define	RCAR_GPIO_INDT1		0xE605100C	/* R	32	General input register 1 */
#define	RCAR_GPIO_INDT2		0xE605200C	/* R	32	General input register 2 */
#define	RCAR_GPIO_INDT3		0xE605300C	/* R	32	General input register 3 */
#define	RCAR_GPIO_INDT4		0xE605400C	/* R	32	General input register 4 */
#define	RCAR_GPIO_INDT5		0xE605500C	/* R	32	General input register 5 */
#define	RCAR_GPIO_INDT6		0xE605540C	/* R	32	General input register 6 */
#define	RCAR_GPIO_INDT7		0xE605580C	/* R	32	General input register 7 */

#define	RCAR_GPIO_OUTDT0	0xE6050008	/* R/W	32	General output register 0 */
#define	RCAR_GPIO_OUTDT1	0xE6051008	/* R/W	32	General output register 1 */
#define	RCAR_GPIO_OUTDT2	0xE6052008	/* R/W	32	General output register 2 */
#define	RCAR_GPIO_OUTDT3	0xE6053008	/* R/W	32	General output register 3 */
#define	RCAR_GPIO_OUTDT4	0xE6054008	/* R/W	32	General output register 4 */
#define	RCAR_GPIO_OUTDT5	0xE6055008	/* R/W	32	General output register 5 */
#define	RCAR_GPIO_OUTDT6	0xE6055408	/* R/W	32	General output register 6 */
#define	RCAR_GPIO_OUTDT7	0xE6055808	/* R/W	32	General output register 7 */

static const uint32_t RCAR_GPIO_INOUTSEL[8]= {
	RCAR_GPIO_INOUTSEL0,
	RCAR_GPIO_INOUTSEL1,
	RCAR_GPIO_INOUTSEL2,
	RCAR_GPIO_INOUTSEL3,
	RCAR_GPIO_INOUTSEL4,
	RCAR_GPIO_INOUTSEL5,
	RCAR_GPIO_INOUTSEL6,
	RCAR_GPIO_INOUTSEL7,
};

static const uint32_t RCAR_GPIO_INDT[8]= {
	RCAR_GPIO_INDT0,
	RCAR_GPIO_INDT1,
	RCAR_GPIO_INDT2,
	RCAR_GPIO_INDT3,
	RCAR_GPIO_INDT4,
	RCAR_GPIO_INDT5,
	RCAR_GPIO_INDT6,
	RCAR_GPIO_INDT7,
};

static const uint32_t RCAR_GPIO_OUTDT[8]= {
	RCAR_GPIO_OUTDT0,
	RCAR_GPIO_OUTDT1,
	RCAR_GPIO_OUTDT2,
	RCAR_GPIO_OUTDT3,
	RCAR_GPIO_OUTDT4,
	RCAR_GPIO_OUTDT5,
	RCAR_GPIO_OUTDT6,
	RCAR_GPIO_OUTDT7,
};

static void writel(const uint32_t Value, const uintptr_t Address)
{
       *((volatile unsigned int*) Address)  = Value;
}

static uint32_t readl(const uintptr_t Address)
{
    return *((volatile unsigned int*)Address);
}

static inline void setbit_l(uint32_t addr, uint32_t pos)
{
	uint32_t val = readl(addr);

	writel(val |= BIT(pos), addr);
}

static inline uint32_t getbit_l(uint32_t addr, uint32_t pos)
{
	return !!(readl(addr) & BIT(pos));
}

static inline void clearbit_l(uint32_t addr, uint32_t pos)
{
	uint32_t val = readl(addr);

	writel(val &= ~BIT(pos), addr);
}

static void pfcWrite(uint32_t addr, uint32_t val)
{
	writel(~val, PFC_PMMR);
	writel(val, addr);
}

static void pfcSetGPSR(uint8_t gpio, uint8_t block, uint8_t pos)
{
	uint32_t val = readl(PFC_GPSR(block));

	val = gpio ? val & ~BIT(pos) : val | BIT(pos);
	pfcWrite(PFC_GPSR(block), val);
}

static void pfcSetPeripheral(uint8_t block, uint8_t pos)
{
	pfcSetGPSR(0, block, pos);
}

static void pfcSetIPSR(uint8_t ipsr, uint8_t val, uint8_t pos)
{
	uint32_t reg = readl(PFC_IPSR(ipsr));

	reg &= ~(0xF << pos);
	reg |= (val << pos);
	pfcWrite(PFC_IPSR(ipsr), reg);
}

static void pfcSetModsel(uint8_t modsel, uint32_t val, uint32_t mask)
{
	uint32_t reg = readl(PFC_MODSEL(modsel));

	reg &= ~mask;
	reg |= val;
	pfcWrite(PFC_MODSEL(modsel), reg);
}

void gpioSetOutput(uint32_t block, uint32_t bit, uint32_t lvl)
{
	if (lvl)
		setbit_l(RCAR_GPIO_OUTDT[block], bit);
	else
		clearbit_l(RCAR_GPIO_OUTDT[block], bit);
}

void gpioSetInput(uint32_t block, uint32_t bit)
{
	setbit_l(RCAR_GPIO_INDT[block], bit);
}

uint32_t gpioGetInput(uint32_t block, uint32_t bit)
{
	return getbit_l(RCAR_GPIO_INDT[block], bit);
}

void pfcSetGPIO(uint8_t block, uint8_t pos, uint8_t output)
{
	if (output)
		setbit_l(RCAR_GPIO_INOUTSEL[block], pos);
	else
		clearbit_l(RCAR_GPIO_INOUTSEL[block], pos);

	pfcSetGPSR(1, block, pos);
}

void pfcConfig(const RcarPin_t *pin)
{
    if(NULL == pin)
        return;

    pfcSetPeripheral(pin->gpsr, pin->gpsrPos);
    pfcSetIPSR(pin->ipsr, pin->ipsrVal, pin->ipsrPos);

    if (pin->modsel == 0xFF)
        return;
    pfcSetModsel(pin->modsel, pin->modselVal, pin->modselMask);
}

