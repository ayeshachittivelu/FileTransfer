/*************************************************************************************************************
* cio_main_h
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/

#ifndef MAIN_H
#define MAIN_H


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
#ifndef MAIN_GLOBALS
#define EXTERN extern
#else
#define EXTERN
#endif


/*******************************************************************************
   Section: Global Defines
 */

#define ERR_MUTEX_RACE -10
#define ERR_SEMA_RACE  -20
#define ERR_TICK_DIFF  -30
#define ERR_COND_RACE  -40
#define ERR_MQ_RCV     -50
#define ERR_MQ_OPEN    -60
#define ERR_MQ_CLOSE   -70
#define ERR_MQ_UNLINK  -80
#define ERR_MQ_SEND    -90
#define ERR_BSP_INIT   -100
#define ERR_BSP_DEINIT -110


/*******************************************************************************
   Section: Global Functions
 */

/*******************************************************************************
   Function: Error

   Central error function.

   Parameters:
   Err        - Error code

   Returns:
   void
 */

void Error(int Err);


#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */

