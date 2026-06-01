/*************************************************************************************************************
* cio_bsp_r-carx3_cr7_can
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/


/*******************************************************************************
   Section: Includes
 */

#include "rcar-xos/osal/r_osal.h"
#ifdef R_USE_CAN
    #include "r_cio_bsp_can_config.h"
#else
#ifdef R_USE_CANFD
   #include "r_cio_bsp_canfd_config.h"
#endif
#endif
#include "r_cpg_api.h"
#include "r_delay_api.h"
#include "r_print_api.h"
#include "r_prr_api.h"


/*******************************************************************************
   Section: Local defines
 */

#define PFC_BASE        0xE6060000
#define RCAR_PFC_PMMR       (PFC_BASE + 0x0000) // LSI Multiplexed Pin Setting Mask Register

#define RCAR_PFC_IPSR4      (PFC_BASE + 0x0210) // Peripheral function select register 4
#define RCAR_PFC_IPSR5      (PFC_BASE + 0x0214) // Peripheral function select register 5
#define RCAR_PFC_IPSR6      (PFC_BASE + 0x0218) // Peripheral function select register 6
#define RCAR_PFC_IPSR7      (PFC_BASE + 0x021C) // Peripheral function select register 7
#define RCAR_PFC_IPSR12     (PFC_BASE + 0x0230) // Peripheral function select register 12

#define RCAR_PFC_MOD_SEL0   (PFC_BASE + 0x0500) // Module select register 0
#define RCAR_PFC_MOD_SEL1   (PFC_BASE + 0x0504) // Module select register 1
#define RCAR_PFC_MOD_SEL2   (PFC_BASE + 0x0508) // Module select register 2

#define RCAR_PFC_GPSR0      0xE6060100  /* R/W	32	GPIO/Peripheral Function Select register 0 */
#define RCAR_PFC_GPSR1      0xE6060104  /* R/W	32	GPIO/Peripheral Function Select register 1 */
#define RCAR_PFC_GPSR2      0xE6060108  /* R/W	32	GPIO/Peripheral_Function Select register 2 */
#define RCAR_PFC_GPSR3      0xE606010C  /* R/W	32	GPIO/Peripheral Function Select register 3 */
#define RCAR_PFC_GPSR4      0xE6060110  /* R/W	32	GPIO/Peripheral Function Select register 4 */
#define RCAR_PFC_GPSR5      0xE6060114  /* R/W	32	GPIO/Peripheral Function Select register 5 */
#define RCAR_PFC_GPSR6      0xE6060118  /* R/W	32	GPIO/Peripheral Function Select register 6 */
#define RCAR_PFC_GPSR7      0xE606011C  /* R/W	32	GPIO/Peripheral Function Select register 7 */


/*********************** RCAR_GPIO *************************/
#define RCAR_GPIO_INOUTSEL0 0xE6050004  /* R/W	32	General input/output switching register 0 */
#define RCAR_GPIO_INOUTSEL1 0xE6051004  /* R/W	32	General input/output switching register 1 */
#define RCAR_GPIO_INOUTSEL2 0xE6052004  /* R/W	32	General input/output switching register 2 */
#define RCAR_GPIO_INOUTSEL3 0xE6053004  /* R/W	32	General input/output switching register 3 */
#define RCAR_GPIO_INOUTSEL4 0xE6054004  /* R/W	32	General input/output switching register 4 */
#define RCAR_GPIO_INOUTSEL5 0xE6055004  /* R/W	32	General input/output switching register 5 */
#define RCAR_GPIO_INOUTSEL6 0xE6055404  /* R/W	32	General input/output switching register 6 */
#define RCAR_GPIO_INOUTSEL7 0xE6055804  /* R/W	32	General input/output switching register 7 */

#if 0
#define RCAR_GPIO_INDT0     0xE605000C  /* R	32	General input register 0 */
#define RCAR_GPIO_INDT1     0xE605100C  /* R	32	General input register 1 */
#define RCAR_GPIO_INDT2     0xE605200C  /* R	32	General input register 2 */
#define RCAR_GPIO_INDT3     0xE605300C  /* R	32	General input register 3 */
#define RCAR_GPIO_INDT4     0xE605400C  /* R	32	General input register 4 */
#define RCAR_GPIO_INDT5     0xE605500C  /* R	32	General input register 5 */
#define RCAR_GPIO_INDT6     0xE605540C  /* R	32	General input register 6 */
#define RCAR_GPIO_INDT7     0xE605580C  /* R	32	General input register 7 */
#endif

