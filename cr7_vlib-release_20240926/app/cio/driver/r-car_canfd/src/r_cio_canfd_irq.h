/*************************************************************************************************************
* cio_canfd_irq_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/


/**@defgroup group5_canfd CAN-FD Driver interrupt intialization functions
 * @ingroup cio_canfd_driver
 *
 * @brief Init and deinit interrupt handling for the CIO server
 *
 * @{
 */



#ifndef R_CIO_CANFD_IRQ_H
#define  R_CIO_CANFD_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_IrqInit
 */
/**
 * @brief  Initialise CANFD Interrupt handling
 *
 *
 * @param[in] Inst       - Pointer to CANFD Device instance
 *
 * @return ==  0 - OK
 * @return == -1 - NG
 */
int32_t R_CIO_CANFD_PRV_IrqInit(r_cio_canfd_prv_DevInst_t *Inst);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_IrqDeInit
 */
/**
 * @brief
 * Deinitialise CANFD Interrupt handling
 *
 * @param[in]  Inst       - Pointer to CANFD Device instance
 *
 * @return ==  0 - OK
 * @return == -1 - NG
 */
int32_t R_CIO_CANFD_PRV_IrqDeInit(r_cio_canfd_prv_DevInst_t *Inst);


#ifdef __cplusplus
}
#endif

#endif

/** @} */
