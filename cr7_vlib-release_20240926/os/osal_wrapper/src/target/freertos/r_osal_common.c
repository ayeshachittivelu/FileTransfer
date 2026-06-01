/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_common.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper common functions for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"
#include "r_cpg_api.h"

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER
 * @defgroup OSAL_Private_Defines_Common Common macro definitions
 *
 * @{
 *********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @var IS_ISR
 * Check Context is ISR or Non-ISR
***********************************************************************************************************************/
#define IS_ISR() (0U != ulPortInterruptNesting)


/** @} */

/*******************************************************************************************************************//**
 * @var gs_osal_common_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_common_is_initialized = false;


/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER
 * @defgroup OSAL_Private_Defines_Static_Variables_Common Common Private static variables
 *
 * @{
 **********************************************************************************************************************/

OSAL_STATIC e_osal_return_t osal_internal_deinitialize_pre(void);
OSAL_STATIC void osal_internal_recover_init_state(uint16_t init_flg);
OSAL_STATIC void osal_internal_recover_io_init_state(uint16_t init_flg);
OSAL_STATIC e_osal_return_t osal_internal_io_deinitailize_pre(uint16_t *p_init_flg);
/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_Initialize()
* XOS1_OSAL_CD_CD_800
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_156]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Initialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (true == gs_osal_common_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_resolve_soc_dependencies();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_clock_time_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_thread_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_threadsync_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_mq_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_io_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_pm_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_interrupt_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_memory_initialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        gs_osal_common_is_initialized = true;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Deinitialize()
* XOS1_OSAL_CD_CD_801
* [Covers: XOS1_OSAL_UD_UD_002]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_157]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (true != gs_osal_common_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_internal_deinitialize_pre();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_pm_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_clock_time_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret =  osal_mq_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_threadsync_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_interrupt_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_thread_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_io_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_memory_deinitialize();
    }
    if (OSAL_RETURN_OK == osal_ret)
    {
        gs_osal_common_is_initialized = false;
    }
    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Deinitialize()
***********************************************************************************************************************/
#ifdef PRINT_CPU_IDLE
extern volatile TickType_t idleTaskTime;
extern volatile TickType_t totalTickTime;

void vPrintTask(void *pvParameters) {
    TickType_t lastWakeTime = xTaskGetTickCount();
    const TickType_t printInterval = pdMS_TO_TICKS(5000);

    for (;;) {
        uint32_t cpu_usage;
        uint32_t whole_part;
        uint32_t fraction_part;
        cpu_usage = (idleTaskTime * 1000 / totalTickTime);
        whole_part = (int)(cpu_usage/ 10);
        fraction_part = 10 - (cpu_usage % 10);
        R_PRINT_Log("CPU Idle: %d.%d%%\n", whole_part, fraction_part);
        vTaskDelayUntil(&lastWakeTime, printInterval);
    }
}
#endif

/***********************************************************************************************************************
* Start of function osal_internal_deinitialize_pre()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_deinitialize_pre(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    uint16_t        init_flg = 0U;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_clock_time_deinitialize_pre();
    if (OSAL_RETURN_OK == osal_ret)
    {
        init_flg |= OSAL_ISINIT_CLOCKTIME;
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_mq_deinitialize_pre();
        if (OSAL_RETURN_OK == osal_ret)
        {
            init_flg |= OSAL_ISINIT_MQ;
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_threadsync_deinitialize_pre();
        if (OSAL_RETURN_OK == osal_ret)
        {
            init_flg |= OSAL_ISINIT_THREADSYNC;
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_internal_io_deinitailize_pre(&init_flg);
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_thread_deinitialize_pre();
        init_flg |= OSAL_ISINIT_THREAD;
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        osal_internal_recover_init_state(init_flg);
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_recover_init_state()
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_recover_init_state(uint16_t init_flg)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_ISINIT_CLOCKTIME == (init_flg & OSAL_ISINIT_CLOCKTIME))
    {
        osal_clock_time_set_is_init();
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_ISINIT_MQ == (init_flg & OSAL_ISINIT_MQ))
    {
        osal_mq_set_is_init();
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_ISINIT_THREADSYNC == (init_flg & OSAL_ISINIT_THREADSYNC))
    {
        osal_threadsync_set_is_init();
    }
    else
    {
        /* Do nothing */
    }

    /* device related OSAL manager initialization flag recover. */
    osal_internal_recover_io_init_state(init_flg);

    if (OSAL_ISINIT_THREAD == (init_flg & OSAL_ISINIT_THREAD))
    {
        osal_thread_set_is_init();
    }
    else
    {
        /* Do nothing */
    }

    return;
}
/***********************************************************************************************************************
* End of function osal_internal_recover_init_state()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_recover_io_init_state()
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_recover_io_init_state(uint16_t init_flg)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_ISINIT_INTERRUPT == (init_flg & OSAL_ISINIT_INTERRUPT))
    {
        osal_interrupt_set_is_init();
    }
    else
    {
        /* Do nothing */
    }