#define RCAR_GPIO_OUTDT0    0xE6050008  /* R/W	32	General output register 0 */
#define RCAR_GPIO_OUTDT1    0xE6051008  /* R/W	32	General output register 1 */
#define RCAR_GPIO_OUTDT2    0xE6052008  /* R/W	32	General output register 2 */
#define RCAR_GPIO_OUTDT3    0xE6053008  /* R/W	32	General output register 3 */
#define RCAR_GPIO_OUTDT4    0xE6054008  /* R/W	32	General output register 4 */
#define RCAR_GPIO_OUTDT5    0xE6055008  /* R/W	32	General output register 5 */
#define RCAR_GPIO_OUTDT6    0xE6055408  /* R/W	32	General output register 6 */
#define RCAR_GPIO_OUTDT7    0xE6055808  /* R/W	32	General output register 7 */

/*********************** RCAR_CPG_Module Standby, Software Reset *************************/
// 7A . Module Standby, Software Reset

#define RCAR_CPG_CPGWPCR                      0xE6150904    // CPG Write Protect Control Register
#define RCAR_CPG_CPGWPR                       0xE6150900    // CPG Write Protect Register

#define RCAR_CPG_CANFDCKCR                    0xE6150244    // CAN-FD clock frequency control register


/* Clock Frequency */
#define OSCCLK_HZ       0x00000083      // 131.57 kHz
#define RCLK_HZ         0x00000020      //  32.89 kHz


/* Register access macros */


#define LOC_REG_WRITE8(ADDR, VAL) *((volatile unsigned char *)((size_t)ADDR)) = (VAL)
#define LOC_REG_READ8(ADDR) *((volatile unsigned char *)((size_t)ADDR))
#define LOC_REG_WRITE16(ADDR, VAL) *((volatile unsigned short *)((size_t)ADDR)) = (VAL)
#define LOC_REG_READ16(ADDR) *((volatile unsigned short *)((size_t)ADDR))
#define LOC_REG_WRITE32(ADDR, VAL) *((volatile unsigned long *)((size_t)ADDR)) = (VAL)
#define LOC_REG_READ32(ADDR) *((volatile unsigned long *)((size_t)ADDR))


/*-----------------------------------------------------------*/
#define BIT(nr)         (1UL << (nr))
#define BIT_MASK(nr)        (BIT(nr) - 1)

osal_device_handle_t osal_devhandle_canfd;

/*******************************************************************************
   Section: Local constants
 */


static const uintptr_t RCAR_PFC_GPSR[8] = {
    (uintptr_t)RCAR_PFC_GPSR0,
    (uintptr_t)RCAR_PFC_GPSR1,
    (uintptr_t)RCAR_PFC_GPSR2,
    (uintptr_t)RCAR_PFC_GPSR3,
    (uintptr_t)RCAR_PFC_GPSR4,
    (uintptr_t)RCAR_PFC_GPSR5,
    (uintptr_t)RCAR_PFC_GPSR6,
    (uintptr_t)RCAR_PFC_GPSR7,
};

static const uintptr_t RCAR_GPIO_INOUTSEL[8] = {
    (uintptr_t)RCAR_GPIO_INOUTSEL0,
    (uintptr_t)RCAR_GPIO_INOUTSEL1,
    (uintptr_t)RCAR_GPIO_INOUTSEL2,
    (uintptr_t)RCAR_GPIO_INOUTSEL3,
    (uintptr_t)RCAR_GPIO_INOUTSEL4,
    (uintptr_t)RCAR_GPIO_INOUTSEL5,
    (uintptr_t)RCAR_GPIO_INOUTSEL6,
    (uintptr_t)RCAR_GPIO_INOUTSEL7,
};

#if 0
static const uintptr_t RCAR_GPIO_INDT[8] = {
    (uintptr_t)RCAR_GPIO_INDT0,
    (uintptr_t)RCAR_GPIO_INDT1,
    (uintptr_t)RCAR_GPIO_INDT2,
    (uintptr_t)RCAR_GPIO_INDT3,
    (uintptr_t)RCAR_GPIO_INDT4,
    (uintptr_t)RCAR_GPIO_INDT5,
    (uintptr_t)RCAR_GPIO_INDT6,
    (uintptr_t)RCAR_GPIO_INDT7,
};
#endif

