/*************************************************************************************************************
* sysc_api_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/*******************************************************************************
   Section: Includes
 */
#include <stdint.h>
#include "r_sysc_regs.h"
#include "r_sysc_api.h"
#include "rcar-xos/osal/r_osal.h"
#include "r_print_api.h"

/*******************************************************************************
   Section: Local types
 */
#define SYSC_MUTEX_RESOURCE_ID 0x8000u /* RESOURCE_ID_BSP */
#define SYSC_MUTEX_ID (SYSC_MUTEX_RESOURCE_ID + 100U) /*OSAL_MUTEX_DRIVER_SYSC*/
#define SYSC_MUTEX_TIMEOUT_MS 3000 /* 3000 milisecond */

#define R_SYSC_REG_NONE (0x0000u) /* No corresponding register */

#define R_SYSC_PWR_RETRY (1000u) /* Power on retry cnt */

static osal_mutex_handle_t SyscMtxHandle; /* mutex handle of sysc driver */

/* Number of modules using each power supply */
static uint32_t EnableCount[R_SYSC_LAST] = {0u};

typedef struct {
    uint16_t reg;
    uint8_t bit;
} sysc_power_info_t;

static const sysc_power_info_t loc_PowerOnRegBits[] = {
    /* [Power Name] =    {Reg, Bit}, */
    [R_SYSC_CA57_CPU0] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU0) */
    [R_SYSC_CA57_CPU1] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU1) */
    [R_SYSC_CA57_CPU2] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU2) */
    [R_SYSC_CA57_CPU3] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU3) */
    [R_SYSC_CA57_SCU] =  {0x01CC, 0}, /* PWRONCR5, PWRUP  (Cortex-A57 SCU)  */
    [R_SYSC_CA53_CPU0] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU0) */
    [R_SYSC_CA53_CPU1] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU1) */
    [R_SYSC_CA53_CPU2] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU2) */
    [R_SYSC_CA53_CPU3] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU3) */
    [R_SYSC_CA53_SCU] =  {0x014C, 0}, /* PWRONCR3, PWRUP  (Cortex-A53 SCU)  */
    [R_SYSC_CR7] =       {0x024C, 0}, /* PWRONCR7, PWRUP  (Cortex-R7)       */
    [R_SYSC_3DG_A] =     {0x010C, 0}, /* PWRONCR2, PWRUP-A(3DG-A)           */
    [R_SYSC_3DG_B] =     {0x010C, 1}, /* PWRONCR2, PWRUP-B(3DG-B)           */
    [R_SYSC_3DG_C] =     {0x010C, 2}, /* PWRONCR2, PWRUP-C(3DG-C)           */
    [R_SYSC_3DG_D] =     {0x010C, 3}, /* PWRONCR2, PWRUP-D(3DG-D)           */
    [R_SYSC_3DG_E] =     {0x010C, 4}, /* PWRONCR2, PWRUP-E(3DG-E)           */
    [R_SYSC_IMP_A3IR] =  {0x018C, 0}, /* PWRONCR4, PWRUP  (IMP/A3IR)        */
    [R_SYSC_A3VP] =      {0x034C, 0}, /* PWRONCR8, PWRUP  (A3VP)            */
    [R_SYSC_A3VC] =      {0x038C, 0}, /* PWRONCR9, PWRUP  (A3VC)            */
    [R_SYSC_A2VC1] =     {0x03CC, 1}, /* PWRONCR10,PWRUP1 (A2VC1)           */
};

