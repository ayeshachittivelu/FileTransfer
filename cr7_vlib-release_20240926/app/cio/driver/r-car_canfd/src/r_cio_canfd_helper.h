/*************************************************************************************************************
* cio_canfd_helper_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

/**@defgroup group3_canfd CAN-FD Driver Helper Functions
 * @ingroup cio_canfd_driver
 *
 * @brief Methods and defines commonly used across different driver files
 *
 * @{
 */


#ifndef R_CIO_CANFD_HELPER_H
#define  R_CIO_CANFD_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
   Section: Global (private helper) Defines
 */

/**
 * @def R_CIO_CANFD_PRV_ROUND_UP
 * @brief Calculate How many chunks of size d are required for a given size n
 *
 * @param[in] n - Size of the message
 * @param[in] d - Size of chunk
 *
 * @return number of chunks
 */
#define R_CIO_CANFD_PRV_ROUND_UP(n, d) \
    ( \
        ((n) + (d) - 1) / (d) \
    )

/**
 * @def R_CIO_CANFD_PRV_READX_LOOP
 * @brief  Loop check
 *
 * @param[in] op      - Register operation (8, 16 or 32 bit read)
 * @param[in] addr    - Register address
 * @param[in] val     - Value to be checked
 * @param[in] cond    - condition for succesfull competion
 * @param[in] loopcnt - max number of retries
 *
 * @return = 1 - OK
 * @return = 0 - NG
 *
 */
#define R_CIO_CANFD_PRV_READX_LOOP(op, addr, val, cond, loopcnt) \
    ({ \
        uint32_t __cnt = loopcnt; \
        while (__cnt--) { \
            (val) = op(addr); \
            if (cond) \
                break; \
        } \
        (cond) ? 1 : 0; \
    })

/**
 * @def R_CIO_CANFD_PRV_FOR_EACH_SET_BIT
 * @brief  A conditional for-loop
 *
 * "Walk" the given mask value bit by bit.
 * The operation inside the loop is only executed if the corresponding mask bit is set
 *
 * @param[in] bit     - loop counter variable, actual bit number
 * @param[in] mask    - mask value
 * @param[in] max     - For loop limit (total number of bits)
 *
 * @return void
 *
 */
#define R_CIO_CANFD_PRV_FOR_EACH_SET_BIT(bit, mask, max) \
    for (bit = 0; bit <  max; bit++) \
        if ( ((0x1 << bit) & mask) != 0)


/*******************************************************************************
   Section: Global (private) helper functions
 */


/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_UpdateReg
 */
/**
 * @brief  Set or clear the register bits defined by the mask parameter.
 *
 *  @param[in] Reg        - Register address
 *  @param[in] Mask       - Bit mask, bits set to 1 will be modified
 *  @param[in] Val        - Bit value to be set
 *
 *  @return void
 */
void R_CIO_CANFD_PRV_UpdateReg(size_t Reg, uint32_t Mask, uint32_t Val);

/*******************************************************************************
 * Function Name: R_CIO_CANFD_PRV_ReadReg
 */
/**
 * @brief  Read a register value.
 *
 * @param[in] Base       - Register base address
 * @param[in] Offset     - Register offset
 *
 * @return register value
 */
uint32_t R_CIO_CANFD_PRV_ReadReg(size_t Base, uint32_t Offset);

/*******************************************************************************
 * Function name: R_CIO_CANFD_PRV_WriteReg
 */
/**
 * @brief Write a register value.
 *
 *  @param[in] Base       - Register base address
 *  @param[in] Offset     - Register offset
 *  @param[in] Val        - Value to be written
 *
 *  @return void
 */
void R_CIO_CANFD_PRV_WriteReg(size_t Base, uint32_t Offset, uint32_t Val);

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_SetBit
 */
/**
 * @brief Set the bits marked by the value parameter.
 *
 *  The bits set as 1 in the Value parameter will be set. Other bits remain unchanged
 *
 *  @param[in] Base       - Register base address
 *  @param[in] Reg        - Register offset
 *  @param[in] Val        - Value (bits) to be set,
 *                          Val bit = 1 => register bit is set
 *
 *  @return void
 */
void R_CIO_CANFD_PRV_SetBit(size_t Base, uint32_t Reg, uint32_t Val);

/*******************************************************************************
 *  Function Name: R_CIO_CANFD_PRV_ClearBit
 */
/**
 * @brief  Clear the bits given by the Value parameter.
 *
 *  The bits set as 1 in the Value parameter will be cleared. Other bits remain unchanged
 *
 * Parameters:
 *  @param[in] Base       - Register base address
 *  @param[in] Reg        - Register offset
 *  @param[in] Val        - Value (bits) to be set,
 *                          Val bit = 1 => register bit is cleared
 *  @return void
 */
void R_CIO_CANFD_PRV_ClearBit(size_t Base, uint32_t Reg, uint32_t Val);


#ifdef __cplusplus
}
#endif

#endif /* R_CIO_CANFD_HELPER_H */

/** @} */
