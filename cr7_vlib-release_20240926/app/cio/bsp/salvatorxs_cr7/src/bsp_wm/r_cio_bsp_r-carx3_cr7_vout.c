/*************************************************************************************************************
* cio_bsp_r-carx3_cr7_vout
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* Copyright (c) [2019-2020] Renesas Electronics (Europe) GmbH.
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#include <stdint.h>
#ifdef R_USE_WM
#include "r_wm_config.h"
#include "r_du_api.h"
//#include "r_i2c_api.h"
//#include "r_adv7511_api.h"
#include "r_prr_api.h"
#include "rcar-xos/osal/r_osal.h"
#include "r_cpg_api.h"
#include "r_pinctrl_api.h"
#include "r_delay_api.h"
#include "r_print_api.h"

/*
 * Section: Local Defines
 */

/**
 * PWM Register Address
 */
#define R_PWM_PWMCR1    0xE6E31000
#define R_PWM_PWMCNT1   0xE6E31004

/*
 * This is the target frame rate. It is used to calculate the pixel clock.
 */
#define LOC_TARGET_FPS  60

typedef enum {
    R_BOARD_EBISU,
    R_BOARD_SALVATOR_XS,
} r_cio_board_t;

typedef enum {
    R_FULLHD_1080P,
    R_WIDE_HD,
    R_WXGA,
    R_HD_720P,
    R_XGA,
    R_SVGA,
    R_WVGA,
    R_VGA,
    R_WIDE_HD_AUO,
} r_display_timings_t;

/*
 * List of common display resolutions and associated timings.
 * If the LVDS or HDMI fields are not set, they will be automatically calculated for you.
 */
static const r_wm_DispTiming_t loc_Disp_Timing[]={
    [R_FULLHD_1080P] = {
        .ScreenWidth = 1920,
        .ScreenHeight = 1080,
        .VoutFlags = (uint32_t) (R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                 | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        .VoutH = { 2200, 44, 148 },
        .VoutV = { 1125, 5, 36 },
    },

    [R_WIDE_HD] = {
        .ScreenWidth = 1920,
        .ScreenHeight = 720,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 2200, 44, 148 },
        .VoutV = { 765, 5, 38 },
    },

    [R_WXGA] = {
        .ScreenWidth = 1280,
        .ScreenHeight = 800,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 1680, 136, 200 },
        .VoutV = { 828, 3, 24 },
    },

    [R_HD_720P] = {
        .ScreenWidth = 1280,
        .ScreenHeight = 720,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 1650, 40, 220 },
        .VoutV = { 750, 5, 20 },
    },

    [R_XGA] = {
        .ScreenWidth = 1024,
        .ScreenHeight = 768,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 1344, 136, 160 },
        .VoutV = { 806, 6, 29 },
    },

    [R_SVGA] = {
        .ScreenWidth = 800,
        .ScreenHeight = 600,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 1056, 128, 88 },
        .VoutV = { 628, 4, 23 },
    },

    [R_WVGA] = {
        .ScreenWidth = 800,
        .ScreenHeight = 480,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 960, 32, 80 },
        .VoutV = { 499, 10, 6  },
    },

    [R_VGA] = {
        .ScreenWidth = 640,
        .ScreenHeight = 480,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 800, 96, 48 },
        .VoutV = { 525, 2, 33 },
    },
    [R_WIDE_HD_AUO] = {
        .ScreenWidth = 1920,
        .ScreenHeight = 720,
        .VoutFlags = (uint32_t)(R_WM_VOUT_FLAG_HSYNC_ACTHI | R_WM_VOUT_FLAG_VSYNC_ACTHI
                                | R_WM_VOUT_FLAG_DESYNC_ACTHI | R_WM_VOUT_FLAG_DVI_MODE),
        /* total/syncwidth/backporch */
        .VoutH = { 1920+20+30+30, 20, 30 },
        .VoutV = { 720+52+52+10, 10, 52 },
    },

};

static int loc_Adv7511_Initialised;

static r_cio_board_t loc_GetBoardId(void)
{
    /* INFO: We currently only support boards that can be determined by the SoC.
     * If there are two boards with the same SoC, we will probably have to use
     * a build time option to distinguish between them. */
    if (R_PRR_GetDevice() == R_PRR_RCARE3) {
        return R_BOARD_EBISU;
    }

    return R_BOARD_SALVATOR_XS;
}

static void loc_SetDisplaySettings(r_wm_DispTiming_t *Out, r_wm_VoutType_t VoutType, r_display_timings_t Disp)
{
    const r_wm_DispTiming_t *in_timing = loc_Disp_Timing + Disp;

    *Out = *in_timing;

    /* If the pixel clock is not set, calculate it */
    if (Out->PixelClock == 0) {
        Out->PixelClock = Out->VoutH.TotalPixel * Out->VoutV.TotalPixel * LOC_TARGET_FPS;
    }

    Out->VoutType = VoutType;
}

