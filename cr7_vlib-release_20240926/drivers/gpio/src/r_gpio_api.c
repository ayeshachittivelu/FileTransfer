/*
 * Copyright (c) 2017-2021, Renesas Electronics Corporation
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Basic driver to set GPIO outputs and read GPIO inputs.
 *
 * WARNING: This driver is not thread safe!
 *  Some individual PFC registers deal with multiple GPIO pins.
 *  The PFC PMMR register must be set for each access to PFC registers.
 */

#include <stdio.h>
#include "r_cpg_api.h"
#include "r_gpio_api.h"
#include "r_pinctrl_api.h"

/* PFC */
#define PFC_PMMR            0xE6060000U    /* LSI Multiplexed Pin Setting Mask Register */
#define PFC_GPSR(x)         (0xE6060100U + ((x) * 4))

/* GPIO */
#define GPIO_IOINTSEL       0x00    /* General IO/interrupt switching register */
#define GPIO_INOUTSEL       0x04    /* General input/output switching register */
#define GPIO_OUTDT          0x08    /* General output register */
#define GPIO_INDT           0x0C    /* General input register */
#define GPIO_INTDT          0x10    /* Interrupt display register */
#define GPIO_INTCLR         0x14    /* Interrupt clear register */
#define GPIO_INTMSK         0x18    /* Interrupt mask register */
#define GPIO_MSKCLR         0x1C    /* Interrupt mask clear register */
#define GPIO_POSNEG         0x20    /* Positive/negative logic select register */
#define GPIO_EDGLEVEL       0x24    /* Edge/level select register */
#define GPIO_FILONOFF       0x28    /* Chattering prevention on/off register */
#define GPIO_INTMSKS        0x38    /* Interrupt sub mask register */
#define GPIO_MSKCLRS        0x3C    /* Interrupt sub mask clear register */
#define GPIO_OUTDTSEL       0x40    /* Output data select register */
#define GPIO_OUTDTH         0x44    /* Output data high register */
#define GPIO_OUTDTL         0x48    /* Output data low register */
#define GPIO_BOTHEDGE       0x4C    /* One edge/both edge select register */

static const uintptr_t GPIO_BASE[8]= {
    0xE6050000,
    0xE6051000,
    0xE6052000,
    0xE6053000,
    0xE6054000,
    0xE6055000,
    0xE6055400,
    0xE6055800,
};

static void loc_WriteReg(uintptr_t Address, uint32_t Value)
{
    *((volatile uint32_t *)Address) = Value;
}

static uint32_t loc_ReadReg(uintptr_t Address)
{
    return *((volatile uint32_t *)Address);
}

static void loc_SetBit(uintptr_t Address, uint32_t Pos)
{
    uint32_t val = loc_ReadReg(Address);

    loc_WriteReg(Address, val | (1 << Pos));
}

static void loc_ClearBit(uintptr_t Address, uint32_t Pos)
{
    uint32_t val = loc_ReadReg(Address);

    loc_WriteReg(Address, val & ~(1 << Pos));
}

static void loc_ModifyBit(uintptr_t Address, int Bit, uint32_t Value)
{
    uint32_t tmp = loc_ReadReg(Address);

    if (Value) {
        tmp |= (1 << Bit);
    } else {
        tmp &= ~(1 << Bit);
    }

    loc_WriteReg(Address, tmp);
}

static void loc_PfcWrite(uintptr_t Address, uint32_t Value)
{
    //taskENTER_CRITICAL();
    loc_WriteReg(PFC_PMMR, ~Value);
    loc_WriteReg(Address, Value);
    //taskEXIT_CRITICAL();
}

static void loc_PfcSetGPSR(uint8_t gpio, uint8_t Block, uint8_t Pos)
{
    uint32_t val = loc_ReadReg(PFC_GPSR(Block));

    if (gpio) {
        val = val & ~(1 << Pos);
    } else {
        val = val | (1 << Pos);
    }
    loc_PfcWrite(PFC_GPSR(Block), val);
    //printf("%s: gpio%d,%d (PFC_GPSR reg 0x%x = 0x%x)\n", __func__, Block, Pos, PFC_GPSR(Block), val);
}

static void loc_GpioSetPin(uint32_t Block, uint32_t gpio, uint32_t Output)
{
    /* Configure positive logic in POSNEG */
    loc_ModifyBit(GPIO_POSNEG + GPIO_BASE[Block], gpio, 0);
    /* Select "General Input/Output Mode" in IOINTSEL */
    loc_ModifyBit(GPIO_IOINTSEL + GPIO_BASE[Block], gpio, 0);
    /* Select Input Mode or Output Mode in INOUTSEL */
    loc_ModifyBit(GPIO_INOUTSEL + GPIO_BASE[Block], gpio, Output);
    /* Select General Output Register to Output data in OUTDTSEL */
    loc_ModifyBit(GPIO_OUTDTSEL + GPIO_BASE[Block], gpio, 0);
}

void R_GPIO_SetOutput(uint8_t Block, uint8_t Bit, uint8_t Level)
{
    //taskENTER_CRITICAL();
    if (Level) {
        loc_SetBit(GPIO_OUTDT + GPIO_BASE[Block], Bit);
    } else {
        loc_ClearBit(GPIO_OUTDT + GPIO_BASE[Block], Bit);
    }
    //taskEXIT_CRITICAL();
}

uint8_t R_GPIO_GetInput(uint8_t Block, uint8_t Bit)
{
    return (loc_ReadReg(GPIO_INDT + GPIO_BASE[Block]) >> Bit) & 1;
}

void R_GPIO_SetGpio(uint8_t Block, uint8_t Pos, uint8_t Output)
{
    /* Enable module clock */
    pfcSetGPIO(9, 12 - Block, 1);

    //taskENTER_CRITICAL();
    if (Output) {
        loc_SetBit(GPIO_INOUTSEL + GPIO_BASE[Block], Pos);
        //printf("%s: gpio%d,%d Output (GPIO_INOUTSEL reg 0x%x = 0x%x)\n", __func__, Block, Pos, GPIO_INOUTSEL + GPIO_BASE[Block], loc_ReadReg(GPIO_INOUTSEL + GPIO_BASE[Block]));
    } else {
        loc_ClearBit(GPIO_INOUTSEL + GPIO_BASE[Block], Pos);
        //printf("%s: gpio%d,%d input  (GPIO_INOUTSEL reg 0x%x = 0x%x)\n", __func__, Block, Pos, GPIO_INOUTSEL + GPIO_BASE[Block], loc_ReadReg(GPIO_INOUTSEL + GPIO_BASE[Block]));
    }

    loc_PfcSetGPSR(1, Block, Pos);
    loc_GpioSetPin(Block, Pos, Output);
    //taskEXIT_CRITICAL();
}
