/*************************************************************************************************************
* cpg_api_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include "r_cpg_regs.h"
#include "r_cpg_api.h"
#include "r_delay_api.h"
#include "rcar-xos/osal/r_osal.h"
#include "r_print_api.h"

#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))

typedef struct {
    uint8_t reg;
    uint8_t bit;
} cpg_module_info_t;

#define CPG_MUTEX_RESOURCE_ID 0x8000u /* RESOURCE_ID_BSP */
#define CPG_MUTEX_ID (CPG_MUTEX_RESOURCE_ID + 99U) /* OSAL_MUTEX_DRIVER_CPG */
#define CPG_MUTEX_TIMEOUT_MS 3000 /* 3000 milisecond */

static osal_mutex_handle_t CpgMtxHandle; /* mutex handle of sysc driver */

typedef struct {
    uint16_t reg;
    uint8_t bit;
} cpg_clock_info_t;

static const cpg_clock_info_t loc_ClockRegBits[] = {
 /* [Clock Name] = {Reg, Bit}, */
    [R_CPG_ZB3_CLOCK] =    { 0x0380, 11}, /* ZB3CKCR,   CKSTP   (ZB3 Clock) */
    [R_CPG_SD0H_CLOCK] =   { 0x0074, 9 }, /* SD0CKCR,   STPnHCK (SD0H Clock) */
    [R_CPG_SD0_CLOCK] =    { 0x0074, 8 }, /* SD0CKCR,   STPnCK  (SD0 Clock) */
    [R_CPG_SD1H_CLOCK] =   { 0x0078, 9 }, /* SD1CKCR,   STPnHCK (SD1H Clock) */
    [R_CPG_SD1_CLOCK] =    { 0x0078, 8 }, /* SD1CKCR,   STPnCK  (SD1 Clock) */
    [R_CPG_SD2H_CLOCK] =   { 0x0268, 9 }, /* SD2CKCR,   STPnHCK (SD2H Clock) */
    [R_CPG_SD2_CLOCK] =    { 0x0268, 8 }, /* SD2CKCR,   STPnCK  (SD2 Clock) */
    [R_CPG_SD3H_CLOCK] =   { 0x026C, 9 }, /* SD3CKCR,   STPnHCK (SD3H Clock) */
    [R_CPG_SD3_CLOCK] =    { 0x026C, 8 }, /* SD3CKCR,   STPnCK  (SD3 Clock) */
    [R_CPG_SSPSRC_CLOCK] = { 0x0248, 8 }, /* SSPSRCCKCR,CKSTP   (SSPSRC Clock)*/
    [R_CPG_SSPRS_CLOCK] =  { 0x024C, 8 }, /* SSPRSCKCR, CKSTP   (SSPRS Clock) */
    [R_CPG_RPC_CLOCK] =    { 0x0238, 8 }, /* RPCCKCR,   CKSTP   (RPC Clock) */
    [R_CPG_RPCD2_CLOCK] =  { 0x0238, 9 }, /* RPCCKCR,   CKSTP2  (RPCD2 Clock) */
    [R_CPG_MSO_CLOCK] =    { 0x0014, 8 }, /* MSOCKCR,   CKSTP   (MSO Clock) */
    [R_CPG_CANFD_CLOCK] =  { 0x0244, 8 }, /* CANFDCKCR, CKSTP   (CANFD Clock) */
    [R_CPG_HDMI_CLOCK] =   { 0x0250, 8 }, /* HDMICKCR,  CKSTP   (HDMI Clock) */
    [R_CPG_CSI0_CLOCK] =   { 0x000C, 8 }, /* CSI0CKCR,  CKSTP   (CSI0 Clock) */
    [R_CPG_DCU_CLOCK] =    { 0x012C, 12}, /* DCUCKCR,   CKSTP   (DCU Clock) */
    [R_CPG_ADG_CLOCK] =    { 0x0984, 22}, /* ADGCKCR,   CKSTP   (Audio ADG Clock) */
    [R_CPG_SSI0_CLOCK] =    { 0x0988, 15}, /* SSI0CKCR,   CKSTP  (SSI0 Clock) */
    [R_CPG_SSI1_CLOCK] =    { 0x0988, 14}, /* SSI0CKCR,   CKSTP  (SSI1 Clock) */
    [R_CPG_SSI2_CLOCK] =    { 0x0988, 13}, /* SSI0CKCR,   CKSTP  (SSI2 Clock) */
    [R_CPG_SSI3_CLOCK] =    { 0x0988, 12}, /* SSI0CKCR,   CKSTP  (SSI3 Clock) */
    [R_CPG_SSI4_CLOCK] =    { 0x0988, 11}, /* SSI0CKCR,   CKSTP  (SSI4 Clock) */
    [R_CPG_SSI5_CLOCK] =    { 0x0988, 10}, /* SSI0CKCR,   CKSTP  (SSI5 Clock) */
    [R_CPG_SSI6_CLOCK] =    { 0x0988, 9}, /* SSI0CKCR,   CKSTP  (SSI6 Clock) */
    [R_CPG_SSI7_CLOCK] =    { 0x0988, 8}, /* SSI0CKCR,   CKSTP  (SSI7 Clock) */
    [R_CPG_SSI8_CLOCK] =    { 0x0988, 7}, /* SSI0CKCR,   CKSTP  (SSI8 Clock) */
    [R_CPG_SSI9_CLOCK] =    { 0x0988, 6}, /* SSI0CKCR,   CKSTP  (SSI0 Clock) */
    [R_CPG_SSI_ALL_CLOCK] = { 0x0988, 5}, /* SSI0CKCR,   CKSTP  (SSI ALL Clock) */
    [R_CPG_SDIF0_CLOCK] =   { 0x011C, 14}, /* SDIF0CKCR,   CKSTP  (SDIF0 Clock) */
    [R_CPG_SDIF1_CLOCK] =   { 0x011C, 13}, /* SDIF1CKCR,   CKSTP  (SDIF1 Clock) */
    [R_CPG_SDIF2_CLOCK] =   { 0x011C, 12}, /* SDIF2CKCR,   CKSTP  (SDIF2 Clock) */
    [R_CPG_SDIF3_CLOCK] =   { 0x011C, 11}, /* SDIF3CKCR,   CKSTP  (SDIF3 Clock) */
};