static void loc_SetBoardExtDotClk(r_cio_board_t boardId, r_du_Unit_t disp_unit, r_wm_DispTiming_t *timing)
{
    r_prr_DevName_t dev = R_PRR_GetDevice();

    /* Populate the dotclkin frequencies for the supported boards */
    if (boardId == R_BOARD_EBISU) {
        /* Ebisu board */
        timing->DotClkIn[0] = 74250000;         /* Crystal oscillator (X13) */
        timing->DotClkIn[1] = 0;
        timing->DotClkIn[2] = 0;
        timing->DotClkIn[3] = 0;
        timing->Extal = 48000000;               /* Crystal oscillator (X4) */
    } else {
        /* Salvator-XS board */
        timing->DotClkIn[0] = 148500000;        /* Versaclock6 OUT1 (U61) */
        timing->DotClkIn[1] = 33000000;         /* X21 */

        if (dev == R_PRR_RCARH3) {
            timing->DotClkIn[2] = 33000000;     /* X22 */
        } else if (dev == R_PRR_RCARM3W) {
            timing->DotClkIn[2] = 108000000;    /* Versaclock6 OUT2 (U61) */
        } else {
            timing->DotClkIn[2] = 0;
        }

        if ((dev == R_PRR_RCARH3) || (dev == R_PRR_RCARM3N)){
            timing->DotClkIn[3] = 108000000;    /* Versaclock6 OUT2 (U61) */
        } else {
            timing->DotClkIn[3] = 0;
        }
        timing->Extal = 16640000;               /* Crystal oscillator (X4) */
    }
}

static uint32_t loc_LvdsLuminanceCtrl(void)
{
    RcarPin_t pwm_pin = {
        .name = "PWM1_A",
        .ipsr = 1,
        .ipsrVal = 0,
        .ipsrPos = 20,
        .gpsr = 2,
        .gpsrPos = 7,
        .modsel = 1,
        .modselVal = 0,
        .modselMask = 0x00000001,
    };

    uint32_t ret = 1;
    r_cpg_Error_t cpg_ret = R_CPG_ERR_SUCCESS;

    /* setting for PWM */
    pfcConfig(&pwm_pin);

    /* PWM module enable */
    cpg_ret = R_PMA_CPG_SetModuleStopStat(R_CPG_REGIDX_5, R_CPG_BIT23, false);

    if (R_CPG_ERR_SUCCESS == cpg_ret)
    {
        /* PWM module reset */
        cpg_ret = R_PMA_CPG_SetResetStat(R_CPG_REGIDX_5, R_CPG_BIT23, 1);
        if (R_CPG_ERR_SUCCESS != cpg_ret)
        {
            R_PRINT_Log("[CioCr7Vout]: loc_LvdsLuminanceCtrl R_PMA_CPG_SetResetStat failed(%d). ResetStat(1).\n", cpg_ret);
        }
        delay_us(R_DELAY_ONE_RCLK_CYCLE_IN_US); /* 1 RCLK cycle wait */
        cpg_ret = R_PMA_CPG_SetResetStat(R_CPG_REGIDX_5, R_CPG_BIT23, 0);
        if (R_CPG_ERR_SUCCESS != cpg_ret)
        {
            R_PRINT_Log("[CioCr7Vout]: loc_LvdsLuminanceCtrl _PMA_CPG_SetResetStat failed(%d). ResetStat(0).\n", cpg_ret);
        }
    }
    else
    {
        R_PRINT_Log("[CioCr7Vout]: loc_LvdsLuminanceCtrl R_PMA_CPG_SetModuleStopStat failed(%d).\n", cpg_ret);
    }

    if (R_CPG_ERR_SUCCESS == cpg_ret) {
        /* PWMCR(ch1) */
        /* Stop PWM *//* Refer HWM 60.4 Usage Notes */
        *((volatile uint32_t *)R_PWM_PWMCR1)  = (uint32_t)0x00000000;

        /* set PWMCNT(ch1) */
        *((volatile uint32_t *)R_PWM_PWMCNT1) = (uint32_t)0x03FF0200;

        /* PWMCR(ch1) */
        /* Start PWM */
        *((volatile uint32_t *)R_PWM_PWMCR1)  = (uint32_t)0x00000001;

        ret = 0;
    }

    return ret;
}

static void loc_LvdsBackLightCtrl(void)
{
    /* setting for GPIO */
    pfcSetGPIO(6, 7 ,1);

    /* OutPut High */
    gpioSetOutput(6 ,7 ,1);

    return;
}

/*******************************************************************************
 * Section: global functions
 */

/*
 * Specify the available displays and their resolutions for this board.
 * In general, if connecting a monitor to the HDMI or RGB/DPAD outputs, you can
 * change the resolutions to any of the above listed in r_display_timings_t.
 */