static const sysc_power_info_t loc_PowerOffRegBits[] = {
    /* [Power Name] =    {Reg, Bit}, */
    [R_SYSC_CA57_CPU0] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU0) */
    [R_SYSC_CA57_CPU1] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU1) */
    [R_SYSC_CA57_CPU2] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU2) */
    [R_SYSC_CA57_CPU3] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A57 CPU3) */
    [R_SYSC_CA57_SCU] =  {0x01C4, 0}, /* PWROFFCR5,PWRDWN (Cortex-A57 SCU)  */
    [R_SYSC_CA53_CPU0] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU0) */
    [R_SYSC_CA53_CPU1] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU1) */
    [R_SYSC_CA53_CPU2] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU2) */
    [R_SYSC_CA53_CPU3] = {R_SYSC_REG_NONE, 0}, /* None,None (Cortex-A53 CPU3) */
    [R_SYSC_CA53_SCU] =  {0x0144, 0}, /* PWROFFCR3, PWRDWN (Cortex-A53 SCU) */
    [R_SYSC_CR7] =       {0x0244, 0}, /* PWROFFCR7, PWRDWN (Cortex-R7)      */
    [R_SYSC_3DG_A] =     {0x0104, 0}, /* PWROFFCR2, PWRDWN-A (3DG-A)        */
    [R_SYSC_3DG_B] =     {0x0104, 1}, /* PWROFFCR2, PWRDWN-B (3DG-B)        */
    [R_SYSC_3DG_C] =     {0x0104, 2}, /* PWROFFCR2, PWRDWN-C (3DG-C)        */
    [R_SYSC_3DG_D] =     {0x0104, 3}, /* PWROFFCR2, PWRDWN-D (3DG-D)        */
    [R_SYSC_3DG_E] =     {0x0104, 4}, /* PWROFFCR2, PWRDWN-E (3DG-E)        */
    [R_SYSC_IMP_A3IR] =  {0x0184, 0}, /* PWROFFCR4, PWRDWN   (IMP/A3IR)     */
    [R_SYSC_A3VP] =      {0x0344, 0}, /* PWROFFCR8, PWRDWN   (A3VP)         */
    [R_SYSC_A3VC] =      {0x0384, 0}, /* PWROFFCR9, PWRDWN   (A3VC)         */
    [R_SYSC_A2VC1] =     {0x03C4, 1}, /* PWROFFCR10,PWRUP1   (A2VC1)        */
};

static const sysc_power_info_t loc_PowerOnStatRegBits[] = {
    /* [Power Name] =    {Reg, Bit}, */
    [R_SYSC_CA57_CPU0] = {0x0080, 4}, /* PWRSR0,PWRUP_CPU[0](Cortex-A57 CPU0) */
    [R_SYSC_CA57_CPU1] = {0x0080, 5}, /* PWRSR0,PWRUP_CPU[1](Cortex-A57 CPU1) */
    [R_SYSC_CA57_CPU2] = {0x0080, 6}, /* PWRSR0,PWRUP_CPU[2](Cortex-A57 CPU2) */
    [R_SYSC_CA57_CPU3] = {0x0080, 7}, /* PWRSR0,PWRUP_CPU[3](Cortex-A57 CPU3) */
    [R_SYSC_CA57_SCU] =  {0x01C0, 4}, /* PWRSR5, PWRUP      (Cortex-A57 SCU)  */
    [R_SYSC_CA53_CPU0] = {0x0200, 4}, /* PWRSR6,PWRUP_CPU[0](Cortex-A53 CPU0) */
    [R_SYSC_CA53_CPU1] = {0x0200, 5}, /* PWRSR6,PWRUP_CPU[1](Cortex-A53 CPU1) */
    [R_SYSC_CA53_CPU2] = {0x0200, 6}, /* PWRSR6,PWRUP_CPU[2](Cortex-A53 CPU2) */
    [R_SYSC_CA53_CPU3] = {0x0200, 7}, /* PWRSR6,PWRUP_CPU[3](Cortex-A53 CPU3) */
    [R_SYSC_CA53_SCU] =  {0x0140, 4}, /* PWRSR3, PWRUP     (Cortex-A53 SCU)   */
    [R_SYSC_CR7] =       {0x0240, 4}, /* PWRSR7, PWRUP     (Cortex-R7)        */
    [R_SYSC_3DG_A] =     {0x0100, 5}, /* PWRSR2, PWRUP-A   (3DG-A)            */
    [R_SYSC_3DG_B] =     {0x0100, 6}, /* PWRSR2, PWRUP-B   (3DG-B)            */
    [R_SYSC_3DG_C] =     {0x0100, 7}, /* PWRSR2, PWRUP-C   (3DG-C)            */
    [R_SYSC_3DG_D] =     {0x0100, 8}, /* PWRSR2, PWRUP-D   (3DG-D)            */
    [R_SYSC_3DG_E] =     {0x0100, 9}, /* PWRSR2, PWRUP-E   (3DG-E)            */
    [R_SYSC_IMP_A3IR] =  {0x0180, 4}, /* PWRSR4, PWRUP     (IMP/A3IR)         */
    [R_SYSC_A3VP] =      {0x0340, 4}, /* PWRSR8, PWRUP     (A3VP)             */
    [R_SYSC_A3VC] =      {0x0380, 4}, /* PWRSR9, PWRUP     (A3VC)             */
    [R_SYSC_A2VC1] =     {0x03C0, 3}, /* PWRSR10,PWRUP1    (A2VC1)            */
};