/* Realtime module stop control registers */
static const uint16_t loc_RmstpOffset[] = {
    R_CPG_RMSTPCR0,
    R_CPG_RMSTPCR1,
    R_CPG_RMSTPCR2,
    R_CPG_RMSTPCR3,
    R_CPG_RMSTPCR4,
    R_CPG_RMSTPCR5,
    R_CPG_RMSTPCR6,
    R_CPG_RMSTPCR7,
    R_CPG_RMSTPCR8,
    R_CPG_RMSTPCR9,
    R_CPG_RMSTPCR10,
    R_CPG_RMSTPCR11,
    /* Added DHD workaround */
    R_CPG_SAMSTPCR0,
    R_CPG_SAMSTPCR1,
    R_CPG_SAMSTPCR2,
    R_CPG_SAMSTPCR3,
    R_CPG_SAMSTPCR4,
    R_CPG_SAMSTPCR5,
    R_CPG_SAMSTPCR6,
    R_CPG_SAMSTPCR7,
    R_CPG_SAMSTPCR8,
    R_CPG_SAMSTPCR9,
    R_CPG_SAMSTPCR10,
    R_CPG_SAMSTPCR11,
};

/* Software reset registers */
static const uint16_t loc_SrcrOffset[] = {
    R_CPG_SRCR0,
    R_CPG_SRCR1,
    R_CPG_SRCR2,
    R_CPG_SRCR3,
    R_CPG_SRCR4,
    R_CPG_SRCR5,
    R_CPG_SRCR6,
    R_CPG_SRCR7,
    R_CPG_SRCR8,
    R_CPG_SRCR9,
    R_CPG_SRCR10,
    R_CPG_SRCR11,
    R_CPG_SRCR0,    /* Added DHD workaround */
    R_CPG_SRCR1,
    R_CPG_SRCR2,
    R_CPG_SRCR3,
    R_CPG_SRCR4,
    R_CPG_SRCR5,
    R_CPG_SRCR6,
    R_CPG_SRCR7,
    R_CPG_SRCR8,
    R_CPG_SRCR9,
    R_CPG_SRCR10,
    R_CPG_SRCR11,
};