#if 0
    if (OSAL_ISINIT_PM == (init_flg & OSAL_ISINIT_PM))
    {
        osal_pm_set_is_init();
    }
    else
    {
        /* Do nothing */
    }
#endif
    if (OSAL_ISINIT_IO == (init_flg & OSAL_ISINIT_IO))
    {
        osal_io_set_is_init();
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_ISINIT_MEMORY == (init_flg & OSAL_ISINIT_MEMORY))
    {
        osal_memory_set_is_init();
    }
    else
    {
        /* Do nothing */
    }

    return;
}
/***********************************************************************************************************************
* End of function osal_internal_recover_io_init_state()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_io_deinitailize_pre()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_io_deinitailize_pre(uint16_t *p_init_flg)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_interrupt_deinitialize_pre();
    if (OSAL_RETURN_OK == osal_ret)
    {
        *p_init_flg |= OSAL_ISINIT_INTERRUPT;
    }
    else
    {
        /* Do nothing */
    }
#if 0
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_pm_deinitialize_pre();
        if (OSAL_RETURN_OK == osal_ret)
        {
            *p_init_flg |= OSAL_ISINIT_PM;
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        /* Do nothing */
    }
#endif
    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_io_deinitialize_pre();
        if (OSAL_RETURN_OK == osal_ret)
        {
            *p_init_flg |= OSAL_ISINIT_IO;
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_ret = osal_memory_deinitialize_pre();
        if (OSAL_RETURN_OK == osal_ret)
        {
            *p_init_flg |= OSAL_ISINIT_MEMORY;
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_io_deinitailize_pre()
***********************************************************************************************************************/
void Irq_Setup(void);
e_osal_return_t R_OSAL_StartOS(void* Arg)
{
    OSAL_DEBUG_INFO("=== FreeRTOS Start ===\n");
    xTaskCreate((int *)Arg, "start_thread", 0x2000, NULL, OSAL_THREAD_PRIORITY_TYPE0, NULL);

    /* Start OS */
    /* Ensure no interrupts execute while the scheduler is in an inconsistent
       state.  Interrupts are automatically enabled when the scheduler is
       started. */
    portDISABLE_INTERRUPTS();

    Irq_Setup();

#ifdef PRINT_CPU_IDLE
    xTaskCreate(vPrintTask, "PrintTask", 0x1000, NULL, tskIDLE_PRIORITY + 1, NULL);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    return OSAL_RETURN_OK;
}

e_osal_return_t R_OSAL_CriticalEnter(void)
{
    taskENTER_CRITICAL();
    return OSAL_RETURN_OK;
}

e_osal_return_t R_OSAL_CriticalExit(void)
{
    taskEXIT_CRITICAL();
    return OSAL_RETURN_OK;
}

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_GetISRContext()
* XOS1_OSAL_CD_CD_802
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_338]
***********************************************************************************************************************/
void R_OSAL_Internal_GetISRContext(bool *is_rsr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != is_rsr)
    {
        if (IS_ISR())
        {
            *is_rsr = true;     /* ISR context */
        }
        else
        {
            *is_rsr = false;    /* task context */
        }
    }

    return;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_GetISRContext()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_CheckTimestamp()
* XOS1_OSAL_CD_CD_811
* [Covers: XOS1_OSAL_UD_UD_341]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Internal_CheckTimestamp(const st_osal_time_t *const p_time_stamp)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_time_stamp)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((OSAL_MAX_SEC < p_time_stamp->tv_sec) || (0L  > p_time_stamp->tv_sec))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((OSAL_MAX_TV_NSEC < p_time_stamp->tv_nsec) || (0L > p_time_stamp->tv_nsec))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = OSAL_RETURN_OK;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_CheckTimestamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_GetTimestamp()
* XOS1_OSAL_CD_CD_808
* [Covers: XOS1_OSAL_UD_UD_344]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Internal_GetTimestamp(st_osal_time_t *const p_time_stamp)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    bool is_isr = false;
    TickType_t current_tick;
    osal_milli_sec_t current_milli_sec;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_time_stamp)
    {
        OSAL_DEBUG("Thread Get TimeStamp: OSAL_RETURN_RAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        R_OSAL_Internal_GetISRContext(&is_isr);
        if (true == is_isr)
        {
            current_tick = xTaskGetTickCountFromISR();
        }
        else
        {
            current_tick = xTaskGetTickCount();
        }

        R_OSAL_Internal_TickToMillisec(&current_milli_sec, current_tick);
        p_time_stamp->tv_sec = (time_t)(current_milli_sec / OSAL_SECTOMSEC);
        p_time_stamp->tv_nsec = (int32_t)((current_milli_sec - (p_time_stamp->tv_sec * OSAL_SECTOMSEC)) * OSAL_MSECTONSEC);

        if ((0 > p_time_stamp->tv_nsec) || (OSAL_MAX_TV_NSEC < p_time_stamp->tv_nsec))
        {
            OSAL_DEBUG("Thread Get TimeStamp: OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_GetTimestamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_GetDiffTime()
* XOS1_OSAL_CD_CD_812
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_337]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Internal_GetDiffTime(const st_osal_time_t *const p_time2,
                                            const st_osal_time_t *const p_time1,
                                            osal_nano_sec_t *const p_time_difference)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    osal_nano_sec_t p_nano_time;


    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if ((NULL == p_time2) || (NULL == p_time1) || (NULL == p_time_difference))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = R_OSAL_Internal_CalcDiffTime(p_time2, p_time1, &p_nano_time);

        if (OSAL_RETURN_OK == osal_ret)
        {
            if ((((int64_t)(OSAL_MAX_SEC / 2) * (int64_t)(OSAL_SECTONSEC)) < p_nano_time) ||
                     (0 > p_nano_time))
            {
                *p_time_difference = 0;
            }
            else
            {
                *p_time_difference = p_nano_time;
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_GetDiffTime()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_CalcDiffTime()
* XOS1_OSAL_CD_CD_809
* [Covers: XOS1_OSAL_UD_UD_345]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Internal_CalcDiffTime(const st_osal_time_t *const p_time2,
                                             const st_osal_time_t *const p_time1,
                                             osal_nano_sec_t *const p_time_difference)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int64_t result_sec;
    int64_t result_nsec;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if ((NULL == p_time2) || (NULL == p_time1) || (NULL == p_time_difference))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        result_sec = (int64_t)(p_time2->tv_sec - p_time1->tv_sec);
        result_nsec = (int64_t)((int64_t)p_time2->tv_nsec - (int64_t)p_time1->tv_nsec);
        *p_time_difference = (result_sec * OSAL_SECTONSEC) + result_nsec;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_CalcDiffTime()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MillisecToTick()
* XOS1_OSAL_CD_CD_804
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_339]
***********************************************************************************************************************/
void R_OSAL_Internal_MillisecToTick(osal_milli_sec_t time_period, TickType_t *tick)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
   TickType_t calculate_tick;


    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != tick)
    {
        calculate_tick = (TickType_t)time_period * (TickType_t)configTICK_RATE_HZ;
        *tick = (TickType_t)( calculate_tick / (TickType_t)OSAL_SECTOMSEC);
    }
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MillisecToTick()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_TickToMillisec()
* XOS1_OSAL_CD_CD_805
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_340]
***********************************************************************************************************************/
void R_OSAL_Internal_TickToMillisec(osal_milli_sec_t *time_period, TickType_t tick)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    TickType_t calculate_time;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != time_period)
    {
        calculate_time = ((TickType_t)tick * (TickType_t)OSAL_SECTOMSEC) / (TickType_t)configTICK_RATE_HZ;
        *time_period = (osal_milli_sec_t) calculate_time;
    }
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_TickToMillisec()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_AddList()
* XOS1_OSAL_CD_CD_806
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_342]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_Internal_AddList(st_osal_event_list_t **list, st_osal_event_list_t *add_list)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    st_osal_event_list_t *local_list;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == list)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }
    else if (NULL == add_list)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        local_list = (st_osal_event_list_t *)(*list);
        if (NULL == local_list)
        {
            *list = add_list;
        }
        else
        {
            while ((NULL != local_list->next) && (add_list->bits != local_list->bits))
            {
                local_list = local_list->next;

            }
            if(add_list->bits != local_list->bits)
            {
            local_list->next = (struct st_osal_event_list *) add_list;
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_AddList()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_RemoveList()
***********************************************************************************************************************/
extern e_osal_return_t R_OSAL_Internal_RemoveList(st_osal_event_list_t **list, st_osal_event_list_t *remove_list)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    st_osal_event_list_t *local_list;
    st_osal_event_list_t *local_list_pre = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == list)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }
    else if (NULL == remove_list)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        local_list = (st_osal_event_list_t *)(*list);
        if (NULL != local_list)
        {
            while ((NULL != local_list) && (remove_list->bits != local_list->bits))
            {
                local_list_pre = local_list;
                local_list = local_list->next;
            }
 
          if (remove_list->bits == local_list->bits)
            {
             /* Head */
             if (NULL == local_list_pre)
                 {
                 local_list = (st_osal_event_list_t *)(*list);
                 *list = (st_osal_event_list_t *)(*list)->next;
                 local_list->next = NULL;
                 }
             /* Middle or Tail */
             else
                 {
                 local_list_pre->next = (struct st_osal_event_list *) local_list->next;
                 }
             /* Middle or Tail */
             }
        }
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_RemoveList()
***********************************************************************************************************************/


/***********************************************************************************************************************
* Start of function R_OSAL_Internal_GetHeadFromList()
* XOS1_OSAL_CD_CD_807
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_343]
***********************************************************************************************************************/
st_osal_event_list_t* R_OSAL_Internal_GetHeadFromList(st_osal_event_list_t **list_head)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    st_osal_event_list_t *local_list = NULL;
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == list_head)
    {
        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
    }
    else if (NULL == *list_head)
    {
        ; /* OSAL_RETURN_OK */
    }
    else
    {
        local_list = *list_head;
        *list_head = (st_osal_event_list_t *)(*list_head)->next;
        local_list->next = NULL;
    }

    return local_list;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_GetHeadFromList()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_CheckListEmpty()
* XOS1_OSAL_CD_CD_810
* [Covers: XOS1_OSAL_UD_UD_346]
***********************************************************************************************************************/
bool R_OSAL_Internal_CheckListEmpty(const st_osal_event_list_t * const list_head)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    bool empty = true;
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != list_head)
    {
        empty = false;
    }

    return empty;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_CheckListEmpty()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MsSleep()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MsSleep(osal_milli_sec_t milliseconds)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    TickType_t sleep_tick = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_MillisecToTick(milliseconds, &sleep_tick);
    vTaskDelay(sleep_tick);
    
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MsSleep()
***********************************************************************************************************************/
st_osal_inner_thread_attrinfo_t g_osal_inner_thread_attrinfo;
/***********************************************************************************************************************
* Start of function R_OSAL_Internal_SetThreadAttr()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_SetThreadAttr(const st_osal_inner_thread_attrinfo_t *const p_attr_info,
                                                     osal_inner_thread_attr_t *const p_attr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    (void)p_attr;
    g_osal_inner_thread_attrinfo.policy = p_attr_info->policy;
    g_osal_inner_thread_attrinfo.priority = p_attr_info->priority;
    g_osal_inner_thread_attrinfo.stack_size = p_attr_info->stack_size;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_SetThreadAttr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_ThreadCreate()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_ThreadCreate(const st_osal_inner_thread_config_t *const p_config,
                                                    osal_inner_thread_handle_t *const p_handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    BaseType_t freertos_ret;
    st_osal_inner_thread_control_t *p_tmp_handle;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_tmp_handle = (st_osal_inner_thread_control_t *)pvPortMalloc(sizeof(st_osal_inner_thread_control_t));
    if (NULL == p_tmp_handle)
    {
        osal_ret =OSAL_RETURN_MEM;
    }
    else
    {
        freertos_ret = xTaskCreate(
                        p_config->func,
                        "PMA Task",
                        (configSTACK_DEPTH_TYPE)OSAL_CONV_BYTE_TO_WORD(g_osal_inner_thread_attrinfo.stack_size),
                        (void *)p_config->userarg,
                        (UBaseType_t)OSAL_CONV_PRIORITY((uint16_t)g_osal_inner_thread_attrinfo.priority),
                        &p_tmp_handle->thread);
        if (errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY == freertos_ret)
        {
            vPortFree(p_tmp_handle);
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            *p_handle = p_tmp_handle;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_ThreadCreate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_ThreadTimedJoin()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_ThreadTimedJoin(osal_inner_thread_handle_t const handle,
                                                       int64_t *const p_return_value,
                                                       const osal_milli_sec_t time_period)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    TickType_t sleep_tick = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    (void)p_return_value;
    (void)time_period;
    
    R_OSAL_Internal_MillisecToTick(1000, &sleep_tick);
    vTaskDelay(sleep_tick);
    //vTaskDelete(handle->thread);
    vPortFree(handle);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_ThreadTimedJoin()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_SetMutexAttr()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_SetMutexAttr(osal_inner_mutexattr_t *const p_mutex_attr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    (void)p_mutex_attr;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_SetMutexAttr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MutexInit()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MutexInit(const osal_inner_mutexattr_t *const p_mutex_attr,
                                                 osal_inner_mutex_handle_t *const p_handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    st_osal_inner_mutex_control_t *p_tmp_handle;
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    (void)p_mutex_attr;

    p_tmp_handle = (st_osal_inner_mutex_control_t*)pvPortMalloc(sizeof(st_osal_inner_mutex_control_t));
    if (NULL == p_tmp_handle)
    {
        osal_ret =OSAL_RETURN_MEM;
    }
    else
    {
        p_tmp_handle->mutex = xSemaphoreCreateMutex();
        if (NULL == p_tmp_handle->mutex)
        {
            vPortFree(p_tmp_handle);
            osal_ret =OSAL_RETURN_MEM;
        }
        else
        {
            *p_handle = p_tmp_handle;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MutexInit()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MutexTimedLock()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MutexTimedLock(osal_inner_mutex_handle_t handle,
                                                      osal_milli_sec_t time_period)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    BaseType_t freertos_ret;
    TickType_t tickcount;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_MillisecToTick(time_period, &tickcount);
    freertos_ret = xSemaphoreTake(handle->mutex, tickcount);
    if (pdFAIL == freertos_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MutexTimedLock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MutexUnlock()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MutexUnlock(osal_inner_mutex_handle_t handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    BaseType_t freertos_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    freertos_ret = xSemaphoreGive(handle->mutex);
    if (pdFAIL == freertos_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MutexUnlock()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MutexDestroy()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MutexDestroy(osal_inner_mutex_handle_t handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    vSemaphoreDelete(handle->mutex);
    vPortFree(handle);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MutexDestroy()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MqCreate()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MqCreate(const st_osal_inner_mq_config_t *const p_config,
                                                osal_inner_mq_handle_t *const p_handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    st_osal_inner_mq_control_t *p_tmp_handle;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_tmp_handle = (st_osal_inner_mq_control_t *)pvPortMalloc(sizeof(st_osal_inner_mq_control_t));
    if (NULL == p_tmp_handle)
    {
        osal_ret =OSAL_RETURN_MEM;
    }
    else
    {
        p_tmp_handle->mqd = xQueueCreate(p_config->max_num_msg, p_config->msg_size);
        if (NULL == p_tmp_handle->mqd)
        {
            vPortFree(p_tmp_handle);
            osal_ret = OSAL_RETURN_MEM;
        }
        else
        {
            *p_handle = p_tmp_handle;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MqCreate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MqTimedSend()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MqTimedSend(osal_inner_mq_handle_t handle,
                                                   osal_milli_sec_t time_period,
                                                   const void * p_buffer,
                                                   size_t buffer_size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    BaseType_t freertos_ret;
    TickType_t tick_time = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    (void)buffer_size;
    R_OSAL_Internal_MillisecToTick(time_period, &tick_time);
    freertos_ret = xQueueSend(handle->mqd, p_buffer, tick_time);
    if (errQUEUE_FULL == freertos_ret)
    {
        osal_ret = OSAL_RETURN_TIME;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MqTimedSend()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MqTimedReceive()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MqTimedReceive(osal_inner_mq_handle_t handle,
                                                      osal_milli_sec_t time_period,
                                                      void * p_buffer,
                                                      size_t buffer_size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;
    BaseType_t freertos_ret;
    TickType_t tick_time = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_MillisecToTick(time_period, &tick_time);
    freertos_ret = xQueueReceive(handle->mqd, p_buffer, tick_time);
    if (errQUEUE_EMPTY == freertos_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MqTimedReceive()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_MqDelete()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_MqDelete(osal_inner_mq_handle_t handle)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    vQueueDelete(handle->mqd);
    vPortFree(handle);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_MqDelete()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_GetHighResoTimeStamp()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_GetHighResoTimeStamp(st_osal_time_t *const p_time_stamp)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = R_OSAL_Internal_GetTimestamp(p_time_stamp);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_GetHighResoTimeStamp()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_Internal_CalcTimeDifference()
***********************************************************************************************************************/
/* PRQA S 9106 1 # This function is intended to be used in common. */
e_osal_return_t R_OSAL_Internal_CalcTimeDifference(const st_osal_time_t *const p_time2, 
                                                          const st_osal_time_t *const p_time1,
                                                          osal_nano_sec_t *const p_time_difference)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret      = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = R_OSAL_Internal_CalcDiffTime(p_time2, p_time1, p_time_difference);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_Internal_CalcTimeDifference()
***********************************************************************************************************************/