static const sysc_power_info_t loc_PowerOffStatRegBits[] = {
    /* [Power Name] =    {Reg, Bit}, */
    [R_SYSC_CA57_CPU0] = {0x0080, 0}, /* PWRSR0,PWRDWN_CPU[0](Cortex-A57 CPU0)*/
    [R_SYSC_CA57_CPU1] = {0x0080, 1}, /* PWRSR0,PWRDWN_CPU[1](Cortex-A57 CPU1)*/
    [R_SYSC_CA57_CPU2] = {0x0080, 2}, /* PWRSR0,PWRDWN_CPU[2](Cortex-A57 CPU2)*/
    [R_SYSC_CA57_CPU3] = {0x0080, 3}, /* PWRSR0,PWRDWN_CPU[3](Cortex-A57 CPU3)*/
    [R_SYSC_CA57_SCU] =  {0x01C0, 0}, /* PWRSR5, PWRDWN      (Cortex-A57 SCU) */
    [R_SYSC_CA53_CPU0] = {0x0200, 0}, /* PWRSR6,PWRDWN_CPU[0](Cortex-A53 CPU0)*/
    [R_SYSC_CA53_CPU1] = {0x0200, 1}, /* PWRSR6,PWRDWN_CPU[1](Cortex-A53 CPU1)*/
    [R_SYSC_CA53_CPU2] = {0x0200, 2}, /* PWRSR6,PWRDWN_CPU[2](Cortex-A53 CPU2)*/
    [R_SYSC_CA53_CPU3] = {0x0200, 3}, /* PWRSR6,PWRDWN_CPU[3](Cortex-A53 CPU3)*/
    [R_SYSC_CA53_SCU] =  {0x0140, 0}, /* PWRSR3, PWRDWN      (Cortex-A53 SCU) */
    [R_SYSC_CR7] =       {0x0240, 0}, /* PWRSR7, PWRDWN      (Cortex-R7)      */
    [R_SYSC_3DG_A] =     {0x0100, 0}, /* PWRSR2, PWRDWN-A    (3DG-A)          */
    [R_SYSC_3DG_B] =     {0x0100, 1}, /* PWRSR2, PWRDWN-B    (3DG-B)          */
    [R_SYSC_3DG_C] =     {0x0100, 2}, /* PWRSR2, PWRDWN-C    (3DG-C)          */
    [R_SYSC_3DG_D] =     {0x0100, 3}, /* PWRSR2, PWRDWN-D    (3DG-D)          */
    [R_SYSC_3DG_E] =     {0x0100, 4}, /* PWRSR2, PWRDWN-E    (3DG-E)          */
    [R_SYSC_IMP_A3IR] =  {0x0180, 0}, /* PWRSR4, PWRDWN      (IMP/A3IR)       */
    [R_SYSC_A3VP] =      {0x0340, 0}, /* PWRSR8, PWRDWN      (A3VP)           */
    [R_SYSC_A3VC] =      {0x0380, 0}, /* PWRSR9, PWRDWN      (A3VC)           */
    [R_SYSC_A2VC1] =     {0x03C0, 1}, /* PWRSR10,PWRDWN1     (A2VC1)          */
};

/*******************************************************************************
   Section: Local Functions
 */

static void loc_WriteReg(uintptr_t Address, uint32_t Value)
{
    *((volatile uint32_t *)Address) = Value;
}

static uint32_t loc_ReadReg(uintptr_t Address)
{
    return *((volatile uint32_t *)Address);
}

/*******************************************************************************
   Section: Global Functions
 */

void R_PMA_SYSC_Init(void)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /* mutex handle create */
    osal_ret = R_OSAL_ThsyncMutexCreate((osal_mutex_id_t)SYSC_MUTEX_ID,
                                        &SyscMtxHandle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[R_PMA_SYSC_Init]MutexCreate Error. osal_ret:%d.\n", osal_ret);
    }
}


void R_PMA_SYSC_Quit(void)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /* mutex handle destory */
    osal_ret = R_OSAL_ThsyncMutexDestroy(SyscMtxHandle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[R_PMA_SYSC_Quit]MutexDestroy Error. osal_ret:%d.\n", osal_ret);
    }
}