static const uintptr_t RCAR_GPIO_OUTDT[8] = {
    (uintptr_t)RCAR_GPIO_OUTDT0,
    (uintptr_t)RCAR_GPIO_OUTDT1,
    (uintptr_t)RCAR_GPIO_OUTDT2,
    (uintptr_t)RCAR_GPIO_OUTDT3,
    (uintptr_t)RCAR_GPIO_OUTDT4,
    (uintptr_t)RCAR_GPIO_OUTDT5,
    (uintptr_t)RCAR_GPIO_OUTDT6,
    (uintptr_t)RCAR_GPIO_OUTDT7,
};



/*******************************************************************************
   Section: Local functions
 */

/* Tight loop check: cond should be based on val */
#define readx_loop(op, addr, val, cond, loopcnt)    \
    ({ \
        uint32_t __cnt = loopcnt; \
        while (__cnt--) { \
            (val) = op(addr); \
            if (cond) \
                break; \
        } \
        (cond) ? 1 : 0; \
    })




static inline void setbit_l(uintptr_t addr, uint32_t pos)
{
    uint32_t val = LOC_REG_READ32(addr);

    LOC_REG_WRITE32(addr, val |= BIT(pos));
}

static inline void clearbit_l(uintptr_t addr, uint32_t pos)
{
    uint32_t val = LOC_REG_READ32(addr);

    LOC_REG_WRITE32(addr, val &= ~BIT(pos));
}


static void pfc_reg_write(uintptr_t addr, uint32_t val)
{
    LOC_REG_WRITE32(RCAR_PFC_PMMR, ~val);
    LOC_REG_WRITE32(addr, val);
}

static void pfc_gpio_input(uint32_t gp, uint32_t bit)
{
    uint32_t val;

    clearbit_l(RCAR_GPIO_INOUTSEL[gp], bit);

    /* Set GPSR */
    val = LOC_REG_READ32(RCAR_PFC_GPSR[gp]);
    val &= ~BIT(bit);        /* set GPIO function */
    pfc_reg_write(RCAR_PFC_GPSR[gp], val);
}

static void pfc_gpio_output(uint32_t gp, uint32_t bit)
{
    uint32_t val;

    setbit_l(RCAR_GPIO_INOUTSEL[gp], bit);

    /* Set GPSR */
    val = LOC_REG_READ32(RCAR_PFC_GPSR[gp]);
    val &= ~BIT(bit);        /* set GPIO function */
    pfc_reg_write(RCAR_PFC_GPSR[gp], val);
}

static void gpio_set_output(uint32_t gp, uint32_t bit, uint32_t lvl)
{
    if (lvl)
        setbit_l(RCAR_GPIO_OUTDT[gp], bit);
    else
        clearbit_l(RCAR_GPIO_OUTDT[gp], bit);
}

void prvCanfdDeviceOpen(void)
{
    e_osal_return_t osal_ret;
    size_t numOfByte = 0;
    char devicelist[32];

    osal_ret = R_OSAL_IoGetDeviceList("canfd", &devicelist[0], sizeof(devicelist), &numOfByte);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[prvCanfdDeviceOpen] R_OSAL_IoGetDeviceList failed(%d)\r\n", osal_ret);
    } else {
        osal_ret = R_OSAL_IoDeviceOpen(&devicelist[0], &osal_devhandle_canfd);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[prvCanfdDeviceOpen] R_OSAL_IoDeviceOpen failed(%d)\r\n", osal_ret);
        }
    }
}

void prvCanfdDeviceClose(void)
{
    e_osal_return_t osal_ret;

    /* Close device */
    osal_ret = R_OSAL_IoDeviceClose(osal_devhandle_canfd);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[prvCanfdDeviceClose] R_OSAL_IoDeviceClose failed(%d)\r\n", osal_ret);
    }
    /* After called R_OSAL_IoDeviceClose,  CAN-FD module will be stopped */
}


/* Be aware using Salvator Board
   Can 0 pins may clash with JTAG and DU */
/* Only #define ENABLE_CANFD0 if you really need it */