/* Software reset clear registers */
static const uint16_t loc_SrstclrOffset[] = {
    R_CPG_SRSTCLR0,
    R_CPG_SRSTCLR1,
    R_CPG_SRSTCLR2,
    R_CPG_SRSTCLR3,
    R_CPG_SRSTCLR4,
    R_CPG_SRSTCLR5,
    R_CPG_SRSTCLR6,
    R_CPG_SRSTCLR7,
    R_CPG_SRSTCLR8,
    R_CPG_SRSTCLR9,
    R_CPG_SRSTCLR10,
    R_CPG_SRSTCLR11,
    R_CPG_SRSTCLR0,     /* Added DHD workaround */
    R_CPG_SRSTCLR1,
    R_CPG_SRSTCLR2,
    R_CPG_SRSTCLR3,
    R_CPG_SRSTCLR4,
    R_CPG_SRSTCLR5,
    R_CPG_SRSTCLR6,
    R_CPG_SRSTCLR7,
    R_CPG_SRSTCLR8,
    R_CPG_SRSTCLR9,
    R_CPG_SRSTCLR10,
    R_CPG_SRSTCLR11,
};

/* Module stop status registers */
static const uint16_t loc_MstpsrOffset[] = {
    R_CPG_MSTPSR0,
    R_CPG_MSTPSR1,
    R_CPG_MSTPSR2,
    R_CPG_MSTPSR3,
    R_CPG_MSTPSR4,
    R_CPG_MSTPSR5,
    R_CPG_MSTPSR6,
    R_CPG_MSTPSR7,
    R_CPG_MSTPSR8,
    R_CPG_MSTPSR9,
    R_CPG_MSTPSR10,
    R_CPG_MSTPSR11,
    R_CPG_MSTPSR0,      /* Added DHD workaround */
    R_CPG_MSTPSR1,
    R_CPG_MSTPSR2,
    R_CPG_MSTPSR3,
    R_CPG_MSTPSR4,
    R_CPG_MSTPSR5,
    R_CPG_MSTPSR6,
    R_CPG_MSTPSR7,
    R_CPG_MSTPSR8,
    R_CPG_MSTPSR9,
    R_CPG_MSTPSR10,
    R_CPG_MSTPSR11,
};

static void loc_WriteReg(uintptr_t Address, uint32_t Value);
static uint32_t loc_ReadReg(uintptr_t Address);
static void loc_ProtectedRegClear(uintptr_t Addr, uint32_t Mask);
static void loc_ProtectedRegSet(uintptr_t Addr, uint32_t Mask);

static void loc_WriteReg(uintptr_t Address, uint32_t Value)
{
    *((volatile uint32_t *)Address) = Value;
}

static uint32_t loc_ReadReg(uintptr_t Address)
{
    return *((volatile uint32_t *)Address);
}

/*
 * Clear a protected register which address is given. It unlock the "write"
 *          acess by writing into Configure register.
 *
 * @param[in]   Addr - Register address
 * @param[in]   Mask - the current register value shall be masked with this variable
 */
static void loc_ProtectedRegClear(uintptr_t Addr, uint32_t Mask)
{
    uint32_t val;

    loc_WriteReg(R_CPG_BASE + R_CPG_WPCR, 0xA5A50000); /* Clear register protection */

    val = loc_ReadReg(Addr);            /* Generate value */
    val = val & Mask;
    loc_WriteReg(R_CPG_BASE + R_CPG_WPR, ~val);       /* Unlock write */
    loc_WriteReg(Addr, val);                /* Write value */

    if ((loc_ReadReg(Addr) & ~Mask) != 0) {
        R_PRINT_Log("CPG did not clear MSTP bits! (Addr 0x%x, Mask 0x%lx, actual 0x%lx)\n", Addr, Mask, loc_ReadReg(Addr));
    }
}

