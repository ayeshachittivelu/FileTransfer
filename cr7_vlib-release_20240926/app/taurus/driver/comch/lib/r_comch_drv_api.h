/*************************************************************************************************************
* taurus_comch_drv_api
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup group1 Taurus COMCHAN server public library API
 *
 * @brief This Cortex-R7 Taurus COMCHAN server is a part of the Taurus firmware.
 *
 * This server depends on CIO and ROS to communicate with the kernel driver
 * The Interface to the kernel is a typical I/O control with functions like
 * open, read, ioctl, etc.
 *
 * @{
 */


#ifndef R_COMCHAN_DRV_API_H
#define R_COMCHAN_DRV_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rcar-xos/osal/r_osal.h"

/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_Open
 */
/**
 * @brief Opens the COMCHAN server
 *
 * @param[in] Guest       - Guest ID (not used)
 * @param[in] Cmd         - Pointer to the command to process (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 *
 * @note Cmd->Par3->params.open->res  == 0 - OK \n
 *       Cmd->Par3->params.open->res  == 2 - Already Initialized or R_CIO_Open failed
 */
int R_COMCHAN_DRV_Open(int Guest, R_TAURUS_CmdMsg_t *Cmd);


/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_Close
 */
/**
 * @brief Closes the COMCHAN server
 *
 * @param[in] Guest       - Guest ID (not used)
 * @param[in] Cmd         - Pointer to the command to process (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 *
 * @note Cmd->Par3->params.close->res  == 0 - OK \n
 *       Cmd->Par3->params.close->res  == 2 - Not Initialized \n
 *       Cmd->Par3->params.close->res  == 3 - R_CIO_Close failed
 */
int R_COMCHAN_DRV_Close(int Guest, R_TAURUS_CmdMsg_t *Cmd);

/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_Read
 */
/**
 * @brief Read request to the COMCHAN server
 *
 * @param[in] Guest       - Guest ID (not used)
 * @param[in] Cmd         - Pointer to the command to process (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 *
 * @note Cmd->Par3->params.receive->res  == 0 - OK \n
 *       Cmd->Par3->params.receive->res  == 2 - Not Initialized \n
 *       Cmd->Par3->params.receive->res  == 3 - R_CIO_Read failed
 */
ssize_t R_COMCHAN_DRV_Read(int Guest, R_TAURUS_CmdMsg_t *Cmd);

/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_Write
 */
/**
 * @brief Write request to the COMCHAN server
 *
 * @param[in] Guest       - Guest ID (not used)
 * @param[in] Cmd         - Pointer to the command to process (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 *
 * @note Cmd->Par3->params.transmit->res  == 0 - OK \n
 *       Cmd->Par3->params.transmit->res  == 2 - Not Initialized \n
 *       Cmd->Par3->params.transmit->res  == 3 - R_CIO_Write failed
 */
ssize_t R_COMCHAN_DRV_Write(int Guest, R_TAURUS_CmdMsg_t *Cmd);

/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_IoCtl
 */
/**
 * @brief Request an IOCTL command to the COMCHAN server
 *
 * @param[in] Guest       - Guest ID (not used)
 * @param[in] Cmd         - Pointer to the command to process (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 */
ssize_t R_COMCHAN_DRV_IoCtl(int Guest, R_TAURUS_CmdMsg_t *Cmd);

/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_Status
 */
/**
 * @brief Query status of the COMCHAN server.
 *
 * @param[in] Guest       - Guest ID (not used)
 * @param[in] Cmd         - Pointer to the command to process (must not be NULL)
 *
 * @return  == 0 - OK
 * @return  != 0 - NG
 *
 * @note Cmd->Par3->params.get_comch_stats->res  == 0 - OK \n
 *       Cmd->Par3->params.get_comch_stats->res  == 1 - R_CIO_Status failed
 */
ssize_t R_COMCHAN_DRV_Status(int Guest, R_TAURUS_CmdMsg_t *Cmd);

/*******************************************************************************
 * Function Name: R_COMCHAN_DRV_SetSendSignalImpl
 */
/**
 * @brief Store the function pointer callback that is called from the server
 *
 * @param[in] SendSignalImpl       - Function callback
 *
 */
void R_COMCH_DRV_SetSendSignalImpl(int (*SendSignalImpl)(int Guest, R_TAURUS_SignalId_t Signal, void* Option));

#ifdef __cplusplus
}
#endif

/** @} */


#endif /* R_COMCHAN_DRV_API_H */
