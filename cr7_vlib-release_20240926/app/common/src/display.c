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
 * File Name    : display.c
 * Version      : 1.0
 * Description  : Encapsulate display and layer operations.
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "display.h"

#include "r_prr_api.h"

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayConfig_E3
 * Description  : Configure a display for E3 SoC based on basic information.
 * Arguments    : DisplayPort       - RCAR display port
 *                DisplayPtr        - Pointer to output display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayConfig_E3(r_wm_DisplayPort_t DisplayPort, r_wm_Display_t * DisplayPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    /* Set WM video out device (DU) */
    switch (DisplayPort) {
    case R_WM_DISP_PORT_LVDS0:
        DisplayPtr->Dev = R_WM_DEV_VOUT0;
        break;
    case R_WM_DISP_PORT_LVDS1:
        DisplayPtr->Dev = R_WM_DEV_VOUT1;
        break;
    default:
        err = R_WM_ERR_INVALID_PROP;
        break;
    }

    /* General fixed settings */
    if (R_WM_ERR_SUCCESS == err) {
        DisplayPtr->Format          = R_WM_FMT_ARGB8888;
        DisplayPtr->BackgroundColor = 0xFF000000; /* black */
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayConfig_E3
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayConfig_H3
 * Description  : Configure a display for H3 SoC based on basic information.
 * Arguments    : DisplayPort       - RCAR display port
 *                DisplayPtr        - Pointer to output display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayConfig_H3(r_wm_DisplayPort_t DisplayPort, r_wm_Display_t * DisplayPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    /* Set WM video out device (DU) */
    switch (DisplayPort) {
    case R_WM_DISP_PORT_LVDS0:
        DisplayPtr->Dev = R_WM_DEV_VOUT0;
        break;
    case R_WM_DISP_PORT_HDMI0:
        DisplayPtr->Dev = R_WM_DEV_VOUT1;
        break;
    case R_WM_DISP_PORT_HDMI1:
        DisplayPtr->Dev = R_WM_DEV_VOUT2;
        break;
    case R_WM_DISP_PORT_VGA:
        DisplayPtr->Dev = R_WM_DEV_VOUT3;
        break;
    default:
        err = R_WM_ERR_INVALID_PROP;
        break;
    }

    /* General fixed settings */
    if (R_WM_ERR_SUCCESS == err) {
        DisplayPtr->Format          = R_WM_FMT_ARGB8888;
        DisplayPtr->BackgroundColor = 0xFF000000; /* black */
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayConfig_H3
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayConfig_M3
 * Description  : Configure a display for M3 (M3-W) SoC based on basic information.
 * Arguments    : DisplayPort       - RCAR display port
 *                DisplayPtr        - Pointer to output display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayConfig_M3(r_wm_DisplayPort_t DisplayPort, r_wm_Display_t * DisplayPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    /* Set WM video out device (DU) */
    switch (DisplayPort) {
    case R_WM_DISP_PORT_LVDS0:
        DisplayPtr->Dev = R_WM_DEV_VOUT0;
        break;
    case R_WM_DISP_PORT_HDMI0:
        DisplayPtr->Dev = R_WM_DEV_VOUT1;
        break;
    case R_WM_DISP_PORT_VGA:
        DisplayPtr->Dev = R_WM_DEV_VOUT2;
        break;
    default:
        err = R_WM_ERR_INVALID_PROP;
        break;
    }

    /* General fixed settings */
    if (R_WM_ERR_SUCCESS == err) {
        DisplayPtr->Format          = R_WM_FMT_ARGB8888;
        DisplayPtr->BackgroundColor = 0xFF000000; /* black */
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayConfig_M3
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayConfig_M3N
 * Description  : Configure a display for M3N SoC based on basic information.
 * Arguments    : DisplayPort       - RCAR display port
 *                DisplayPtr        - Pointer to output display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayConfig_M3N(r_wm_DisplayPort_t DisplayPort, r_wm_Display_t * DisplayPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    /* Set WM video out device (DU) */
    switch (DisplayPort) {
    case R_WM_DISP_PORT_LVDS0:
        DisplayPtr->Dev = R_WM_DEV_VOUT0;
        break;
    case R_WM_DISP_PORT_HDMI0:
        DisplayPtr->Dev = R_WM_DEV_VOUT1;
        break;
    case R_WM_DISP_PORT_VGA:
        DisplayPtr->Dev = R_WM_DEV_VOUT3;
        break;
    default:
        err = R_WM_ERR_INVALID_PROP;
        break;
    }

    /* General fixed settings */
    if (R_WM_ERR_SUCCESS == err) {
        DisplayPtr->Format          = R_WM_FMT_ARGB8888;
        DisplayPtr->BackgroundColor = 0xFF000000; /* black */
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayConfig_M3N
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayConfig
 * Description  : Configure a display based on basic information. Common wrapper that calls specific implementation
 *                based on the detected in run-time SoC type.
 * Arguments    : DisplayPort       - RCAR display port
 *                DisplayPtr        - Pointer to output display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayConfig(r_wm_DisplayPort_t DisplayPort, r_wm_Display_t * DisplayPtr)
{
    r_prr_DevName_t soc;

    soc = R_PRR_GetDevice();
    switch (soc) {
    case R_PRR_RCARE3:  return R_WM_DisplayConfig_E3(DisplayPort, DisplayPtr);
    case R_PRR_RCARH3:  return R_WM_DisplayConfig_H3(DisplayPort, DisplayPtr);
    case R_PRR_RCARM3W: return R_WM_DisplayConfig_M3(DisplayPort, DisplayPtr);
    case R_PRR_RCARM3N: return R_WM_DisplayConfig_M3N(DisplayPort, DisplayPtr);
    default: return R_WM_ERR_FAILED;
    }
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayConfig
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayInit
 * Description  : Initialize a display based on configuration.
 * Arguments    : DisplayPtr - Pointer to display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayInit(const r_wm_Display_t * DisplayPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if (!DisplayPtr) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        err = R_WM_DevInit(DisplayPtr->Dev);
    }

    if (R_WM_ERR_UNIT_ALREADY_INIT != err) {
        if (R_WM_ERR_SUCCESS == err) {
            err = R_WM_ScreenPropertySet(DisplayPtr->Dev, R_WM_SCREEN_PROP_FMT, (void *)&DisplayPtr->Format);
        }
        if (R_WM_ERR_SUCCESS == err) {
            err = R_WM_ScreenPropertySet(DisplayPtr->Dev, R_WM_SCREEN_PROP_BKGCOLOR,
                                         (void *)&DisplayPtr->BackgroundColor);
        }
    }
    else {
        /* Specified screen has been initialized and just return success */
        err = R_WM_ERR_SUCCESS;
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayInit
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayDeInit
 * Description  : Deinitialize a display based on configuration.
 * Arguments    : DisplayPtr - Pointer to display configuration instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayDeInit(const r_wm_Display_t * DisplayPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if (!DisplayPtr) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        err = R_WM_DevDeinit(DisplayPtr->Dev);
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayDeInit
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_DisplayEnable
 * Description  : Enable or disable a display.
 *                Display can be configured (by R_WM_ScreenPropertySet) only when it is disabled.
 * Arguments    : DisplayPtr - Pointer to display configuration instance.
 *                Enable     - True to enable otherwise disable.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_DisplayEnable(const r_wm_Display_t * DisplayPtr, bool Enable)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if (!DisplayPtr) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        err = R_WM_ScreenEnable(DisplayPtr->Dev, Enable);
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_DisplayEnable
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_LayerInit
 * Description  : Initialize a display layer based on input configuration.
 * Arguments    : LayerPtr = Pointer to display layer instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_LayerInit(r_wm_Layer_t * LayerPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if (!LayerPtr) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        err = R_WM_SurfaceGet(LayerPtr->ParentPtr->Dev, &LayerPtr->VoutSurface);
    }

    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_SurfacePropertySet(LayerPtr->ParentPtr->Dev, R_WM_SURF_PROP_POS, (void *)&LayerPtr->VoutSurface);
    }
    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_SurfacePropertySet(LayerPtr->ParentPtr->Dev, R_WM_SURF_PROP_SIZE, (void *)&LayerPtr->VoutSurface);
    }
    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_SurfacePropertySet(LayerPtr->ParentPtr->Dev, R_WM_SURF_PROP_COLORFMT, (void *)&LayerPtr->VoutSurface);
    }

    if ((R_WM_ERR_SUCCESS == err) && (0 != LayerPtr->VoutSurface.FBuf)) {
        err = R_WM_SurfacePropertySet(LayerPtr->ParentPtr->Dev, R_WM_SURF_PROP_BUFFER, (void *)&LayerPtr->VoutSurface);
        if (R_WM_ERR_SUCCESS == err) {
            err = R_WM_ScreenSurfacesFlush(LayerPtr->ParentPtr->Dev, 1);
        }
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_LayerInit
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_LayerDeInit
 * Description  : Deinitialize a display layer based on input configuration.
 * Arguments    : LayerPtr = Pointer to display layer instance.
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_LayerDeInit(r_wm_Layer_t * LayerPtr)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if (!LayerPtr) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        err = R_WM_SurfaceRelease(LayerPtr->ParentPtr->Dev, &LayerPtr->VoutSurface);
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_LayerDeInit
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_LayerFlush
 * Description  : Flush a frame buffer to specified layer.
 * Arguments    : LayerPtr       - Pointer to target layer
 *              : FrameBufferPtr - Pointer to frame buffer to be flushed.
 *              : WaitVSync    - Wait vsync singal (1) or not (0)
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_LayerFlush(r_wm_Layer_t * LayerPtr, void * FrameBufferPtr, uint32_t WaitVSync)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if ((!FrameBufferPtr) || (!LayerPtr)) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        LayerPtr->VoutSurface.FBuf = (uintptr_t)FrameBufferPtr;
//        LayerPtr->VoutSurface.FBufC0 = 0;
        LayerPtr->VoutSurface.FBufC1 = 0;

    }

    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_SurfacePropertySet(LayerPtr->ParentPtr->Dev, R_WM_SURF_PROP_BUFFER, (void *)&LayerPtr->VoutSurface);
    }
    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_ScreenSurfacesFlush(LayerPtr->ParentPtr->Dev, WaitVSync);
    }

    return err;
}

/**********************************************************************************************************************
 * End of function R_WM_LayerFlushMultiPlane
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_WM_LayerFlushMultiPlane
 * Description  : Flush a frame buffer to specified layer.
 * Arguments    : LayerPtr       - Pointer to target layer
 *              : FrameBuffer - Pointer to frame buffer for each plane to be flushed.
 *              : WaitVSync    - Wait vsync singal (1) or not (0)
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_LayerFlushMultiPlane(r_wm_Layer_t * LayerPtr, uintptr_t *FrameBuffer, uint32_t WaitVSync)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if ((!FrameBuffer) || (!LayerPtr)) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        LayerPtr->VoutSurface.FBuf = FrameBuffer[0];
        LayerPtr->VoutSurface.FBufC0 = FrameBuffer[1];
        LayerPtr->VoutSurface.FBufC1 = FrameBuffer[2];
    }

    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_SurfacePropertySet(LayerPtr->ParentPtr->Dev, R_WM_SURF_PROP_BUFFER, (void *)&LayerPtr->VoutSurface);
    }
    if (R_WM_ERR_SUCCESS == err) {
        err = R_WM_ScreenSurfacesFlush(LayerPtr->ParentPtr->Dev, WaitVSync);
    }

    return err;
}

/**********************************************************************************************************************
 * End of function R_WM_LayerFlush
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function Name: R_WM_LayerShow
 * Description  : Turn on or off specified display layer.
 * Arguments    : LayerPtr - Pointer to target layer
 *              : On     - 1:on, 0:off
 * Return Value : r_wm_Error_t returning from R_WM_xxx API.
 *********************************************************************************************************************/
r_wm_Error_t R_WM_LayerShow(r_wm_Layer_t * LayerPtr, bool On)
{
    r_wm_Error_t err = R_WM_ERR_SUCCESS;

    if (!LayerPtr) {
        err = R_WM_ERR_NULL_POINTER;
    }
    else {
        if (On && (!LayerPtr->VoutSurface.Handle)) {
            err = R_WM_LayerInit(LayerPtr);
        }
        else if ((!On) && LayerPtr->VoutSurface.Handle) {
            err = R_WM_SurfaceRelease(LayerPtr->ParentPtr->Dev, &LayerPtr->VoutSurface);
            if (R_WM_ERR_SUCCESS == err) {
                err = R_WM_ScreenSurfacesFlush(LayerPtr->ParentPtr->Dev, 0);
            }
        }
    }

    return err;
}
/**********************************************************************************************************************
 * End of function R_WM_LayerShow
 *********************************************************************************************************************/