/*
 * Set protected register which address is given. It unlock the "write"
 *          acess by writing into Configure register.
 *
 * @param[in]   Addr - Register address
 * @param[in]   Mask - the current register value shall be masked with this variable
 */
static void loc_ProtectedRegSet(uintptr_t Addr, uint32_t Mask)
{
    uint32_t val;

    loc_WriteReg(R_CPG_BASE + R_CPG_WPCR, 0xA5A50000); /* Clear register protection */
    val = loc_ReadReg(Addr);
    val = val | Mask;                             /* Generate value */
    loc_WriteReg(R_CPG_BASE + R_CPG_WPR, ~val);           /* Unlock write */
    loc_WriteReg(Addr, val);                  /* Write value */
}

/*
 * Enable module clock
 *
 * @param[in]   RegNr - Index of RMSTP registers
 * @param[in]   BitNr - bit in RMSTP corresponding to the module
 */
static void loc_ModuleEnable(int RegNr, uint32_t BitNr)
{
    loc_ProtectedRegClear(R_CPG_BASE + loc_RmstpOffset[RegNr], ~(1 << BitNr));
}

/*
 * Disable module clock
 *
 * @param[in]   RegNr - Index of RMSTP registers
 * @param[in]   BitNr - bit in RMSTP corresponding to the module
 */
static void loc_ModuleDisable(int RegNr, uint32_t BitNr)
{
    loc_ProtectedRegSet(R_CPG_BASE + loc_RmstpOffset[RegNr], (1 << BitNr));
}

void R_PMA_CPG_Init(void)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /* mutex handle create */
    osal_ret = R_OSAL_ThsyncMutexCreate((osal_mutex_id_t)CPG_MUTEX_ID,
                                        &CpgMtxHandle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[R_PMA_CPG_Init]MutexCreate Error. osal_ret:%d.\n", osal_ret);
    }
}


void R_PMA_CPG_Quit(void)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /* mutex handle destory */
    osal_ret = R_OSAL_ThsyncMutexDestroy(CpgMtxHandle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[R_PMA_CPG_Quit]MutexDestroy Error. osal_ret:%d.\n", osal_ret);
    }
}


