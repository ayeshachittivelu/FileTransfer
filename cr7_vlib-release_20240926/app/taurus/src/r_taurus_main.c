/*************************************************************************************************************
* taurus_main_c
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/*******************************************************************************
   Title: Main Module

   This module contains the main and the main loop functions.
 */

/*******************************************************************************
   Section: Includes
 */
#include "rcar-xos/osal/r_osal.h"
#include "r_taurus_bridge.h"
#include "r_taurus_main.h"
#include "r_comsv_main.h"
#include "r_print_api.h"


/*******************************************************************************
   Section: Local Defines
 */

/*******************************************************************************
   Define: LOC_MAINLOOP_STACKSIZE

   Stack size of the main loop thread.
 */

#define LOC_MAINLOOP_STACKSIZE ((uintptr_t)0x2000)


/*******************************************************************************
   Define: LOC_MAINLOOP_PRIO

   Priority of the main loop thread.
 */

#define LOC_MAINLOOP_PRIO      0x24

/*******************************************************************************
   Define: LOC_TAURUS_THREAD_ID

   Create thread ID.
 */

#define LOC_TAURUS_RESOURCE_ID 0x8000U                       /* RESOURCE_ID_BSP         */
#define LOC_TAURUS_THREAD_ID   (LOC_TAURUS_RESOURCE_ID + 8U) /* OSAL_THREAD_TAURUS_MAIN */


/*******************************************************************************
   Section: Local Variables
 */

static char loc_compile_time[] = __TIME__;
static char loc_compile_date[] = __DATE__;


/*******************************************************************************
   Variable: locStackMainLoopThread

   Stack of the main loop thread.
 */

static volatile int locQuit;


/*******************************************************************************
   Section: Local Functions
 */


/*******************************************************************************
   Function: loc_TaurusMainLoop

   The application main loop.

   This thread waits for commands sent by other threads
   and executes them.

   Parameters:
   Arg        - Thread parameter (not used).

   Returns:
   void*
 */

static int64_t loc_TaurusMainLoop(void * Arg)
{
    int x;
    e_osal_return_t osal_ret = OSAL_RETURN_FAIL;

    R_PRINT_Log("[TaurusMain]: loc_TaurusMainLoop START\r\n");
    locQuit = 0;
    x = R_COMSV_Init(1);
    if (x != 0) {
        R_PRINT_Log("[TaurusMain]: loc_TaurusMainLoop R_COMSV_Init failed(%d)\r\n", x);
        TAURUS_Error(-1);
    }
    do {
        osal_ret = R_OSAL_ThreadSleepForTimePeriod(10);
    } while (locQuit == 0);
    if (OSAL_RETURN_OK == osal_ret) {
        x = R_COMSV_DeInit(1);
        if (x != 0) {
            R_PRINT_Log("[TaurusMain]: loc_TaurusMainLoop R_COMSV_DeInit failed(%d)\r\n", x);
            TAURUS_Error(-1);
        }
    } else {
        R_PRINT_Log("[TaurusMain]: loc_TaurusMainLoop R_OSAL_ThreadSleepForTimePeriod failed(%d)\r\n", osal_ret);
        TAURUS_Error(-1);
    }

    R_PRINT_Log("[TaurusMain]: loc_TaurusMainLoop END\r\n");
    return 0;
}


/*******************************************************************************
   Function: Error
 */

void TAURUS_Error(int Err)
{
    R_PRINT_Log("[TaurusMain]: TAURAS NG\r\n");
}

/*******************************************************************************
   Section: Global Functions
 */

void TAURUS_SignalLeave(int Code)
{
    locQuit = 1;
}

/*******************************************************************************
   Function: main
 */

int taurus_main(void)
{
    e_osal_return_t             osal_ret = OSAL_RETURN_FAIL;
    osal_thread_handle_t        taurus_thread_handle;
    st_osal_thread_config_t     thread_config;

    R_PRINT_Log("\n\r[TaurusMain]: Taurus Virtual Peripheral Server %s %s START\n\r",
                  loc_compile_date, loc_compile_time);

    thread_config.func        = (p_osal_thread_func_t)loc_TaurusMainLoop;
    thread_config.userarg     = NULL;
    osal_ret = R_OSAL_ThreadCreate(&thread_config, LOC_TAURUS_THREAD_ID, &taurus_thread_handle);
    if (OSAL_RETURN_OK == osal_ret) {
            //R_OSAL_StartOS(); /* depending on the OS, this call may not return */

            /* The following code is only executed in case the OS
               supports leaving an application */
        osal_ret = R_OSAL_ThreadJoin(taurus_thread_handle, 0);
        if (OSAL_RETURN_OK != osal_ret) {
            R_PRINT_Log("[TaurusMain]: Thread join failed(%d)\n", osal_ret);
            TAURUS_Error(-1);
        }
    } else {
        R_PRINT_Log("[TaurusMain]: Thread create failed(%d)\n", osal_ret);
        TAURUS_Error(-1);
    }

    R_PRINT_Log("\n\r[TaurusMain]: Taurus Virtual Peripheral Server END\n\r\n\r");
    return 0;
}
