/*************************************************************************************************************
* OSAL Sample App
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    osal_sample_memroy_main.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  Sample application to use OSAL Memory API
***********************************************************************************************************************/

#include "osal_sample_memory_common.h"

int alloc_use_main_task(st_osal_mmngr_config_t memConfig);
int init_obj_main_task();
int common_alloc_use(osal_memory_manager_handle_t memHandle);
void monitorCb(void* user_arg, e_osal_mmngr_event_t event,
    e_osal_return_t error, osal_memory_buffer_handle_t buffer_hndl);

/**
 * @defgroup osal_sample_memory OSAL Application: Sample For OSAL Memory Usage
 * @{
 */
/**
 * @brief The main function of OSAL Sample Memory application
 * - Call R_OSAL_Initialize and call the memory_task function. Then call R_OSAL_Deinitialize
 * - The memory_task is divided into 2 main tasks as below:
 *      -# @link alloc_use_main_task @endlink describes the function of Memory allocate and use
 *      -# @link init_obj_main_task @endlink describes the initialization memory buffer and handle object APIS
 * 
 * @return 0 on success
 * @return -1 on failure
 */
void test_main(void *arg)
{
    (void)arg;

    printf("================== osal_sample_memory start ==================\n");
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    bool success = true;
    st_osal_mmngr_config_t memMaxConfig;

    OSAL_SAMPLE_INFO("initialize osal\n");
    osal_ret = R_OSAL_Initialize();
    if(OSAL_RETURN_OK != osal_ret)
    {
        OSAL_SAMPLE_ERR("R_OSAL_Initialize was failed, osal_ret = %d\n", osal_ret);
        return(-1);
    }
    
    if (true == success)
    {
        OSAL_SAMPLE_INFO("Get the maximum memory configuration\n");
        osal_ret = R_OSAL_MmngrGetOsalMaxConfig(&memMaxConfig);
        if(OSAL_RETURN_OK != osal_ret)
        {
            success = false;
            OSAL_SAMPLE_ERR("Can not get the memory max configuration, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("execute alloc_use_main_task\n");
            app_ret = alloc_use_main_task(memMaxConfig);
            if(0 != app_ret)
            {
                success = false;
                OSAL_SAMPLE_ERR("alloc_use_main_task operation was failed\n");
            }
            else
            {
                OSAL_SAMPLE_INFO("alloc_use_main_task operation is completed\n");
            }

            OSAL_SAMPLE_INFO("execute init_obj_main_task\n");
            app_ret = init_obj_main_task();
            if(0 != app_ret)
            {
                success = false;
                OSAL_SAMPLE_ERR("init_obj_main_task operation was failed\n");
            }
            else
            {
                OSAL_SAMPLE_INFO("init_obj_main_task operation is completed\n");
            }
        }
        OSAL_SAMPLE_INFO("de-initialize osal\n");
        osal_ret = R_OSAL_Deinitialize();
        if(OSAL_RETURN_OK != osal_ret)
        {
            success = false;
            OSAL_SAMPLE_ERR("R_OSAL_Deinitialize was failed, osal_ret = %d\n", osal_ret);
        }
    }

    if(true == success)
    {
        OSAL_SAMPLE_INFO("osal_sample_memory run successfully\n");
    }
    else
    {
        OSAL_SAMPLE_ERR("osal_sample_memory was failed\n");
    }

    printf("================== osal_sample_memory finish ==================\n");
}

int main(void)
{
    /* Start OS and initial thread */
    /* Note: parameter unused by AutoSAR, see TASK(maintask) */
    R_OSAL_StartOS(test_main);
    return(-1);
}

/**
 * @defgroup alloc_use_main_task OSAL Memory allocation and use APIs
 * @{
 */
/**
 * @brief Alloc-use main task:<br>
 * (OSAL API UM reference: Chapter 2.9.1: allocate & use)
 * - Open a memory handle by using R_OSAL_MmngrOpen with max configuration
 * - Allocate and use memory by using @link common_alloc_use @endlink
 * - Close memory handle by using R_OSAL_MmngrClose
 * 
 * @param[in] memConfig The memory configuration
 * @return 0 on success
 * @return -1 on failure
 */
int alloc_use_main_task(st_osal_mmngr_config_t memConfig)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    osal_memory_manager_handle_t memHandle = OSAL_MEMORY_MANAGER_HANDLE_INVALID;

    printf("\n================== alloc_use_main_task start ==================\n");
    OSAL_SAMPLE_INFO("Open Memory Handle with the max configuration\n");
    osal_ret = R_OSAL_MmngrOpen(&memConfig, &memHandle);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("Open memory handle failed , osal_ret = %d\n", osal_ret);
    }
    else
    {
        /* Allocation and use */
        app_ret = common_alloc_use(memHandle);
        if(0 != app_ret)
        {
            OSAL_SAMPLE_ERR("Operation failed\n");
        }

        OSAL_SAMPLE_INFO("Close memory handle\n");
        osal_ret = R_OSAL_MmngrClose(memHandle);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Can not close memory manager, osal_ret = %d\n", osal_ret);
        }
    }

    printf("================== alloc_use_main_task finish ==================\n\n");
    return app_ret;
}

