/*************************************************************************************************************
* cio_api
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef R_CIO_API_H
#define R_CIO_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "r_cio_bridge.h"

/**@defgroup cio_um_library_api CIO UM Library
 * @ingroup cio
 * @{
 */

/*******************************************************************************
   Function: R_CIO_Init

   @brief Initialize CIO

   @return == 0 - OK
   @return != 0 - NG
 */

int R_CIO_Init(void);

/*******************************************************************************
   Function: R_CIO_DeInit

   @brief Deinitialize CIO

   @return == 0 - OK
   @return != 0 - NG
 */

int R_CIO_DeInit(void);


/*******************************************************************************
   Function: R_CIO_Open

   @brief Open a driver

   @param[in] Name              - Name of the message queue(must not be NULL)
   @param[in] Mode(not used)    - File mode

   @return == -1 - NG
   @return == -2 - Mutex lock error
   @return >  -1 - Id for driver communication
 */

int R_CIO_Open(const char *Name, const char *Mode);


/*******************************************************************************
   Function: R_CIO_Close

   @brief Close a driver

   @param[in] Id         - Id for communication with the driver

   @return == 0 - OK
   @return != 0 - NG
 */

int R_CIO_Close(int Id);


/*******************************************************************************
   Function: R_CIO_Read

   @brief Read from a driver

   @param[in]  Id        - Id for communication with the driver
   @param[out] Buffer    - Buffer to receive data(must not be NULL)
   @param[in]  MaxLen    - Maximum number of bytes to read

   @return >  -1 - Number of received bytes
   @return == -1 - NG
 */

ssize_t R_CIO_Read(int Id, char *Buffer, size_t MaxLen);


/*******************************************************************************
   Function: R_CIO_Write

   @brief Write to a driver

   @param[in] Id         - Id for communication with the driver
   @param[in] Buffer     - Buffer to send to the driver(must not be NULL)
   @param[in] Len        - Size of buffer to be sent

   @return >  -1 - Number of sent bytes
   @return == -1 - NG
 */

ssize_t R_CIO_Write(int Id, char *Buffer, size_t Len);


/*******************************************************************************
   Function: R_CIO_Status

   @brief Get Statis of the driver

   @param[in]     Id     - Id for communication with the driver
   @param[in/out] Par1   - Optional parameter
   @param[in/out] Par2   - Optional parameter
   @param[in/out] Par3   - Optional parameter

   @return != -1 - OK
   @return == -1 - NG
 */

ssize_t R_CIO_Status(int Id, void *Par1, void *Par2, void *Par3);


/*******************************************************************************
   Function: R_CIO_IoCtl

   @brief Escape function to interact with a driver.

   The intention is to be able do define own functions for a driver.
   Par1 whould be used to define the function, Par2 & Par3 can be used as
   parameters.

   @param[in]     Id     - Id for communication with the driver
   @param[in/out] Par1   - Usually a driver defined function
   @param[in/out] Par2   - Optional parameter
   @param[in/out] Par3   - Optional parameter

   @return != -1 - OK
   @return == -1 - NG
 */

ssize_t R_CIO_IoCtl(int Id, void *Par1, void *Par2, void *Par3);

/** @} */


#ifdef __cplusplus
}
#endif

#endif /* R_CIO_API_H */

