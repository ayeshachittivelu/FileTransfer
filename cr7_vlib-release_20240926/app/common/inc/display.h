/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : display.h
 * Version      : 1.0
 * Description  : Encapsulate display and layer operations.
 *********************************************************************************************************************/

#ifndef DISPLAY_H_
#define DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "r_wm_api.h"
#include "r_wm_config.h"

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/
typedef enum
{
    R_WM_DISP_PORT_LVDS0, /* H3, H3N, M3, M3N, E3 */
    R_WM_DISP_PORT_LVDS1, /* E3 */
    R_WM_DISP_PORT_HDMI0, /* H3, H3N, M3, M3N */
    R_WM_DISP_PORT_HDMI1, /* H3 */
    R_WM_DISP_PORT_VGA    /* H3, H3N, M3, M3N */
} r_wm_DisplayPort_t;

typedef struct {
    r_wm_Dev_t      Dev;
    r_wm_ColorFmt_t Format;
    uint32_t        BackgroundColor;
} r_wm_Display_t;

typedef struct {
    r_wm_Surface_t   VoutSurface;
    r_wm_Display_t * ParentPtr;
} r_wm_Layer_t;

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/
extern r_wm_Error_t R_WM_DisplayConfig(r_wm_DisplayPort_t DisplayPort, r_wm_Display_t * DisplayPtr);
extern r_wm_Error_t R_WM_DisplayInit(const r_wm_Display_t * DisplayPtr);
extern r_wm_Error_t R_WM_DisplayDeInit(const r_wm_Display_t * DisplayPtr);
extern r_wm_Error_t R_WM_DisplayEnable(const r_wm_Display_t * DisplayPtr, bool Enable);
extern r_wm_Error_t R_WM_LayerInit(r_wm_Layer_t * LayerPtr);
extern r_wm_Error_t R_WM_LayerDeInit(r_wm_Layer_t * LayerPtr);
extern r_wm_Error_t R_WM_LayerFlush(r_wm_Layer_t * LayerPtr, void * FrameBufferPtr, uint32_t WaitVSync);
extern r_wm_Error_t R_WM_LayerFlushMultiPlane(r_wm_Layer_t * LayerPtr, uintptr_t *FrameBuffer, uint32_t WaitVSync);
extern r_wm_Error_t R_WM_LayerShow(r_wm_Layer_t * LayerPtr, bool On);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H_ */