/**
 * @brief Memory allocation and use function:
 * - Initialize the memory configuration structure by using R_OSAL_MmngrInitializeMmngrConfigSt
 * - Get the current memory configuration by using R_OSAL_MmngrGetConfig
 * - Get the debug information of memory handle with steps at below:
 *      -# Open a file which use to store debug information by using fopen
 *      -# Get debug information of memory handle by using R_OSAL_MmngrDebugMmngrPrint
 *      -# Close this file by using fclose
 * - Setup the monitor callback for some events with the steps at below:
 *      -# Register monitor callback function @link monitorCb @endlink for OSAL_MMNGR_EVENT_ALLOCATE event by using R_OSAL_MmngrRegisterMonitor
 *      -# Register monitor callback function @link monitorCb @endlink for OSAL_MMNGR_EVENT_DEALLOCATE event by using R_OSAL_MmngrRegisterMonitor
 *      -# If allocation mode is OSAL_MMNGR_ALLOC_MODE_***_PROT, register monitor callback function @link monitorCb @endlink for OSAL_MMNGR_EVENT_OVERFLOW_DETECTED event by using R_OSAL_MmngrRegisterMonitor.
 *      -# If allocation mode is OSAL_MMNGR_ALLOC_MODE_***_PROT, register monitor callback function @link monitorCb @endlink for OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED event by using R_OSAL_MmngrRegisterMonitor
 * - Allocate buffers which size and aligment are 64-byte. Please refer to the steps at below:
 *      -# Use a "for" loop to allocate 3 buffers by using R_OSAL_MmngrAlloc
 *      -# If there is any failed allocation, de-allocate previous allocated buffer by using R_OSAL_MmngrDealloc
 * - Get the debug information of memory buffers with steps at below:
 *      -# Open a file which use to store debug information by using fopen
 *      -# Get debug information of 3 memory buffers by using R_OSAL_MmngrDebugBufferPrint
 *      -# Close this file by using fclose
 * - Get again the debug information of memory handle with steps at below:
 *      -# Open a file which use to store debug information by using fopen
 *      -# Get debug information of memory handle by using R_OSAL_MmngrDebugMmngrPrint
 *      -# Close this file by using fclose
 * - If allocation mode is OSAL_MMNGR_ALLOC_MODE_***_PROT, check memory buffer underflow or overflow by using R_OSAL_MmngrCheckBuffer.
 * - Get buffer size of allocated buffer by using R_OSAL_MmngrGetSize.<br>
 * The expected value of "bufferSize" is 64-byte
 * - Get CPU accessible (read/write) pointer by using R_OSAL_MmngrGetCpuPtr
 * - Get CPU accessible (read only) pointer by using R_OSAL_MmngrGetConstCpuPtr
 * - Get axi bus domain specific address of buffer by using R_OSAL_MmngrGetHwAddr
 * - Modify CPU buffer and synchronizes to HW by using R_OSAL_MmngrFlush
 * - Synchronizes the buffer HW memory to the CPU memory by using R_OSAL_MmngrInvalidate
 * - Dealloc allocated buffer with the steps at blow:
 *      -# If allocation mode is OSAL_MMNGR_ALLOC_MODE_STACK_***, de-allocate the buffer in reverse order of the allocation by using R_OSAL_MmngrDealloc
 *      -# If allocation mode is OSAL_MMNGR_ALLOC_MODE_FREELIST_***, de-allocate the buffer any order by using R_OSAL_MmngrDealloc.<br>
 *         In this case, sample app use the same order of the allocation.
 * 
 * @param[in] memHandle The memory handle
 * @return 0 on success
 * @return -1 on failure
 */