void prvCanEnableTransceiver(void)
{

    /* Can FD channel 0 */
#ifdef ENABLE_CANFD0
/* TODO add proper board configuration */
#if 1 /* ERGUOTOU BOARD */
    /* For Can/Can-FD 0 */
    pfc_gpio_output(1, 12);         /* _EN */
    pfc_gpio_input(1, 10);          /* _RCAR_CAN_ERR_N */
    pfc_gpio_output(1, 11);         /* _STB_N */
    pfc_gpio_output(1, 5);          /* _WAKE */

    /* Toggle EN & STB_N to bring up the transceiver */
    gpio_set_output(1, 12, 0);
    gpio_set_output(1, 11, 0);

    gpio_set_output(1, 11, 1);
    gpio_set_output(1, 12, 1);
    gpio_set_output(1,  5, 1);
#else
    /* For Can/Can-FD 0 */
    pfc_gpio_output(1, 12);         /* _EN */
    pfc_gpio_output(1, 11);         /* _STB_N */

    /* Toggle EN & STB_N to bring up the transceiver */
    gpio_set_output(1, 12, 0);
    gpio_set_output(1, 11, 0);

    gpio_set_output(1, 11, 1);

    gpio_set_output(1, 12, 1);
#endif
#endif

/* TODO add proper board configuration */
#if 1 /* ERGUOTOU BOARD */
    /* For Can/Can-FD 1 */
    pfc_gpio_output(1, 1);            /* _EN */
    pfc_gpio_input(1, 7);             /* _RCAR_CAN_ERR_N */
    pfc_gpio_output(1, 0);            /* _STB_N */
    pfc_gpio_output(1, 6);            /* _WAKE */

    /* Toggle EN & STB_N to bring up the transceiver */
    gpio_set_output(1, 1, 0);
    gpio_set_output(1, 0, 0);

    gpio_set_output(1, 0, 1);
    gpio_set_output(1, 1, 1);
    gpio_set_output(1, 6, 1);
#else
    /* For Can/Can-FD 1 */
    pfc_gpio_output(1, 9);            /* _EN */
    pfc_gpio_input(1, 26);            /* _RCAR_CAN_ERR_N */
    pfc_gpio_output(1, 8);            /* _STB_N */

    /* Toggle EN & STB_N to bring up the transceiver */
    gpio_set_output(1, 9, 0);
    gpio_set_output(1, 8, 0);

    gpio_set_output(1, 8, 1);
    gpio_set_output(1, 9, 1);
#endif

}

#ifdef R_USE_CANFD

