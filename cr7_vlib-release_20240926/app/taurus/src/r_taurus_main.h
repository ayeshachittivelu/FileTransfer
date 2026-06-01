/*************************************************************************************************************
* taurus_main_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
#ifndef TAURUS_MAIN_H
#define TAURUS_MAIN_H


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
   Title: Main Module Interface

   This header contains the declaration of all  specific
   functions and types of the main test module.
 */

/*******************************************************************************
   Section: Global Defines

   Definition of all global defines.
 */

#undef EXTERN
#ifndef TAURUS_MAIN_GLOBALS
#define EXTERN extern
#else
#define EXTERN
#endif


/*******************************************************************************
   Section: Global Defines
 */


/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Function: TAURUS_Error

   Central error function.

   Parameters:
   Err        - Error code

   Returns:
   void
 */

void TAURUS_Error(int Err);


#ifdef __cplusplus
}
#endif

#endif /* TAURUS_MAIN_H */

