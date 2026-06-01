/*************************************************************************************************************
* canfd_sample_main
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : main.c
 * Version      : 1.0
 * Description  : main function for standalone canfd sample app.
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "rcar-xos/osal/r_osal.h"
#include "cio_server.h"
#include "taurus_app.h"
#include "canfd_sample.h"
#include "r_cio_api.h"
#include "r_print_api.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#define CANFD_THREAD_SERVER_ID      0x8000U                         /* RESOURCE_ID_BSP              */
#define CANFD_THREAD_CIO_SERVER_ID  (CANFD_THREAD_SERVER_ID + 62U)  /* OSAL_THREAD_CAN_CIO_SERVER   */
#define CANFD_THREAD_TAURUS_ID      (CANFD_THREAD_SERVER_ID + 63U)  /* OSAL_THREAD_CAN_TAURUS       */
#if defined(USE_CANFD_CR)
#define CANFD_THREAD_MAIN_ID        (CANFD_THREAD_SERVER_ID + 64U)  /* OSAL_THREAD_CAN_MAIN         */
#endif

osal_memory_manager_handle_t g_hndl_mmngr = NULL;

/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/

static int64_t loc_CioServerThread(void * Arg)
{
    cio_main();
    while (1) {
        R_OSAL_ThreadSleepForTimePeriod(10);
    }
    return 0;
}

static int64_t loc_TaurusThread(void * Arg)
{
    taurus_main();
    while (1) {
        R_OSAL_ThreadSleepForTimePeriod(10);
    }
    return 0;
}

#if defined(USE_CANFD_CR)
static int64_t loc_CanfdSampleThread(void * Arg)
{
    canfd_sample_main();
    while (1) {
        R_OSAL_ThreadSleepForTimePeriod(10);
    }
    return 0;
}
#endif

static void loc_CioInit(void *Arg) {
    /* This relies on R_CIO_Init being able to run before
       the scheduler has been started.
       If this cannot be done, you have to create a thread
       for this function.
    */
    R_CIO_Init();
}

void loc_MainThread(void* arg)
{
    e_osal_return_t         osal_ret;
    st_osal_thread_config_t thrd1_cfg;
    st_osal_thread_config_t thrd2_cfg;
    osal_thread_handle_t    cio_thread_handle       = OSAL_THREAD_HANDLE_INVALID;
    osal_thread_handle_t    taurus_thread_handle    = OSAL_THREAD_HANDLE_INVALID;
#if defined(USE_CANFD_CR)
    st_osal_thread_config_t thrd3_cfg;
    osal_thread_handle_t    canfdmain_thread_handle = OSAL_THREAD_HANDLE_INVALID;
#endif
    st_osal_mmngr_config_t mmngr_config;

    /* Init OSAL */
    osal_ret = R_OSAL_Initialize();
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("OSAL Initialization failed with error %d\n", osal_ret);
    }

    osal_ret = R_OSAL_MmngrGetOsalMaxConfig(&mmngr_config);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[main]: ERROR!!! R_OSAL_MmngrGetOsalMaxConfig return value = %d\n", osal_ret);
    }

    osal_ret = R_OSAL_MmngrOpen(&mmngr_config, &g_hndl_mmngr);
    if (OSAL_RETURN_OK != osal_ret) {
        R_PRINT_Log("[main]: ERROR!!! R_OSAL_MmngrOpen return value = %d\n", osal_ret);
    }

    osal_ret = R_OSAL_MmngrInitializeMmngrConfigSt(&mmngr_config);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[main]: ERROR!!! R_OSAL_MmngrInitializeMmngrConfigSt return value = %d\n", osal_ret);
    }

    loc_CioInit(0);

    thrd1_cfg.func       = (p_osal_thread_func_t)loc_CioServerThread;
    thrd1_cfg.userarg    = NULL;
    osal_ret = R_OSAL_ThreadCreate(&thrd1_cfg, CANFD_THREAD_CIO_SERVER_ID, &cio_thread_handle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[main]: ERROR!!! CioServerThread R_OSAL_ThreadCreate return value = %d\n", osal_ret);
    }

    thrd2_cfg.func       = (p_osal_thread_func_t)loc_TaurusThread;
    thrd2_cfg.userarg    = NULL;
    osal_ret = R_OSAL_ThreadCreate(&thrd2_cfg, CANFD_THREAD_TAURUS_ID, &taurus_thread_handle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[main]: ERROR!!! TaurusThread R_OSAL_ThreadCreate return value = %d\n", osal_ret);
    }


#if defined(USE_CANFD_CR)
    thrd3_cfg.func       = (p_osal_thread_func_t)loc_CanfdSampleThread;
    thrd3_cfg.userarg    = NULL;
    osal_ret = R_OSAL_ThreadCreate(&thrd3_cfg, CANFD_THREAD_MAIN_ID, &canfdmain_thread_handle);
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("[main]: ERROR!!! CanfdSampleThread R_OSAL_ThreadCreate return value = %d\n", osal_ret);
    }
#endif

#if 0
    /* Deinit OSAL */
    osal_ret = R_OSAL_Deinitialize();
    if (OSAL_RETURN_OK != osal_ret)
    {
        R_PRINT_Log("OSAL de-initialize failed with error %d\n", osal_ret);
        return -1;
    }

    /* Stop the tasks and timer running. */
    vTaskEndScheduler();
#endif
}

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/
int main(void)
{
    /* Start OS and initial thread */
    /* Note: parameter unused by AutoSAR, see TASK(maintask) */
    R_OSAL_StartOS(loc_MainThread);
    return 0;
}