void prvCanPFCInit(void)
{
    uint32_t val;
    r_prr_DevName_t soc = R_PRR_GetDevice();
    soc = R_PRR_GetDevice();

    if (R_PRR_RCARE3 == soc) {
        uint32_t ipsr;
        uint32_t val;
        ipsr = 1;
        /* The below code needs to be adapted if different set of pins are used */
        /* CAN Peripheral settings  */
        val = LOC_REG_READ32(RCAR_PFC_GPSR0);
        val |= BIT(12) | BIT(13);
    #if RCAR_CAN_FCANCLK_EXT
        val |= BIT(14);
    #endif
        pfc_reg_write(RCAR_PFC_GPSR0, val);

        /* CANFD0 Ch1_XX IPSR setting */
        val = LOC_REG_READ32(RCAR_PFC_IPSR7) & 0xFFFFFF0F;
        val |= 0x00000010; // CANFD1_TX
        pfc_reg_write(RCAR_PFC_IPSR7, val);

        val = LOC_REG_READ32(RCAR_PFC_IPSR7) & 0xFFFFF0FF;
        val |= 0x00000100; // CANFD1_TX
        pfc_reg_write(RCAR_PFC_IPSR7, val);

    #if RCAR_CAN_FCANCLK_EXT
        val = LOC_REG_READ32(RCAR_PFC_IPSR7) & 0xFFFF0FFF;
        val |= 0x00001000; // CANFD1_TX
        pfc_reg_write(RCAR_PFC_IPSR7, val);
    #endif
    }
    else if (R_PRR_RCARD3 != soc)
    {
        /* The below code needs to be adapted if different set of pins are used
        * for CANFD0 */

        /* CANFD0_XX_A Peripheral settings  */
        val = LOC_REG_READ32(RCAR_PFC_GPSR1);
        val |= BIT(23) | BIT(24);
    #if RCAR_CAN_FCANCLK_EXT
        val |= BIT(25);
    #endif
        pfc_reg_write(RCAR_PFC_GPSR1, val);

        /* CANFD0_XX_A IPSR setting */
        val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0x00FFFFFF;
        val |= 0x99000000;
        pfc_reg_write(RCAR_PFC_IPSR4, val);

    #if RCAR_CAN_FCANCLK_EXT
        val = LOC_REG_READ32(RCAR_PFC_IPSR5) & 0xFFFFFFF0;
        val |= 0x00000008;
        pfc_reg_write(RCAR_PFC_IPSR5, val);
    #endif
        /* mode selection for CANFD 0
        - make sure to use "default" */
        val = LOC_REG_READ32(RCAR_PFC_MOD_SEL0);
        val &= ~(BIT(5));
        pfc_reg_write(RCAR_PFC_MOD_SEL0,  val);

        /* The below code needs to be adapted if different set of pins are used
        * for CANFD1 */

        /* CANFD1_XX Peripheral settings */
        val = LOC_REG_READ32(RCAR_PFC_GPSR1);
        val |= BIT(22) | BIT(26);
    #if RCAR_CAN_FCANCLK_EXT
        val |= BIT(25);
    #endif
        pfc_reg_write(RCAR_PFC_GPSR1, val);

        /* CANFD0 Ch1_XX IPSR setting */
        val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0xFF0FFFFF;
        val |= 0x00900000; // CANFD1_TX
        pfc_reg_write(RCAR_PFC_IPSR4, val);

        val = LOC_REG_READ32(RCAR_PFC_IPSR5) & 0xFFFFFF0F;
        val |= 0x00000090; // CANFD1_RX
        pfc_reg_write(RCAR_PFC_IPSR5, val);

    #if RCAR_CAN_FCANCLK_EXT
        val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0xFFFFFFF0;
        val |= 0x00000008;
        pfc_reg_write(RCAR_PFC_IPSR5, val);
    #endif
    }
    else /* only D3 case */
    {
        R_PRINT_Log("[prvCanPFCInit] This platform does not currently support CAN for D3\n");
    }
}