uint32_t R_CIO_BSP_WM_PRV_VideoOutDisplays(uint32_t DuUnit, r_wm_DispTiming_t *Timings)
{
    r_cio_board_t boardId = loc_GetBoardId();
    uint32_t err = 0;
    if (boardId == R_BOARD_EBISU) {
        /*
         * R-Car E3 Ebisu board
         *
         * The board allows either of the LVDS outputs to be connected to the
         * ADV7511 HDMI transmitter. Either DU can be used for RGB/DPAB output.
         * However we only use LVDS0 with the ADV7511 using connector CN37, and
         * DU1 with RGB/DPAD using connector CN15.
         */
        if (DuUnit == 0) {
#ifdef USE_LVDS
#ifdef USE_WIDE_HD_AUO                        
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_WIDE_HD_AUO);
#else  /* USE_WIDE_HD_AUO */
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_WIDE_HD);
#endif /* USE_WIDE_HD_AUO */
#else  /* USE_LVDS */
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_FULLHD_1080P);
#endif /* USE_LVDS */
        } else if (DuUnit == 1) {
#ifdef USE_LVDS
#ifdef USE_WIDE_HD_AUO                        
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_WIDE_HD_AUO);
#else  /* USE_WIDE_HD_AUO */
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_WIDE_HD);
#endif /* USE_WIDE_HD_AUO */
#else  /* USE_LVDS */
            loc_SetDisplaySettings(Timings, R_WM_VOUT_VGA, R_WVGA);
#endif /* USE_LVDS */
        } else {
            R_PRINT_Log("[CioCr7Vout]: R_CIO_BSP_WM_PRV_VideoOutDisplays R_BOARD_EBISU Argument error. DuUnit(%ld)\n", DuUnit);
            err = 1;
        }
    } else {
        /* Salvator-XS boards */
        if (DuUnit == 0) {
            
            /* LVDS out on connector CN18 */
#ifdef USE_WIDE_HD_AUO                        
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_WIDE_HD_AUO);
#else
            loc_SetDisplaySettings(Timings, R_WM_VOUT_LVDS, R_WIDE_HD);
#endif            


            /* --- Luminance Control --- */
            err = loc_LvdsLuminanceCtrl();

            /* --- BackLight Control --- */
            if (0 == err) {
                loc_LvdsBackLightCtrl();
            } else {
                R_PRINT_Log("[CioCr7Vout]: R_CIO_BSP_WM_PRV_VideoOutDisplays loc_LvdsBackLightCtrl failed(%d).\n", err);
            }
        } else if (DuUnit == 1) {
            /* HDMI0 out on connector CN16 */
            loc_SetDisplaySettings(Timings, R_WM_VOUT_HDMI, R_FULLHD_1080P);
        } else if (DuUnit == 2) {
            if (R_PRR_GetDevice() == R_PRR_RCARH3) {
                /* HDMI1 out on connector CN17 */
                loc_SetDisplaySettings(Timings, R_WM_VOUT_HDMI, R_FULLHD_1080P);
            } else {
                /* ARGB out on connector CN15 */
                loc_SetDisplaySettings(Timings, R_WM_VOUT_VGA, R_VGA);
            }
        } else if (DuUnit == 3) {
            if (R_PRR_GetDevice() == R_PRR_RCARH3
		|| R_PRR_GetDevice() == R_PRR_RCARM3N) {
                /* ARGB out on connector CN15 */
                loc_SetDisplaySettings(Timings, R_WM_VOUT_VGA, R_XGA);
            } else {
                err = 1;
                R_PRINT_Log("[CioCr7Vout]: R_CIO_BSP_WM_PRV_VideoOutDisplays R-Car device type failed(%d).\n", err);
            }
        } else {
            R_PRINT_Log("[CioCr7Vout]: R_CIO_BSP_WM_PRV_VideoOutDisplays DuUnit failed(%d).\n", DuUnit);
            err = 1;
        }
    }

    return err;
}

/*
 * Setup any additional hardware, i.e. external to the R-Car device, for this board.
 */
uint32_t R_CIO_BSP_WM_PRV_VideoOutInit(uint32_t DuUnit, r_wm_DispTiming_t *timings)
{
    r_cio_board_t boardId = loc_GetBoardId();
    r_du_Unit_t disp_unit = (r_du_Unit_t)DuUnit;
    r_wm_VoutType_t wm_vout_type = timings->VoutType;
    uint32_t err = 0;
    loc_SetBoardExtDotClk(boardId, disp_unit, timings);

    /* On the Ebisu board, there is an ADV7511 HDMI Transmitter that converts
     * LVDS0 or LVDS1 signals to HDMI. */
   /* if ((loc_Adv7511_Initialised == 0) &&
        (boardId == R_BOARD_EBISU) &&
        (wm_vout_type == R_WM_VOUT_LVDS) &&
        ((disp_unit == R_DU_DU0) || (disp_unit == R_DU_DU1))) {
        err = R_ADV7511_Init(R_I2C_IF0, 0x39);
        if (0 != err) {
            R_PRINT_Log("[CioCr7Vout]: R_CIO_BSP_WM_PRV_VideoOutInit R_ADV7511_Init failed(%ld).\n", err);
        }
        loc_Adv7511_Initialised = 1;
    }*/

    return err;
}

#else

uint32_t R_CIO_BSP_WM_PRV_VideoOutDisplays(uint32_t DuUnit, r_wm_DispTiming_t *timings)
{
    uint32_t err = 0;
    return err;
}

uint32_t R_CIO_BSP_WM_PRV_VideoOutInit(uint32_t DuUnit, r_wm_DispTiming_t *timings)
{
    uint32_t err = 0;
    return err;
}

#endif