int common_alloc_use(osal_memory_manager_handle_t memHandle)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    st_osal_mmngr_config_t memConfig;
    osal_memory_buffer_handle_t buffHandle[3] = {OSAL_MEMORY_BUFFER_HANDLE_INVALID,
                                                    OSAL_MEMORY_BUFFER_HANDLE_INVALID,
                                                        OSAL_MEMORY_BUFFER_HANDLE_INVALID};
    size_t bufferSize = 0;
    void *cpuPTR;
    const void *cpuConstPTR;
    uintptr_t hwAddress;
    char mode[20];
    int i;

    if(OSAL_MEMORY_MANAGER_HANDLE_INVALID == memHandle)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("Invalid memory handle\n");
    }
    else
    {
        OSAL_SAMPLE_INFO("Initialize the memory configuration structure\n");
        osal_ret = R_OSAL_MmngrInitializeMmngrConfigSt(&memConfig);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Memory configuration initialization failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("Get the current memory configuration of opended memory handle\n");
            osal_ret = R_OSAL_MmngrGetConfig(memHandle, &memConfig);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("Get the configuration of opened memory manager failed, osal_ret = %d\n", osal_ret);
            }
            else if(memConfig.mode != OSAL_MMNGR_ALLOC_MODE_STACK && memConfig.mode != OSAL_MMNGR_ALLOC_MODE_STACK_PROT
                    && memConfig.mode != OSAL_MMNGR_ALLOC_MODE_FREE_LIST && memConfig.mode != OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("Allocation mode (%d) is invalid\n", memConfig.mode);
            }
            else
            {
                OSAL_SAMPLE_INFO("Dump the memory configuration information\n");
                switch (memConfig.mode)
                {
                    case OSAL_MMNGR_ALLOC_MODE_STACK :
                        printf("Allocation with mode OSAL_MMNGR_ALLOC_MODE_STACK\n");
                        sprintf(mode, "stack");
                        break;
                    case OSAL_MMNGR_ALLOC_MODE_STACK_PROT :
                        printf("Allocation with mode OSAL_MMNGR_ALLOC_MODE_STACK_PROT\n");
                        sprintf(mode, "stack_prot");
                        break;
                    case OSAL_MMNGR_ALLOC_MODE_FREE_LIST :
                        printf("Allocation with mode OSAL_MMNGR_ALLOC_MODE_FREE_LIST\n");
                        sprintf(mode, "freelist");
                        break;
                    case OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT :
                        printf("Allocation with mode OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT\n");
                        sprintf(mode, "freelist_prot");
                        break;
                    default:
                        break;
                }
                printf("Memory limit: %lu\n", (unsigned long)memConfig.memory_limit);
                printf("Maximum allowed allocations: %lu\n", (unsigned long)memConfig.max_allowed_allocations);
                printf("Maximum number of registered monitors callbacks: %lu\n", (unsigned long)memConfig.max_registered_monitors_cbs);
            }
        }

        if(0 == app_ret)
        {
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
            OSAL_SAMPLE_INFO("Get the debug information of memory handle and ptint to stdout\n");
            osal_ret = R_OSAL_MmngrDebugMmngrPrint(memHandle, stdout);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("Can not print memory manager information, osal_ret = %d\n", osal_ret);
            }