void prvCanClkEnable(void)
{
    uint32_t val;
    r_cpg_Error_t cpg_ret;
    e_osal_return_t osal_ret;
    e_osal_pm_required_state_t osal_state;

    /* Enable CAN-FD */
    osal_ret = R_OSAL_PmSetRequiredState(osal_devhandle_canfd, OSAL_PM_REQUIRED_STATE_REQUIRED, false);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[prvCanClkEnable]: R_OSAL_PmSetRequiredState failed(%d), OSAL_PM_REQUIRED_STATE_REQUIRED\r\n", osal_ret);
    }
    else
    {
        /* Wait CAN-FD enabled */
        do
        {
            R_OSAL_ThreadSleepForTimePeriod(1);
            osal_ret = R_OSAL_PmGetRequiredState(osal_devhandle_canfd, &osal_state);
            if (OSAL_RETURN_OK != osal_ret)
            {
                R_PRINT_Log("[prvCanClkEnable]: R_OSAL_PmGetRequiredState failed(%d)\r\n", osal_ret);
            }
        } while (OSAL_PM_REQUIRED_STATE_REQUIRED != osal_state);
    }

    /* Enable CAN-IF0 */
    cpg_ret = R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT16, false);
    /* Enable CAN-IF1 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT15, false);
    /* Enable CAN-FD */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT14, false);
    /* Enable GPIO0 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9 ,R_CPG_BIT12, false);
    /* Enable GPIO1 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT11, false);
    /* Enable GPIO2 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT10, false);
    /* Enable GPIO3 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT9, false);
    /* Enable GPIO4 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT8, false);
    /* Enable GPIO5 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT7, false);
    /* Enable GPIO6 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT6, false);
    /* Enable GPIO7 */
    cpg_ret |= R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT5, false);
    if (R_CPG_ERR_SUCCESS != cpg_ret)
    {
        R_PRINT_Log("[prvCanClkEnable]: prvCanClkEnable CAN-FD R_PMA_CPG_SetModuleStopStat failed(%d)\r\n", cpg_ret);
    }

    /* Stop Clock */
    val = 0xA5A50000;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPCR, val); /* Clear register protection */
    val = 0x00000109;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPR, ~val);
    LOC_REG_WRITE32(RCAR_CPG_CANFDCKCR, val);    /* CANFD CLK to 80MHz */

    /* Start Clock */
    val = 0xA5A50000;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPCR, val); /* Clear register protection */
    val = 0x00000009;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPR, ~val);
    LOC_REG_WRITE32(RCAR_CPG_CANFDCKCR, val);    /* CANFD CLK to 80MHz */
}
void prvCanSoftReset(uint32_t Unit)
{
    e_osal_return_t osal_ret;
    e_osal_pm_reset_state_t osal_reset_state;

    /* CAN-FD module reset */
    osal_ret = R_OSAL_PmSetResetState(osal_devhandle_canfd, OSAL_PM_RESET_STATE_APPLIED);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[prvCanSoftReset]: R_OSAL_PmSetResetState failed(%d), OSAL_PM_RESET_STATE_APPLIED\r\n", osal_ret);
    }
    else
    {
        /* Wait CAN-FD reset start */
        do
        {
            R_OSAL_ThreadSleepForTimePeriod(1);
            osal_ret = R_OSAL_PmGetResetState(osal_devhandle_canfd, &osal_reset_state);
            if (OSAL_RETURN_OK != osal_ret)
            {
                R_PRINT_Log("[prvCanSoftReset]: R_OSAL_PmGetResetState failed(%d), OSAL_PM_RESET_STATE_APPLIED\r\n", osal_ret);
            }
        } while (OSAL_PM_RESET_STATE_APPLIED != osal_reset_state);
    }
    delay_us(R_DELAY_ONE_RCLK_CYCLE_IN_US); /* 1 RCLK cycle wait */
    osal_ret = R_OSAL_PmSetResetState(osal_devhandle_canfd, OSAL_PM_RESET_STATE_RELEASED);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[prvCanSoftReset]: R_OSAL_PmSetResetState failed(%d), OSAL_PM_RESET_STATE_RELEASED\r\n", osal_ret);
    }
    else
    {
        /* Wait CAN-FD reset end */
        do
        {
            R_OSAL_ThreadSleepForTimePeriod(1);
            osal_ret = R_OSAL_PmGetResetState(osal_devhandle_canfd, &osal_reset_state);
            if (OSAL_RETURN_OK != osal_ret)
            {
                R_PRINT_Log("[prvCanSoftReset]: R_OSAL_PmGetResetState failed(%d), OSAL_PM_RESET_STATE_RELEASED\r\n", osal_ret);
            }
        } while (OSAL_PM_RESET_STATE_RELEASED != osal_reset_state);
    }
}

#else /* #ifdef R_USE_CANFD */

void prvCanPFCInit(void)
{
    uint32_t val;

    /* The below code needs to be adapted if different set of pins are used
     * for CAN0 or CAN1 */

    /* CAN0_XX_A Peripheral settings */
    val = LOC_REG_READ32(RCAR_PFC_GPSR1);
    val |= BIT(23) | BIT(24);
#if RCAR_CAN_FCANCLK_EXT
    val |= BIT(25);
#endif
    pfc_reg_write(RCAR_PFC_GPSR1, val);

    /* CAN0_XX_A IPSR setting */
    val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0x00FFFFFF;
    val |= 0x88000000;
    pfc_reg_write(RCAR_PFC_IPSR4, val);

#if RCAR_CAN_FCANCLK_EXT
    val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0xFFFFFFF0;
    val |= 0x00000008;
    pfc_reg_write(RCAR_PFC_IPSR5, val);
#endif

    /* The below code needs to be adapted if different set of pins are used
     * for CAN0 or CAN1 */

    /* CAN1_XX_A Peripheral settings */
    val = LOC_REG_READ32(RCAR_PFC_GPSR1);
    val |= BIT(22) | BIT(26);
#if RCAR_CAN_FCANCLK_EXT
    val |= BIT(25);
#endif
    pfc_reg_write(RCAR_PFC_GPSR1, val);

    /* CAN2_XX IPSR setting */
    val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0xFF0FFFFF;
    val |= 0x00800000; // CAN1_TX
    pfc_reg_write(RCAR_PFC_IPSR4, val);

    val = LOC_REG_READ32(RCAR_PFC_IPSR5) & 0xFFFFFF0F;
    val |= 0x00000080; // CAN1_RX
    pfc_reg_write(RCAR_PFC_IPSR5, val);

#if RCAR_CAN_FCANCLK_EXT
    val = LOC_REG_READ32(RCAR_PFC_IPSR4) & 0xFFFFFFF0;
    val |= 0x00000008;
    pfc_reg_write(RCAR_PFC_IPSR5, val);
#endif
}