r_sysc_Error_t R_PMA_SYSC_SetPowerStat(r_sysc_PowerName_t PowerName,
                                       bool power_on)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_sysc_Error_t ret = R_SYSC_ERR_SUCCESS;
    const sysc_power_info_t *regs;
    uint32_t     val = 0u;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(SyscMtxHandle,
                                      (osal_milli_sec_t)SYSC_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_SYSC_ERR_FAILED;
        R_PRINT_Log("[R_PMA_SYSC_SetPowerStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    if (R_SYSC_ERR_SUCCESS == ret)
    {
        /* Argument check */
        if (PowerName >= R_SYSC_LAST)
        {
            ret = R_SYSC_ERR_INVALID_PARAMETER;
            R_PRINT_Log("[R_PMA_SYSC_SetPowerStat]: 1st argument error(over).\n");
        }
    }

    if (R_SYSC_ERR_SUCCESS == ret)
    {
        if (true == power_on)
        {
            /* Gets the registers and bits for the power on. */
            regs = &loc_PowerOnRegBits[PowerName];
        }
        else
        {
            /* Gets the registers and bits for the power off. */
            regs = &loc_PowerOffRegBits[PowerName];
        }

        /* When there is no corresponding register */
        if (R_SYSC_REG_NONE == regs->reg)
        {
            ret = R_SYSC_ERR_INVALID_PARAMETER;
            R_PRINT_Log("[R_PMA_SYSC_SetPowerStat]: 1st arg error(PowerName:%d).\n",
                    PowerName);
        }
    }

    if (R_SYSC_ERR_SUCCESS == ret)
    {
        /* power on: first power on */
        /* power off: last power off */
        /* (Do not power off when more than one module is in use) */
        if (((true == power_on) && (0u == EnableCount[PowerName])) ||
            ((false == power_on) && (1u == EnableCount[PowerName])))
        {
            /* Set the target bit of register to 1. */

            val = loc_ReadReg(R_SYSC_BASE + regs->reg);
            val = val | (1 << regs->bit);
            loc_WriteReg(R_SYSC_BASE + regs->reg, val);
        }

        if (true == power_on)
        {
            /* EnableCount Increment */
            EnableCount[PowerName]++;
        }
        else
        {
            if (0 < EnableCount[PowerName])
            {
                /* EnableCount Decrement */
                EnableCount[PowerName]--;
            }
            else
            {
                ret = R_SYSC_ERR_FAILED;
                R_PRINT_Log("[R_PMA_SYSC_SetPowerStat]Count mismatch.PowerName:%d\n",
                        PowerName);
            }
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(SyscMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_SYSC_ERR_FAILED;
            R_PRINT_Log("[R_PMA_SYSC_SetPowerStat]mutex unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}

r_sysc_Error_t R_PMA_SYSC_GetPowerStat(r_sysc_PowerName_t PowerName,
                                       bool power_on,
                                       uint32_t* PowerStat)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_sysc_Error_t ret = R_SYSC_ERR_SUCCESS;
    const sysc_power_info_t *regs;
    uint32_t     val = 0u;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(SyscMtxHandle,
                                      (osal_milli_sec_t)SYSC_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_SYSC_ERR_FAILED;
        R_PRINT_Log("[R_PMA_SYSC_GetPowerStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    /* Argument check */
    if (PowerName >= R_SYSC_LAST)
    {
        ret = R_SYSC_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_SYSC_GetPowerStat]: 1st argument error(over).\n");
    }

    /* Argument check */
    if (NULL == PowerStat)
    {
        ret = R_SYSC_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_SYSC_GetPowerStat]: 3rd argument error(NULL).\n");
    }

    if (R_SYSC_ERR_SUCCESS == ret)
    {
        if (true == power_on)
        {
           /* Gets the registers and bits for getting power on status. */
            regs = &loc_PowerOnStatRegBits[PowerName];
        }
        else
        {
            /* Gets the registers and bits for getting power off status. */
            regs = &loc_PowerOffStatRegBits[PowerName];
        }
    }

    if (R_SYSC_ERR_SUCCESS == ret)
    {
        /* Get the target register value. */
        val = loc_ReadReg(R_SYSC_BASE + regs->reg);

        /* Get the target bit value. */
        if (0 == (val & (1 << regs->bit)))
        {
            *PowerStat = 0;
        }
        else
        {
            *PowerStat = 1;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(SyscMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_SYSC_ERR_FAILED;
            R_PRINT_Log("[R_PMA_SYSC_GetPowerStat]mutex unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}