#endif

            if(0 == app_ret)
            {
                /* Setup the monitor callback. 
                 * OSAL_MMNGR_EVENT_OVERFLOW_DETECTED and OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED only be available in OSAL_MMNGR_ALLOC_MODE_***_PROT*/
                OSAL_SAMPLE_INFO("Setup the monitor callbacks for event OSAL_MMNGR_EVENT_ALLOCATE \n");
                osal_ret = R_OSAL_MmngrRegisterMonitor(memHandle, OSAL_MMNGR_EVENT_ALLOCATE, monitorCb, NULL);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Register monitor for OSAL_MMNGR_EVENT_ALLOCATE falied, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("Setup the monitor callbacks for event OSAL_MMNGR_EVENT_DEALLOCATE \n");
                    osal_ret = R_OSAL_MmngrRegisterMonitor(memHandle, OSAL_MMNGR_EVENT_DEALLOCATE, monitorCb, NULL);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Register monitor for OSAL_MMNGR_EVENT_DEALLOCATE falied, osal_ret = %d\n", osal_ret);
                    }
                    else if(OSAL_MMNGR_ALLOC_MODE_STACK_PROT == memConfig.mode || OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT == memConfig.mode)
                    {
                        OSAL_SAMPLE_INFO("Setup the monitor callbacks for event OSAL_MMNGR_EVENT_OVERFLOW_DETECTED \n");
                        osal_ret = R_OSAL_MmngrRegisterMonitor(memHandle, OSAL_MMNGR_EVENT_OVERFLOW_DETECTED, monitorCb, NULL);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Register monitor for OSAL_MMNGR_EVENT_OVERFLOW_DETECTED falied, osal_ret = %d\n", osal_ret);
                        }
                        else
                        {
                            OSAL_SAMPLE_INFO("Setup the monitor callbacks for event OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED \n");
                            osal_ret = R_OSAL_MmngrRegisterMonitor(memHandle, OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED, monitorCb, NULL);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("Register monitor for OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED falied, osal_ret = %d\n", osal_ret);
                            }
                        }
                    }
                    else
                    {
                        /* do nothing */
                    }
                }
            }

            if(0 == app_ret)
            {
                /* Allocate 3 buffers with the same size and aligment
                 * If there is any failed allocation, de-allocate the previous allocated buffers */
                OSAL_SAMPLE_INFO("Allocate 3 buffers with the same size (64-byte) and aligment (64-byte)\n");
                for(i = 0; i < sizeof(buffHandle)/sizeof(buffHandle[0]); i++)
                {
                    osal_ret = R_OSAL_MmngrAlloc(memHandle, ALLOC_SIZE, ALLOC_ALIGN, buffHandle + i);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Memory allocation falied, osal_ret = %d\n", osal_ret);
                        for(;i > 0; i--)
                        {
                            osal_ret = R_OSAL_MmngrDealloc(memHandle, *(buffHandle + (i - 1)));
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                OSAL_SAMPLE_INFO("Memory de-allocation falied, osal_ret = %d\n", osal_ret);
                            }
                        }
                        break;
                    }
                }

                /* Get the debug information of memory buffer */
                if(0 == app_ret)
                {
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
                    OSAL_SAMPLE_INFO("Print the debug information of 3 memory buffers to stdout\n");
                    for(i = 0; i < sizeof(buffHandle)/sizeof(buffHandle[0]); i++)
                    {
                        osal_ret = R_OSAL_MmngrDebugBufferPrint(buffHandle[i], stdout);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not print memory buffer information, osal_ret = %d\n", osal_ret);
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Print again the debug information of memory handle after the allocation completed\n");
                        osal_ret = R_OSAL_MmngrDebugMmngrPrint(memHandle, stdout);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not print memory manager information, osal_ret = %d\n", osal_ret);
                        }
                    }
