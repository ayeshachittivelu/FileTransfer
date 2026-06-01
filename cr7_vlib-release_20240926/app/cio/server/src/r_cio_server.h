/*************************************************************************************************************
* cio_server_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef SERVER_H
#define SERVER_H


#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
   Title: Server Module Interface

 */

/*******************************************************************************
   Section: Global Defines

   Definition of all global defines.
 */

#undef EXTERN
#ifndef SERVER_GLOBALS
#define EXTERN extern
#else
#define EXTERN
#endif

/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Function: ServerOpen

   Open a CIO server & queue for I/O operations.

   Parameters:
   Cmd        - Pointer to the OPEN command

   Returns:
   == 0       - Success
   != 0       - Failed
 */

int ServerOpen(r_cio_CmdMsg_t  *Cmd);

int ServerInit(void);


#ifdef __cplusplus
}
#endif

#endif /* SERVER_H */
