/*************************************************************************************************************
* cio_vin_public
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef R_CIO_VIN_PUBLIC_H
#define R_CIO_VIN_PUBLIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**@addtogroup cio_vin CIO VIN User Interaction
 * @ingroup cio_bridge
 * @{
 */

/**
 * @def    R_CIO_VIN_ERR_XX
 * @brief  CIO VIN return value
 */
#define R_CIO_VIN_ERR_OK                   (0)
#define R_CIO_VIN_ERR_FAILED               (-1)
#define R_CIO_VIN_ERR_IOC_UNKNOWN          (-2)
#define R_CIO_VIN_ERR_NO_INST              (-3)
#define R_CIO_VIN_ERR_PARA                 (-10)
#define R_CIO_VIN_ERR_WRONG_STATE          (-100)
#define R_CIO_VIN_ERR_IOC_CHN_CONFIGED     (-101)
#define R_CIO_VIN_ERR_IOC_CHN_UNINIT       (-102)
#define R_CIO_VIN_ERR_IOC_CHN_NOT_STARTED  (-103)

/**
 * @struct  r_cio_vin_Csi2Type_t
 * @brief  csi2 type
 */
typedef enum {
    R_CIO_VIN_CSI2_TYPE_CSI20,
    R_CIO_VIN_CSI2_TYPE_CSI40
} r_cio_vin_Csi2Type_t;

/**
 * @struct  r_cio_vin_Config_t
 * @brief  Video input configuration information
 */
typedef struct r_cio_vin_Config {
    uint32_t ImageInWidth;          /**< length of the camera horizontal line */
    uint32_t ImageInHeight;         /**< length of the camera vertical line */
    uint32_t ImageScaledWidth;      /**< length of the scaled image horizontal line */
    uint32_t ImageScaledHeight;     /**< length of the scaled image vertical line */
    uint32_t ImageOutWidth;         /**< CL_HSIZE[11:0] of VnUDS_CLIP_SIZE */
    uint32_t ImageOutHeight;        /**< CL_VSIZE[11:0] of VnUDS_CLIP_SIZE */
    uint32_t ImageUvaof;            /**< UV Data Address Offset (unit:byte) */
    uint32_t ImageStride;           /**< Data size per one horizontal line (pixel) */
    uint8_t  VmcInf;                /**< VIN VMC register INF bit */
    uint8_t  VmcIm;                 /**< VIN VMC register IM bit */
    uint16_t DataRate;              /**< MIPI input data rate (Mbps) from ADV7482 */
    uint8_t  Lanes;                 /**< Number of lanes to use */
    bool     Interlaced;            /**< true:Interlace, false:Progressive */
    uint8_t  ColorSpaceOut;         /**< hdmi color space output from ADV7482 */
    uint8_t  BoundaryMbps;          /**< hdmi boundary 600Mbps from ADV7482 */
    uintptr_t FrameBuffer[3];       /**< Address of the capture buffer */
    r_cio_vin_Csi2Type_t Csi2Type;  /**< CSI2 type */
    uint32_t OutputFmt;             /**< vin output format */
    uint32_t  pos_x;                /**< Start Pixel Pre-Clip (SPPrC[11:0] of VnSPPrC) */
    uint32_t  pos_y;                /**< Start Pixel Pre-Clip (SLPrC[11:0] of VnSLPrC) */
} r_cio_vin_Config_t;

/**@enum r_cio_vin_Ioctl_t
 * @brief CIO VIN Control elements
 *
 * The list of functions and settings available for IOCTL
 */
typedef enum {
    R_CIO_VIN_IOC_NULL,
    R_CIO_VIN_IOC_CONFIG,
    R_CIO_VIN_IOC_GET_CONFIG,
    R_CIO_VIN_IOC_START_CAPTURE,
    R_CIO_VIN_IOC_STOP_CAPTURE,
    R_CIO_VIN_IOC_GET_VALID_FRAME,
    R_CIO_VIN_IOC_REGISTER_FRAME_READY_EVT,
    R_CIO_VIN_IOC_DISABLE_INTERRUPT,
    R_CIO_VIN_IOC_RESTART_CAPTURE,
    R_CIO_VIN_IOC_LAST
} r_cio_vin_Ioctl_t;

#ifdef __cplusplus
}
#endif

#endif /* R_CIO_VIN_PUBLIC_H */

/** @} */