#endif

                    if(0 == app_ret)
                    {
                        if(OSAL_MMNGR_ALLOC_MODE_STACK_PROT == memConfig.mode || OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT == memConfig.mode)
                        {
                            OSAL_SAMPLE_INFO("check buffer underflow or overflow for the first allocated buffer (only available in OSAL_MMNGR_ALLOC_MODE_***_PROT mode)\n");
                            OSAL_SAMPLE_INFO("No OSAL_MMNGR_EVENT_OVERFLOW_DETECTED or OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED event will be occur\n");
                            osal_ret = R_OSAL_MmngrCheckBuffer(buffHandle[0]);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("Can not check the underflow or overflow of allocated buffer, osal_ret = %d\n", osal_ret);
                            }
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Get buffer size of the first buffer. The returned size will be 64-byte\n");
                        osal_ret = R_OSAL_MmngrGetSize(buffHandle[0], &bufferSize);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Get allocated buffer size failed, osal_ret = %d\n", osal_ret);
                        }
                        else if(ALLOC_SIZE != bufferSize)
                        {
                            OSAL_SAMPLE_ERR("Operation failed\n");
                        }
                        else
                        {
                            /* R_OSAL_MmngrGetSize work normally */
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Get CPU accessible (read/write) pointer to the first allocated buffer\n");
                        osal_ret = R_OSAL_MmngrGetCpuPtr(buffHandle[0], &cpuPTR);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not get the CPU accessible (read/write) pointer of buffer, osal_ret = %d\n", osal_ret);
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Get CPU accessible (read only) pointer to the first allocated buffer\n");
                        osal_ret = R_OSAL_MmngrGetConstCpuPtr(buffHandle[0], &cpuConstPTR);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not get the CPU accessible (read only) pointer of buffer, osal_ret = %d\n", osal_ret);
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Get axi bus domain specific address which related to OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY\n");
                        osal_ret = R_OSAL_MmngrGetHwAddr(buffHandle[0], OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY, &hwAddress);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not get the axi bus domain specific address, osal_ret = %d\n", osal_ret);
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Make a char array (64-byte) which all elements content is 0x1\n");
                        OSAL_SAMPLE_INFO("Copy this array to first allocated buffer and synchronizes to HW memory with offset is 0, size is 64-byte\n");
                        char data[bufferSize];
                        (void)memset(data, 0x1, bufferSize);
                        (void)memcpy(cpuPTR, data, bufferSize);
                        osal_ret = R_OSAL_MmngrFlush(buffHandle[0], 0, bufferSize);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not synchronizes CPU memory to HW, osal_ret = %d\n", osal_ret);
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Synchronizes the buffer HW memory to the CPU memory with offset is 0, size is 64-byte\n")
                        osal_ret = R_OSAL_MmngrInvalidate(buffHandle[0], 0, bufferSize);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Can not synchronizes HW memory to CPU, osal_ret = %d\n", osal_ret);
                        }
                    }

                    /* Dealloc buffer 
                     * In case stack mode: Need to de-allocate in the reverse order of allocation 
                     * In case freelist mode: Can de-allocate in any order. In this case, sample uses the same order with the allocation */
                    if(0 == app_ret)
                    {
                        if(OSAL_MMNGR_ALLOC_MODE_STACK_PROT == memConfig.mode || OSAL_MMNGR_ALLOC_MODE_STACK == memConfig.mode)
                        {
                            OSAL_SAMPLE_INFO("Dealloc buffer. In case stack mode: Need to de-allocate in the reverse order of allocation\n")
                            for(i = (sizeof(buffHandle)/sizeof(buffHandle[0])) - 1; i >= 0 ; i--)
                            {
                                osal_ret = R_OSAL_MmngrDealloc(memHandle, buffHandle[i]);
                                if(OSAL_RETURN_OK != osal_ret)
                                {
                                    app_ret = -1;
                                    OSAL_SAMPLE_ERR("Deallocation in stack mode failed, osal_ret = %d\n", osal_ret);
                                }
                            }
                        }
                        else /* OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT or OSAL_MMNGR_ALLOC_MODE_FREE_LIST */
                        {
                            OSAL_SAMPLE_INFO("Dealloc buffer. In case freelist mode: Can de-allocate in any order\n")
                            for(i = 0; i < sizeof(buffHandle)/sizeof(buffHandle[0]); i++)
                            {
                                osal_ret = R_OSAL_MmngrDealloc(memHandle, buffHandle[i]);
                                if(OSAL_RETURN_OK != osal_ret)
                                {
                                    app_ret = -1;
                                    OSAL_SAMPLE_ERR("Deallocation in freelist mode failed, osal_ret = %d\n", osal_ret);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return app_ret;
}

/**
 * @brief Dummy monitor callback funcion
 * - Print the callback event for user
 */
void monitorCb(void* user_arg, e_osal_mmngr_event_t event,
    e_osal_return_t error, osal_memory_buffer_handle_t buffer_hndl)
{
    (void)user_arg;
    (void)error;
    (void)buffer_hndl;

    switch (event)
    {
        case OSAL_MMNGR_EVENT_ALLOCATE :
            OSAL_SAMPLE_INFO("====== EVENT: OSAL_MMNGR_EVENT_ALLOCATE ======\n");
            break;
        case OSAL_MMNGR_EVENT_DEALLOCATE :
            OSAL_SAMPLE_INFO("====== EVENT: OSAL_MMNGR_EVENT_DEALLOCATE ======\n");
            break;
        case OSAL_MMNGR_EVENT_OVERFLOW_DETECTED :
            OSAL_SAMPLE_INFO("====== EVENT: OSAL_MMNGR_EVENT_OVERFLOW_DETECTED ======\n");
            break;
        case OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED :
            OSAL_SAMPLE_INFO("====== EVENT: OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED ======\n");
            break;
        default:
            break;
    }
}
/**@} alloc-use*/

/**
 * @defgroup init_obj_main_task OSAL Memory Object Initialization APIs
 * @{
 */
/**
 * @brief Memory object initialization main task:
 * - Initialize all members of st_osal_memory_manager_obj to invalid/defined values by using R_OSAL_MmngrInitializeMemoryManagerObj.
 * - Initializes all members of st_osal_memory_buffer_obj to invalid/defined values by using R_OSAL_MmngrInitializeMemoryBufferObj.
 * 
 * @return 0 on success
 * @return -1 on failure
 */
int init_obj_main_task()
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    osal_memory_manager_t memHandleObject;
    osal_memory_buffer_t memBufferObject;

    printf("\n================== init_obj_main_task start ==================\n");
    OSAL_SAMPLE_INFO("Initializes all members of st_osal_memory_manager_obj to invalid/defined values\n");
    osal_ret = R_OSAL_MmngrInitializeMemoryManagerObj(&memHandleObject);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("Can not initialize the memory handle object, osal_ret = %d\n", osal_ret);
    }
    else
    {
        OSAL_SAMPLE_INFO("Initializes all members of st_osal_memory_buffer_obj to invalid/defined values\n");
        osal_ret = R_OSAL_MmngrInitializeMemoryBufferObj(&memBufferObject);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Can not initialize the memory buffer object, osal_ret = %d\n", osal_ret);
        }
    }

    printf("================== init_obj_main_task finish ==================\n\n");
    return app_ret;
}
/**@} object init*/

/**@} sample memory*/