void prvCanClkEnable(void)
{
    uint32_t val;
    bool enable = false;
    e_osal_return_t osal_ret;
    e_osal_pm_required_state_t osal_state;
    r_cpg_Error_t cpg_ret;

    /* Enable CAN-FD */
    osal_ret = R_OSAL_PmSetRequiredState(osal_devhandle_canfd, OSAL_PM_REQUIRED_STATE_REQUIRED, false);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[prvCanClkEnable]: R_OSAL_PmSetRequiredState failed(%d), OSAL_PM_REQUIRED_STATE_REQUIRED\r\n", osal_ret);
    }
    else
    {
        /* Wait CAN-FD enabled */
        do
        {
            R_OSAL_ThreadSleepForTimePeriod(1);
            osal_ret = R_OSAL_PmGetRequiredState(osal_devhandle_canfd, &osal_state);
            if (OSAL_RETURN_OK != osal_ret)
            {
                R_PRINT_Log("[prvCanClkEnable]: R_OSAL_PmGetRequiredState failed(%d)\r\n", osal_ret);
            }
        } while (OSAL_PM_REQUIRED_STATE_REQUIRED != osal_state);
    }

    /* Enable CAN-IF0 */
    cpg_ret = R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT16, false);
    /* Enable CAN-IF1 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT15, false);
    /* Enable GPIO0 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT12, false);
    /* Enable GPIO1 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT11, false);
    /* Enable GPIO2 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT10, false);
    /* Enable GPIO3 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT9, false);
    /* Enable GPIO4 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT8, false);
    /* Enable GPIO5 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT7, false);
    /* Enable GPIO6 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT6, false);
    /* Enable GPIO7 */
    cpg_ret |=R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_9, R_CPG_BIT5, false);
    if (R_CPG_ERR_SUCCESS != cpg_ret)
    {
        R_PRINT_Log("[CioCr7Can]: prvCanClkEnable CAN R_PMA_CPG_SetModuleStopStat failed(%d)\n", cpg_ret);
    }

    /* Stop Clock */
    val = 0xA5A50000;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPCR, val); /* Clear register protection */
    val = 0x00000113;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPR, ~val);
    LOC_REG_WRITE32(RCAR_CPG_CANFDCKCR, val);    /* CANFD CLK to 40MHz */

    /* Start Clock */
    val = 0xA5A50000;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPCR, val); /* Clear register protection */
    val = 0x00000013;
    LOC_REG_WRITE32(RCAR_CPG_CPGWPR, ~val);
    LOC_REG_WRITE32(RCAR_CPG_CANFDCKCR, val);    /* CANFD CLK to 40MHz */
}

void prvCanSoftReset(uint32_t Unit)
{
    r_cpg_BitPos_t canbit;
    r_cpg_Error_t  cpg_ret;

    if (0 == Unit)
    {
        canbit = R_CPG_BIT16;   /* SRPC9 - CAN0 */
    }
    else
    {
        canbit = R_CPG_BIT15;   /* SRPC9 - CAN1 - Default */
    }

    /* CAN-IF0 or CAN-IF1 module reset */
    cpg_ret = R_PMA_CPG_SetResetStat(R_CPG_REGIDX_9, canbit, 1);
    if (R_CPG_ERR_SUCCESS != cpg_ret)
    {
        R_PRINT_Log("[CioCr7Can]: prvCanSoftReset CAN R_PMA_CPG_SetResetStat failed(%d), ResetStat(1)\n", cpg_ret);
    }

    delay_us(R_DELAY_ONE_RCLK_CYCLE_IN_US); /* 1 RCLK cycle wait */
    cpg_ret = R_PMA_CPG_SetResetStat(R_CPG_REGIDX_9, canbit, 0);
    if (R_CPG_ERR_SUCCESS != cpg_ret)
    {
        R_PRINT_Log("[CioCr7Can]: prvCanSoftReset CAN R_PMA_CPG_SetResetStat failed(%d), ResetStat(0)\n", cpg_ret);
    }

}
#endif /* #ifdef R_USE_CANFD */