r_cpg_Error_t R_PMA_CPG_SetModuleStopStat(r_cpg_RegIdx_t RegIdx,
                                           r_cpg_BitPos_t BitPos,
                                           bool enable)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_cpg_Error_t ret = R_CPG_ERR_SUCCESS;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(CpgMtxHandle,
                                      (osal_milli_sec_t)CPG_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_CPG_ERR_FAILED;
        R_PRINT_Log("[R_PMA_CPG_SetModuleStopStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* Argument check */
        if (RegIdx > R_CPG_REGIDX_20) /* workaround for DHD */
        {
            ret = R_CPG_ERR_INVALID_PARAMETER;
            R_PRINT_Log("[R_PMA_CPG_SetModuleStopStat]: 1st argument error.\n");
        }
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* Argument check */
        if (BitPos > R_CPG_BIT31)
        {
            ret = R_CPG_ERR_INVALID_PARAMETER;
            R_PRINT_Log("[R_PMA_CPG_SetModuleStopStat]: 2nd argument error.\n");
        }
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* module stop: enable */
        if (true == enable)
        {
            /* Disable module clock */
            loc_ModuleDisable((int)RegIdx, BitPos);
        }
        /* module stop: diable */
        else
        {
            /* Enable module clock */
            loc_ModuleEnable((int)RegIdx, BitPos);
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(CpgMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_CPG_ERR_FAILED;
            R_PRINT_Log("[R_PMA_CPG_SetModuleStopStat]unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}


r_cpg_Error_t R_PMA_CPG_GetModuleStopStat(r_cpg_RegIdx_t RegIdx,
                                           r_cpg_BitPos_t BitPos,
                                           bool* enable)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_cpg_Error_t ret = R_CPG_ERR_SUCCESS;
    uint32_t ModuleStopStat = 0;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(CpgMtxHandle,
                                      (osal_milli_sec_t)CPG_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_CPG_ERR_FAILED;
        R_PRINT_Log("[R_PMA_CPG_GetModuleStopStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    /* Argument check */
    if (RegIdx > R_CPG_REGIDX_20) /* workaround for DHD */
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;

        R_PRINT_Log("[R_PMA_CPG_GetModuleStopStat]: 1st argument error.\n");
    }

    /* Argument check */
    if (BitPos > R_CPG_BIT31)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;

        R_PRINT_Log("[R_PMA_CPG_GetModuleStopStat]: 2nd argument error.\n");
    }

    /* Argument check */
    if (NULL == enable)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;

        R_PRINT_Log("[R_PMA_CPG_GetModuleStopStat]: 3rd argument error(NULL).\n");
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* Get value of MSTPSRn(n:RegIdx) BitPos */
        ModuleStopStat = loc_ReadReg(R_CPG_BASE + loc_MstpsrOffset[RegIdx])
                         & (1 << BitPos);

       /* 0:Supply of the clock signal to the corresponding module is enabled */
       if (0 == ModuleStopStat)
       {
           *enable = false;
       }
       /* 1:Supply of the clock signal to the corresponding module is stopped */
       else
       {
           *enable = true;
       }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(CpgMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_CPG_ERR_FAILED;
            R_PRINT_Log("[R_PMA_CPG_GetModuleStopStat]unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}

r_cpg_Error_t R_PMA_CPG_SetResetStat(r_cpg_RegIdx_t RegIdx,
                                     r_cpg_BitPos_t BitPos,
                                     uint32_t ResetStat)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_cpg_Error_t ret = R_CPG_ERR_SUCCESS;
    uintptr_t SrcrAddr = 0u;
    uintptr_t SrstclrAddr = 0u;
    uint32_t val = 0u;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(CpgMtxHandle,
                                      (osal_milli_sec_t)CPG_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_CPG_ERR_FAILED;
        R_PRINT_Log("[R_PMA_CPG_SetResetStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    /* Argument check */
    if (RegIdx > R_CPG_REGIDX_20)  /* workaround for DHD */
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_CPG_SetResetStat]: 1st argument error.\n");
    }
    else
    {
        SrcrAddr = R_CPG_BASE + loc_SrcrOffset[RegIdx];
        SrstclrAddr = R_CPG_BASE + loc_SrstclrOffset[RegIdx];
    }

    /* Argument check */
    if (BitPos > R_CPG_BIT31)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_CPG_SetResetStat]: 2nd argument error.\n");
    }
    else
    {
        val = 1 << BitPos;
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        if (1 == ResetStat)
        {
            /* Set Software Reset */
            loc_WriteReg(SrcrAddr, val);
        }
        else
        {
            /* Clear Software Reset */
            loc_WriteReg(SrstclrAddr, val);
            /* Wait for the bit in SRCRn to become 0 */
            while (loc_ReadReg(SrcrAddr) & val) {
            }
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(CpgMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_CPG_ERR_FAILED;
            R_PRINT_Log("[R_PMA_CPG_SetResetStat]mutex unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}

r_cpg_Error_t R_PMA_CPG_GetResetStat(r_cpg_RegIdx_t RegIdx,
                                      r_cpg_BitPos_t BitPos,
                                      uint32_t* ResetStat)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_cpg_Error_t ret = R_CPG_ERR_SUCCESS;
    uint32_t     val = 0u;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(CpgMtxHandle,
                                      (osal_milli_sec_t)CPG_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_CPG_ERR_FAILED;
        R_PRINT_Log("[R_PMA_CPG_GetResetStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    /* Argument check */
    if (RegIdx > R_CPG_REGIDX_20) /* workaround for DHD */
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_CPG_GetResetStat]: 1st argument error.\n");
    }

    /* Argument check */
    if (BitPos > R_CPG_BIT31)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_CPG_GetResetStat]: 2nd argument error.\n");
    }

    /* Argument check */
    if (NULL == ResetStat)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;

        R_PRINT_Log("[R_PMA_CPG_GetResetStat]: 3rd argument error(NULL).\n");
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* Get the target register value. */
        val = loc_ReadReg(R_CPG_BASE + loc_SrcrOffset[RegIdx]);

        /* Get value of SRCRn(n:RegIdx) BitPos */
        if (0 == (val & (1 << BitPos)))
        {
            *ResetStat = 0;
        }
        else
        {
            *ResetStat = 1;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(CpgMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_CPG_ERR_FAILED;
            R_PRINT_Log("[R_PMA_CPG_GetResetStat]mutex unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}

r_cpg_Error_t R_PMA_CPG_SetClockStopStat(r_cpg_ClockName_t ClockName,
                                          bool enable)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_cpg_Error_t ret = R_CPG_ERR_SUCCESS;
    const cpg_clock_info_t *regs;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(CpgMtxHandle,
                                      (osal_milli_sec_t)CPG_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_CPG_ERR_FAILED;
        R_PRINT_Log("[R_PMA_CPG_SetClockStopStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    /* Argument check */
    if (ClockName >= R_CPG_CLOCK_LAST)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;
        R_PRINT_Log("[R_PMA_CPG_SetClockStopStat]: 1st argument error.\n");
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* clock stop: enable */
        if (true == enable)
        {
            /* 1: Stops clock */
            regs = &loc_ClockRegBits[ClockName];
            loc_ProtectedRegSet(R_CPG_BASE + regs->reg, (1 << regs->bit));
        }
        /* clock stop: disable */
        else
        {
            /* 0: Supplies clock */
            regs = &loc_ClockRegBits[ClockName];
            loc_ProtectedRegClear(R_CPG_BASE + regs->reg,~(1 << regs->bit));
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(CpgMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_CPG_ERR_FAILED;
            R_PRINT_Log("[R_PMA_CPG_SetClockStopStat]unlock error.osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}

r_cpg_Error_t R_PMA_CPG_GetClockStopStat(r_cpg_ClockName_t ClockName,
                                          bool* enable)
{
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    r_cpg_Error_t ret = R_CPG_ERR_SUCCESS;
    const cpg_clock_info_t *regs;
    uint32_t ClockStat = 0u;

    /* Lock a mutex */
    osal_ret = R_OSAL_ThsyncMutexLockForTimePeriod(CpgMtxHandle,
                                      (osal_milli_sec_t)CPG_MUTEX_TIMEOUT_MS);
    if (OSAL_RETURN_OK != osal_ret)
    {
        ret = R_CPG_ERR_FAILED;
        R_PRINT_Log("[R_PMA_CPG_GetClockStopStat]mutex lock error. osal_ret:%d.\n",
                osal_ret);
    }

    /* Argument check */
    if (ClockName >= R_CPG_CLOCK_LAST)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;

        R_PRINT_Log("[R_PMA_CPG_GetClockStopStat]: 1st argument error.\n");
    }

    /* Argument check */
    if (NULL == enable)
    {
        ret = R_CPG_ERR_INVALID_PARAMETER;

        R_PRINT_Log("[R_PMA_CPG_GetClockStopStat]: 2nd argument error(NULL).\n");
    }

    if (R_CPG_ERR_SUCCESS == ret)
    {
        /* Get value of reg->reg reg->bit */
        regs = &loc_ClockRegBits[ClockName];
        ClockStat = loc_ReadReg(R_CPG_BASE + regs->reg) & (1 << regs->bit);

        /* 0: Supplies clock */
        if (0 == ClockStat)
        {
            *enable = false;
        }
        /* 1: Stops clock */
        else
        {
            *enable = true;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        /* Unlock a mutex */
        osal_ret = R_OSAL_ThsyncMutexUnlock(CpgMtxHandle);
        if (OSAL_RETURN_OK != osal_ret)
        {
            ret = R_CPG_ERR_FAILED;
            R_PRINT_Log("[R_PMA_CPG_GetClockStopStat]unlock error. osal_ret:%d.\n",
                    osal_ret);
        }
    }

    return ret;
}

