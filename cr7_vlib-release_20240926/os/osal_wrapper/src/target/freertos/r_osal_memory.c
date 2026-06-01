/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_memory.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper Memory Manager for FreeRTOS
***********************************************************************************************************************/

/**
 * @file r_osal_memory.c
 */
#include "target/freertos/r_osal_common.h"
#include "rcar-xos/osal/r_osal_memory_impl.h"
#include "rcar-xos/memory_allocator/r_meml.h"
#include <stdlib.h>
#include <string.h>

/*******************************************************************************************************************//**
 * @var gs_osal_memory_manager
 * Handle of memory manager object
***********************************************************************************************************************/
OSAL_STATIC osal_memory_manager_handle_t gs_osal_memory_manager = NULL;

/*******************************************************************************************************************//**
 * @var gs_osal_mem_mmngr_ptr_self
 * Pointor Self of memory manager object
***********************************************************************************************************************/
OSAL_STATIC st_osal_mem_mmngr_ptr_self_t *gs_osal_mem_mmngr_ptr_self = NULL;

/*******************************************************************************************************************//**
 * @var gs_osal_memory_is_initialized
 * Initialize status
***********************************************************************************************************************/
OSAL_STATIC uint32_t gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_NONE;

/*******************************************************************************************************************//**
 * @var gs_osal_memory_init_state
 * Initialize status to check deinitialize.
***********************************************************************************************************************/
OSAL_STATIC uint32_t gs_osal_memory_init_state = OSAL_MMNGR_STATE_NONE;

/*******************************************************************************************************************//**
 * @var SharedBase
 * The base address of the shared area to be defined by a upper layer software.
***********************************************************************************************************************/
extern const void * SharedBase;

/*******************************************************************************************************************//**
 * @var SharedLimit
 * The bottom address of the shared area to be defined by a upper layer software.
***********************************************************************************************************************/
extern const void * SharedLimit;

/** @} */

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FreeRTOS
 * @defgroup OSAL_Private_Functions_Memory Memory Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/
/*******************************************************************************************************************//**
* @brief            Fill the guard area with the pattern value.
* @param[in]        p_ptr_self   A pointer to Memory Manager handle.
* @param[in]        bytes        Requested size of the memory.
* @param[in]        alignment    Alignment of the memory.
* @param[out]       p_buffer_obj Handle to the allocate buffer object.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
* @retval           OSAL_RETURN_DEV
* @retval           OSAL_RETURN_CONF
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_PROHIBITED
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_alloc (void * p_ptr_self,
                                                     size_t bytes,
                                                     size_t alignment,
                                                     osal_memory_buffer_handle_t * const p_buffer_obj);

/*******************************************************************************************************************//**
* @brief            Fill the guard area with the pattern value.
* @param[in/out]    p_buffer_info     A pointer to the buffer information area.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_mem_alloc_fill_guard_area (st_osal_mem_buf_ptr_self_t * p_buffer_info);

/*******************************************************************************************************************//**
* @brief            Check the current buffer object for underflow or overflow.
* @param[in/out]    p_mngr_control  A pointer to the memory manager control table.
* @param[in/out]    p_buffer_info   A pointer to the buffer information area.
* @param[in]        bytes           Requested size of the memory.
* @param[in]        alignment       Alignment of the memory which has been adjusted in the system page.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_alloc_lib (st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                         st_osal_mem_buf_ptr_self_t * p_buffer_info,
                                                         size_t bytes,
                                                         size_t alignment);

/*******************************************************************************************************************//**
* @brief            Allocates memory using the system call.
* @param[in/out]    p_buffer_info  A pointer to the buffer information area.
* @param[in]        p_usr_config   A pointer to configuration to instanciate the OSAL Memory Manager.
* @param[in]        bytes          Requested size of the memory.
* @param[in]        alignment      Alignment of the memory which has been adjusted in the system page.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_DEV
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_alloc_map (st_osal_mem_buf_ptr_self_t * p_buffer_info,
                                                         st_osal_mmngr_config_t * p_usr_config,
                                                         size_t bytes,
                                                         size_t alignment);

/*******************************************************************************************************************//**
* @brief            Check the current buffer object for underflow or overflow.
* @param[in]        p_ptr_self      A pointer to Memory Buffer handle.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_CONF
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_checkbuffer (const void* p_ptr_self);

/*******************************************************************************************************************//**
* @brief            Close memory manager.
* @param[in]        hndl_mmngr       Memory Manager handle.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_close (osal_memory_manager_handle_t hndl_mmngr);

#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
/*******************************************************************************************************************//**
* @brief            Performs unit conversion and returns as a string.
* @param[in]        size       Conversion target size.
* @param[out]       p_output   Converted string.
* @param[in]        array_size Array size.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_conv_unit_and_str (size_t size, char * p_output, size_t array_size);
#endif

/*******************************************************************************************************************//**
* @brief            Deallocates memory.
* @param[in]        p_ptr_self A pointer to Memory Manager handle.
* @param[in]        buffer_obj A pointer to Memory Buffer Object.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_HANDLE_NOT_SUPPORTED
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_dealloc (void * p_ptr_self, osal_memory_buffer_handle_t buffer_obj);

/*******************************************************************************************************************//**
* @brief            Deallocates memory process.
* @param[in]        p_mngr_control   A pointer to the memory manager control table.
* @param[in]        buffer_obj       A pointer to Memory Buffer Object.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_MEM
* @retval           OSAL_RETURN_HANDLE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_dealloc_proc(st_osal_mem_mmngr_ptr_self_t *p_mngr_control,
                                                           osal_memory_buffer_handle_t buffer_obj);

#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
/*******************************************************************************************************************//**
* @brief            Output buffer information.
* @param[in]        p_ptr_self  A pointer to Memory Buffer handle.
* @param[out]       p_output    Output stream to write information.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_IO
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_debugbufferprint (const void* p_ptr_self, FILE* p_output);

/*******************************************************************************************************************//**
* @brief            Output buffer information.
* @param[in]        p_ptr_self  A pointer to Memory Manager handle.
* @param[out]       p_output    Output stream to write information.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_IO
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_PAR
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_debugmmngrprint (const void* p_ptr_self, FILE* p_output);
#endif

/*******************************************************************************************************************//**
* @brief            Find Memory Buffer Information Area.
* @param[in]        p_mngr_control   A pointer to the memory manager control table.
* @param[out]       pp_buffer_info   A pointer to store the address of the Memory Buffer Information Area.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_find_free_buffer_info(st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                                    st_osal_mem_buf_ptr_self_t ** pp_buffer_info);

/*******************************************************************************************************************//**
* @brief            Synchronize a sub region of the buffer CPU memory to the HW.
* @param[in]        p_ptr_self A pointer to Memory Buffer handle.
* @param[in]        offset     Offset with the buffer object limits.
* @param[in]        size       Size of the region to be synced.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_flush (void* p_ptr_self, size_t offset, size_t size);

/*******************************************************************************************************************//**
* @brief            Get Configuration Pointer.
* @param[in]        p_ptr_self A pointer to Memory Buffer handle.
* @param[out]       p_config   A pointer to store the configuration.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getconfig (const void* p_ptr_self, st_osal_mmngr_config_t* const p_config);

/*******************************************************************************************************************//**
* @brief            Get Const CPU Pointer.
* @param[in]        p_ptr_self       A pointer to Memory Buffer handle.
* @param[out]       pp_const_cpu_ptr A pointer to the cpu accessible memory.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getconstcpuptr (void* p_ptr_self, const void** const pp_const_cpu_ptr);

/*******************************************************************************************************************//**
* @brief            Get CPU Pointer.
* @param[in]        p_ptr_self A pointer to Memory Buffer handle.
* @param[out]       pp_cpu_ptr A pointer to the cpu modifiable memory.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getcpuptr (void* p_ptr_self, void** const pp_cpu_ptr);

/*******************************************************************************************************************//**
* @brief            Get allocate buffer size.
* @param[in]        p_ptr_self A pointer to Memory Buffer handle.
* @param[out]       p_size     A pointer to store the buffer size.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getsize (const void* p_ptr_self, size_t* const p_size);

/*******************************************************************************************************************//**
* @brief            Get HW Address.
* @param[in]        p_ptr_self A pointer to Memory Buffer handle.
* @param[in]        axi_id     AXI Bus ID.
* @param[out]       p_hw_addr  A pointer to store the hardware address.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_gethwaddr (const void* p_ptr_self,
                                                         osal_axi_bus_id_t axi_id,
                                                         uintptr_t* const p_hw_addr);

/*******************************************************************************************************************//**
* @brief            Synchronize a sub region of the buffer CPU memory to the HW.
* @param[in]        p_ptr_self A pointer to Memory Buffer handle.
* @param[in]        offset     Offset with the buffer object limits.
* @param[in]        size       Size of the region to be synced.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_invalidate (void* p_ptr_self, size_t offset, size_t size);

/*******************************************************************************************************************//**
* @brief            Open memory manager.
* @param[in]        p_usr_config     A pointer to configuration to instanciate the OSAL Memory Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_DEV
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_open (const st_osal_mmngr_config_t * const p_usr_config);

/*******************************************************************************************************************//**
* @brief            Open memory manager (allocation process).
* @param[in]        p_mngr_control   A pointer to the memory manager control table.
* @param[in]        p_usr_config     A pointer to configuration to instanciate the OSAL Memory Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_DEV
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_open_malloc (st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                           const st_osal_mmngr_config_t * const p_usr_config);

/*******************************************************************************************************************//**
* @brief            Open memory manager (mapping process).
* @param[in]        p_mngr_control   A pointer to the memory manager control table.
* @param[in]        p_usr_config     A pointer to configuration to instanciate the OSAL Memory Manager.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_DEV
* @retval           OSAL_RETURN_MEM
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_open_map (st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                        const st_osal_mmngr_config_t * const p_usr_config);

/*******************************************************************************************************************//**
* @brief            Register the user's callback function.
* @param[in]        p_ptr_self      A pointer to Memory Manager handle.
* @param[in]        monitored_event Event that shall be monitored by the provided monitor function.
* @param[in]        monitor_handler Function to be called by the mmngr for the monitored_event.
* @param[in]        p_user_arg      User argument that will be passed without modification/accessed to the monitor handler function.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_PROHIBITED
* @retval           OSAL_RETURN_HANDLE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_registermonitor (void* p_ptr_self,
                                                               e_osal_mmngr_event_t monitored_event,
                                                               fp_osal_memory_mngr_cb_t monitor_handler,
                                                               void* p_monitor_handler_user_arg);

/*******************************************************************************************************************//**
* @brief            Send monitor event.
* @param[in]        p_mutex_id     A pointer to mutex object.
* @param[in]        event_id       Event ID.
* @param[in]        error_code     Error code.
* @param[in]        p_mngr_control A pointer to the memory manager control table.
* @param[in]        buffer_hndl    Memory Buffer handle.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_send_monitor_event (SemaphoreHandle_t * p_mutex_id,
                                                                  e_osal_mmngr_event_t event_id,
                                                                  e_osal_return_t error_code,
                                                                  st_osal_mem_mmngr_ptr_self_t const * p_mngr_control,
                                                                  osal_memory_buffer_handle_t buffer_hndl);

/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrInitializeMmngrConfigSt()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrInitializeMmngrConfigSt(st_osal_mmngr_config_t* const p_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != p_config)
    {
        p_config->mode                        = OSAL_MMNGR_ALLOC_MODE_INVALID;
        p_config->memory_limit                = 0;
        p_config->max_allowed_allocations     = 0;
        p_config->max_registered_monitors_cbs = 0;
    }
    else
    {
        osal_ret = OSAL_RETURN_PAR;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrInitializeMmngrConfigSt()
***********************************************************************************************************************/


/***********************************************************************************************************************
* Start of function R_OSAL_MmngrInitializeMemoryManagerObj()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrInitializeMemoryManagerObj(osal_memory_manager_t* const p_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != p_obj)
    {
        p_obj->p_ptr_self        = NULL;
        p_obj->p_allocate        = NULL;
        p_obj->p_deallocate      = NULL;
        p_obj->p_getConfig       = NULL;
        p_obj->p_registerMonitor = NULL;
        p_obj->p_print           = NULL;
    }
    else
    {
        osal_ret = OSAL_RETURN_PAR;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrInitializeMemoryManagerObj()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrInitializeMemoryBufferObj()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrInitializeMemoryBufferObj(osal_memory_buffer_t* const p_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != p_obj)
    {
        p_obj->p_ptr_self       = NULL;
        p_obj->mmngr_owner_hndl = OSAL_MEMORY_MANAGER_HANDLE_INVALID;
        p_obj->p_getCpuPtr      = NULL;
        p_obj->p_getConstCpuPtr = NULL;
        p_obj->p_flush          = NULL;
        p_obj->p_invalidate     = NULL;
        p_obj->p_getHwAddr      = NULL;
        p_obj->p_getSize        = NULL;
        p_obj->p_checkBuffer    = NULL;
        p_obj->p_print          = NULL;
    }
    else
    {
        osal_ret = OSAL_RETURN_PAR;
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrInitializeMemoryBufferObj()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrOpen()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrOpen(const st_osal_mmngr_config_t* const p_config, osal_memory_manager_handle_t* const p_hndl_mmngr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t        osal_ret      = OSAL_RETURN_OK;
    st_osal_mmngr_config_t *p_max_config = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_INIT != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        if (OSAL_MEMORY_HANDLE_ID == gs_osal_mem_mmngr_ptr_self->handle_id)
        {
            osal_ret = OSAL_RETURN_STATE;
        }
        else if ((NULL == p_config) || (NULL == p_hndl_mmngr))
        {
            osal_ret = OSAL_RETURN_PAR;
        }
        else
        {
            osal_ret = R_OSAL_DCFG_GetMaxConfigOfMemory(&p_max_config);
            if ((OSAL_RETURN_OK != osal_ret) || (NULL == p_max_config))
            {
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                /* Do nothing */
            }
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            if (1 >  p_config->max_allowed_allocations)
            {
                osal_ret = OSAL_RETURN_CONF;
            }
            else if (p_max_config->max_allowed_allocations < p_config->max_allowed_allocations)
            {
                osal_ret = OSAL_RETURN_CONF;
            }
            else if (p_max_config->max_registered_monitors_cbs < p_config->max_registered_monitors_cbs)
            {
                osal_ret = OSAL_RETURN_CONF;
            }
            else if ((OSAL_MEMORY_MANAGER_CFG_UNLIMITED != p_config->memory_limit) &&
                     (p_max_config->memory_limit < p_config->memory_limit))
            {
                osal_ret = OSAL_RETURN_CONF;
            }
            else if (OSAL_MMNGR_MEM_4K > p_config->memory_limit)
            {
                osal_ret = OSAL_RETURN_CONF;
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
            switch (p_config->mode)
            {
                case OSAL_MMNGR_ALLOC_MODE_STACK:
                case OSAL_MMNGR_ALLOC_MODE_STACK_PROT:
                case OSAL_MMNGR_ALLOC_MODE_FREE_LIST:
                case OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT:
                    /* Do nothing */
                    break;
                case OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED:
                case OSAL_MMNGR_ALLOC_MODE_UNSPECIFIED_PROT:
                    osal_ret = OSAL_RETURN_CONF_UNSUPPORTED;
                    break;
                default:
                    osal_ret = OSAL_RETURN_CONF;
                    break;
            }
        }
        else
        {
           /* Do nothing */
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = osal_internal_mem_open(p_config);
            if (OSAL_RETURN_OK == osal_ret)
            {
                *p_hndl_mmngr = gs_osal_memory_manager;
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
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrOpen()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrClose()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrClose(osal_memory_manager_handle_t hndl_mmngr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_OPEN != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == hndl_mmngr)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == hndl_mmngr->p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *) hndl_mmngr->p_ptr_self;
        if (OSAL_MEMORY_HANDLE_ID != p_mngr_control->handle_id)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else if (NULL == p_mngr_control->p_buffers)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else if (NULL == p_mngr_control->p_monitor)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            osal_ret = osal_internal_mem_close(hndl_mmngr);
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_INIT;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrClose()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrAlloc()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrAlloc(osal_memory_manager_handle_t hndl_mmngr, size_t size, size_t alignment,
                                  osal_memory_buffer_handle_t* const p_buffer_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == hndl_mmngr)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_buffer_obj)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == hndl_mmngr->p_allocate)
    {
        osal_ret = OSAL_RETURN_HANDLE_NOT_SUPPORTED;
    }
    else
    {
        osal_ret = hndl_mmngr->p_allocate((void *)hndl_mmngr->p_ptr_self, size, alignment, p_buffer_obj);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrAlloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrDealloc()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrDealloc(osal_memory_manager_handle_t hndl_mmngr,
                                    osal_memory_buffer_handle_t  buffer_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if ((NULL == hndl_mmngr) || (NULL == buffer_obj))
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == hndl_mmngr->p_deallocate)
    {
        osal_ret = OSAL_RETURN_HANDLE_NOT_SUPPORTED;
    }
    else
    {
        osal_ret = hndl_mmngr->p_deallocate((void *)hndl_mmngr->p_ptr_self, buffer_obj);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrDealloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrGetConfig()
***********************************************************************************************************************/
/* PRQA S 3673 1 # This is requirement. Not changed for compatibility with other OSAL implementations. */
e_osal_return_t R_OSAL_MmngrGetConfig(osal_memory_manager_handle_t hndl_mmngr,
                                      st_osal_mmngr_config_t* const p_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t        osal_ret      = OSAL_RETURN_OK;
    st_osal_mmngr_config_t *p_max_config = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_MANAGER_HANDLE_INVALID == hndl_mmngr)
    {
        if (NULL == p_config)
        {
            osal_ret = OSAL_RETURN_PAR;
        }
        else
        {
            osal_ret = R_OSAL_DCFG_GetMaxConfigOfMemory(&p_max_config);
            if (OSAL_RETURN_OK == osal_ret)
            {
                p_config->mode                        = p_max_config->mode;
                p_config->memory_limit                = p_max_config->memory_limit;
                p_config->max_allowed_allocations     = p_max_config->max_allowed_allocations;
                p_config->max_registered_monitors_cbs = p_max_config->max_registered_monitors_cbs;
            }
            else
            {
                osal_ret = OSAL_RETURN_PAR;
            }
        }
    }
    else if (NULL == p_config)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == hndl_mmngr->p_getConfig)
    {
        osal_ret = OSAL_RETURN_HANDLE_NOT_SUPPORTED;
    }
    else
    {
        osal_ret = hndl_mmngr->p_getConfig((const void *)hndl_mmngr->p_ptr_self, p_config);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrGetConfig()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrGetOsalMaxConfig()
***********************************************************************************************************************/
/* PRQA S 1505 1 # This function is public API that is also called internally. */
e_osal_return_t R_OSAL_MmngrGetOsalMaxConfig(st_osal_mmngr_config_t* const p_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t        osal_ret      = OSAL_RETURN_OK;
    st_osal_mmngr_config_t *p_max_config = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_config)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = R_OSAL_DCFG_GetMaxConfigOfMemory(&p_max_config);
        if (OSAL_RETURN_OK == osal_ret)
        {
            p_config->mode                        = p_max_config->mode;
            p_config->memory_limit                = p_max_config->memory_limit;
            p_config->max_allowed_allocations     = p_max_config->max_allowed_allocations;
            p_config->max_registered_monitors_cbs = p_max_config->max_registered_monitors_cbs;
        }
        else
        {
            osal_ret = OSAL_RETURN_PAR;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrGetOsalMaxConfig()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrRegisterMonitor()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrRegisterMonitor(osal_memory_manager_handle_t hndl_mmngr,
                                            e_osal_mmngr_event_t monitored_event,
                                            fp_osal_memory_mngr_cb_t monitor_handler, void* p_user_arg)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_MANAGER_HANDLE_INVALID == hndl_mmngr)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == monitor_handler)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((OSAL_MMNGR_EVENT_INVALID >= monitored_event) || (OSAL_MMNGR_EVENT_ERROR <= monitored_event))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == hndl_mmngr->p_registerMonitor)
    {
        osal_ret = OSAL_RETURN_HANDLE_NOT_SUPPORTED;
    }
    else
    {
        osal_ret = hndl_mmngr->p_registerMonitor((void *)hndl_mmngr->p_ptr_self, monitored_event, monitor_handler, p_user_arg);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrRegisterMonitor()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrDebugMmngrPrint()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrDebugMmngrPrint(osal_memory_manager_handle_t hndl_mmngr, FILE* p_output)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == hndl_mmngr)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_output)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == hndl_mmngr->p_print)
    {
        osal_ret = OSAL_RETURN_HANDLE_NOT_SUPPORTED;
    }
    else
    {
        osal_ret = hndl_mmngr->p_print((const void *)hndl_mmngr->p_ptr_self, p_output);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrDebugMmngrPrint()
***********************************************************************************************************************/


/***********************************************************************************************************************
* Start of function R_OSAL_MmngrGetCpuPtr()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrGetCpuPtr(osal_memory_buffer_handle_t memory_buffer_obj_hndl, void** const pp_cpu_ptr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == pp_cpu_ptr)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == memory_buffer_obj_hndl->p_getCpuPtr)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_getCpuPtr((void *)memory_buffer_obj_hndl->p_ptr_self, pp_cpu_ptr);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrGetCpuPtr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrGetConstCpuPtr()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrGetConstCpuPtr(osal_memory_buffer_handle_t memory_buffer_obj_hndl,
                                           const void** const pp_const_cpu_ptr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == pp_const_cpu_ptr)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == memory_buffer_obj_hndl->p_getConstCpuPtr)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_getConstCpuPtr((void *)memory_buffer_obj_hndl->p_ptr_self, pp_const_cpu_ptr);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrGetConstCpuPtr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrFlush()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrFlush(osal_memory_buffer_handle_t memory_buffer_obj_hndl, size_t offset, size_t size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == memory_buffer_obj_hndl->p_flush)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_flush((void *)memory_buffer_obj_hndl->p_ptr_self, offset, size);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrFlush()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrInvalidate()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrInvalidate(osal_memory_buffer_handle_t memory_buffer_obj_hndl, size_t offset, size_t size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == memory_buffer_obj_hndl->p_invalidate)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_invalidate((void *)memory_buffer_obj_hndl->p_ptr_self, offset, size);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrInvalidate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrGetHwAddr()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrGetHwAddr(osal_memory_buffer_handle_t memory_buffer_obj_hndl,
                                      osal_axi_bus_id_t axi_id, uintptr_t * const p_hw_addr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_hw_addr)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_AXI_BUS_ID_INVALID == axi_id)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == memory_buffer_obj_hndl->p_getHwAddr)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_getHwAddr((const void *)memory_buffer_obj_hndl->p_ptr_self, axi_id, p_hw_addr);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrGetHwAddr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrGetSize()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrGetSize(osal_memory_buffer_handle_t memory_buffer_obj_hndl, size_t* const p_size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_size)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == memory_buffer_obj_hndl->p_getSize)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_getSize((const void *)memory_buffer_obj_hndl->p_ptr_self, p_size);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrGetSize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrCheckBuffer()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrCheckBuffer(osal_memory_buffer_handle_t memory_buffer_obj_hndl)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == memory_buffer_obj_hndl)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == memory_buffer_obj_hndl->p_checkBuffer)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = memory_buffer_obj_hndl->p_checkBuffer((const void *)memory_buffer_obj_hndl->p_ptr_self);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrCheckBuffer()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_MmngrDebugBufferPrint()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_MmngrDebugBufferPrint(osal_memory_buffer_handle_t hndl_buffer, FILE* p_output)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MEMORY_BUFFER_HANDLE_INVALID == hndl_buffer)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_output)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (NULL == hndl_buffer->p_print)
    {
        osal_ret = OSAL_RETURN_UNSUPPORTED_OPERATION;
    }
    else
    {
        osal_ret = hndl_buffer->p_print((const void *)hndl_buffer->p_ptr_self, p_output);
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_MmngrDebugBufferPrint()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_memory_initialize()
***********************************************************************************************************************/
e_osal_return_t osal_memory_initialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t     osal_ret    = OSAL_RETURN_OK;
    SemaphoreHandle_t mtx;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_NONE != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_INIT;

        gs_osal_mem_mmngr_ptr_self = (st_osal_mem_mmngr_ptr_self_t *)pvPortMalloc(sizeof(st_osal_mem_mmngr_ptr_self_t));
        if (NULL == gs_osal_mem_mmngr_ptr_self)
        {
            osal_ret = OSAL_RETURN_MEM;
        }
        else
        {
            gs_osal_memory_manager = (osal_memory_manager_handle_t)&gs_osal_mem_mmngr_ptr_self->mmngr_obj;

            gs_osal_mem_mmngr_ptr_self->inner_timeout    = OSAL_MEMORY_INNER_TIMEOUT;
            gs_osal_mem_mmngr_ptr_self->physical_addr    = 0;
            gs_osal_mem_mmngr_ptr_self->p_virtual_addr   = NULL;
            gs_osal_mem_mmngr_ptr_self->p_buffers        = NULL;
            gs_osal_mem_mmngr_ptr_self->num_buffers      = 0;
            gs_osal_mem_mmngr_ptr_self->use_buffer_total = 0;
            gs_osal_mem_mmngr_ptr_self->p_monitor        = NULL;
            gs_osal_mem_mmngr_ptr_self->num_monitor      = 0;
            gs_osal_mem_mmngr_ptr_self->handle_id        = 0;

            gs_osal_memory_manager->p_ptr_self = (void *)gs_osal_memory_manager;

            mtx = xSemaphoreCreateBinary();
            if (NULL == mtx)
            {
                OSAL_DEBUG("OSAL_RETURN_MEM\n");
                osal_ret = OSAL_RETURN_MEM;
            }
            else
            {
                gs_osal_mem_mmngr_ptr_self->inner_mtx      = mtx;
                (void)xSemaphoreGive(mtx);
            }
        }
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_memory_deinitialize();
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_memory_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_memory_deinitialize()
***********************************************************************************************************************/
e_osal_return_t osal_memory_deinitialize(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_memory_init_state     = gs_osal_memory_is_initialized;
    gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_NONE;

    /* If it is not closed, it forces the resource to be released. */
    if (OSAL_MMNGR_STATE_OPEN == gs_osal_memory_init_state)
    {
        (void)osal_internal_mem_close(gs_osal_memory_manager);
    }
    else
    {
        /* Do nothing */
    }

    if (NULL != gs_osal_mem_mmngr_ptr_self)
    {
        if (NULL != gs_osal_mem_mmngr_ptr_self->inner_mtx)
        {
            vSemaphoreDelete(gs_osal_mem_mmngr_ptr_self->inner_mtx);
            gs_osal_mem_mmngr_ptr_self->inner_mtx= NULL;
        }
        else
        {
            /* Do nothing */
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            vPortFree(gs_osal_mem_mmngr_ptr_self);
            gs_osal_mem_mmngr_ptr_self = NULL;
            gs_osal_memory_manager     = OSAL_MEMORY_MANAGER_HANDLE_INVALID;
            gs_osal_memory_init_state  = OSAL_MMNGR_STATE_NONE;
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
* End of function osal_memory_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_memory_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_memory_deinitialize_pre(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_NONE == gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_memory_init_state     = gs_osal_memory_is_initialized;
        gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_NONE;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_memory_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_memory_set_is_init()
***********************************************************************************************************************/
void osal_memory_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_memory_is_initialized = gs_osal_memory_init_state;
    gs_osal_memory_init_state     = OSAL_MMNGR_STATE_NONE;

    return;
}
/***********************************************************************************************************************
* End of function osal_memory_set_is_init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_open()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_open(const st_osal_mmngr_config_t * const p_usr_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;
    BaseType_t semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *) gs_osal_memory_manager->p_ptr_self;

    semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
    if (pdPASS != semaphore_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        if (OSAL_MEMORY_MANAGER_CFG_UNLIMITED != p_usr_config->memory_limit)
        {
            osal_ret = osal_internal_mem_open_map(p_mngr_control, p_usr_config);
        }
        else
        {
            /* Do nothing */
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            osal_ret = osal_internal_mem_open_malloc(p_mngr_control, p_usr_config);
        }
        else
        {
            /* Do nothing */
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            gs_osal_memory_manager->p_allocate        = osal_internal_mem_alloc;
            gs_osal_memory_manager->p_deallocate      = osal_internal_mem_dealloc;
            gs_osal_memory_manager->p_getConfig       = osal_internal_mem_getconfig;
            gs_osal_memory_manager->p_registerMonitor = osal_internal_mem_registermonitor;
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
            gs_osal_memory_manager->p_print           = osal_internal_mem_debugmmngrprint;
#else
            gs_osal_memory_manager->p_print           = NULL;
#endif

            p_mngr_control->config.mode                        = p_usr_config->mode;
            p_mngr_control->config.memory_limit                = p_usr_config->memory_limit;
            p_mngr_control->config.max_allowed_allocations     = p_usr_config->max_allowed_allocations;
            p_mngr_control->config.max_registered_monitors_cbs = p_usr_config->max_registered_monitors_cbs;

            p_mngr_control->handle_id = OSAL_MEMORY_HANDLE_ID;
            gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_OPEN;
        }
        else
        {
            /* Do nothing */
        }

        semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
        if (pdFAIL == semaphore_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* Do nothing */
        }
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_internal_mem_open()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_open_map()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_open_map(st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                       const st_osal_mmngr_config_t * const p_usr_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t     osal_ret     = OSAL_RETURN_OK;
    e_meml_return_t     meml_ret     = MEML_RETURN_OK;
    e_meml_alloc_mode_t meml_mode    = MEML_ALLOC_MODE_INVALID;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/

    if (OSAL_RETURN_OK == osal_ret)
    {
        p_mngr_control->size           = (p_usr_config->memory_limit + OSAL_MMNGR_MEM_RESERVE_SIZE) & ~4095U;
        p_mngr_control->p_virtual_addr = &SharedBase;
        if (NULL == p_mngr_control->p_virtual_addr)
        {
            osal_ret = OSAL_RETURN_MEM;
        }
        else
        {
            p_mngr_control->physical_addr  = (uintptr_t)p_mngr_control->p_virtual_addr;
        }
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        switch (p_usr_config->mode)
        {
            case OSAL_MMNGR_ALLOC_MODE_STACK:
            case OSAL_MMNGR_ALLOC_MODE_STACK_PROT:
                meml_mode = MEML_ALLOC_MODE_STACK;
                break;
            case OSAL_MMNGR_ALLOC_MODE_FREE_LIST:
            default:
                meml_mode = MEML_ALLOC_MODE_FREE_LIST;
                break;
        }

        meml_ret = R_MEML_Initialize(&p_mngr_control->meml_handle,
                                     (uintptr_t)p_mngr_control->physical_addr,
                                     (uintptr_t)p_mngr_control->p_virtual_addr,
                                     p_mngr_control->size,
                                     meml_mode);
        if (MEML_RETURN_OK != meml_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
            p_mngr_control->p_virtual_addr = NULL;
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
* End of function osal_internal_mem_open_map()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_open_malloc()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_open_malloc(st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                          const st_osal_mmngr_config_t * const p_usr_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t     osal_ret      = OSAL_RETURN_OK;
    size_t              loop_count    = 0;
    size_t              destroy_count = 0;
    SemaphoreHandle_t mtx;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/

    p_mngr_control->p_monitor = pvPortMalloc(sizeof(st_osal_mem_monitor_desc_t) * p_usr_config->max_registered_monitors_cbs);
    if (NULL == p_mngr_control->p_monitor)
    {
        osal_ret = OSAL_RETURN_MEM;
    }
    else
    {
       /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        p_mngr_control->p_buffers = pvPortMalloc(sizeof(st_osal_mem_buf_ptr_self_t) * p_usr_config->max_allowed_allocations);
        if (NULL == p_mngr_control->p_buffers)
        {
            osal_ret = OSAL_RETURN_MEM;
            vPortFree(p_mngr_control->p_monitor);
        }
        else
        {
            for (loop_count = 0; loop_count < p_usr_config->max_allowed_allocations; loop_count++)
            {
                p_mngr_control->p_buffers[loop_count].handle_id = OSAL_INVALID_HANDLE_ID;
            }
        }
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        loop_count = 0;
        while ((p_usr_config->max_allowed_allocations > loop_count) && (OSAL_RETURN_OK == osal_ret))
        {
            mtx = xSemaphoreCreateBinary();
            if (NULL == mtx)
            {
                osal_ret = OSAL_RETURN_MEM;
            }
            else
            {
                p_mngr_control->p_buffers[loop_count].inner_mtx = mtx;
                (void)xSemaphoreGive(mtx);
                p_mngr_control->p_buffers[loop_count].inner_timeout = OSAL_MEMORY_INNER_TIMEOUT;
            }
            loop_count++;
        }

        if (OSAL_RETURN_OK != osal_ret)
        {
            while (loop_count >= destroy_count)
            {
                (void)vSemaphoreDelete(p_mngr_control->p_buffers[destroy_count].inner_mtx);
                p_mngr_control->p_buffers[destroy_count].inner_mtx = NULL;
                destroy_count++;
            }
            vPortFree(p_mngr_control->p_monitor);
            vPortFree(p_mngr_control->p_buffers);
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
* End of function osal_internal_mem_open_malloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_close()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_close (osal_memory_manager_handle_t hndl_mmngr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;
    osal_memory_buffer_handle_t  mngr_buffer_hndl = NULL;
    e_meml_return_t              meml_ret        = MEML_RETURN_OK;
    size_t                       loop_count      = 0;
    ssize_t                      buff_count       = 0;
    BaseType_t                   semaphore_ret;
    size_t                       use_size         = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *) hndl_mmngr->p_ptr_self;

    semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
    if (pdPASS != semaphore_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        if (0 != p_mngr_control->num_buffers)
        {
            for (buff_count = ((ssize_t)p_mngr_control->num_buffers - (ssize_t)1); 0 <= buff_count; buff_count--)
            {
                mngr_buffer_hndl = (osal_memory_buffer_handle_t)&p_mngr_control->p_buffers[buff_count].mbuff_obj;
                if (NULL == mngr_buffer_hndl->p_ptr_self)
                {
                    osal_ret = OSAL_RETURN_HANDLE;
                }
                else
                {
                    use_size = p_mngr_control->size;

                    osal_ret = osal_internal_mem_dealloc_proc(p_mngr_control, mngr_buffer_hndl);
                    if (OSAL_RETURN_OK != osal_ret)
                    {
                        break;
                    }
                    else
                    {
                        p_mngr_control->num_buffers      -= 1;
                        p_mngr_control->use_buffer_total -= use_size;
                    }
                }
            }
        }
        else
        {
            /* Do nothing */
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            if (OSAL_MEMORY_MANAGER_CFG_UNLIMITED != p_mngr_control->config.memory_limit)
            {
                meml_ret = R_MEML_Deinitialize(&p_mngr_control->meml_handle);
                if (MEML_RETURN_OK == meml_ret)
                {
                    //vPortFree(p_mngr_control->p_virtual_addr);
                }
                else
                {
                    osal_ret = OSAL_RETURN_MEM;
                }
            }
            else
            {
                /* Do nothing */
            }
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            while ((p_mngr_control->config.max_allowed_allocations > loop_count) && (OSAL_RETURN_OK == osal_ret))
            {
                vSemaphoreDelete(p_mngr_control->p_buffers[loop_count].inner_mtx);
                p_mngr_control->p_buffers[loop_count].inner_mtx = NULL;
                loop_count++;
            }
        }
        else
        {
            /* Do nothing */
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            vPortFree(p_mngr_control->p_buffers);
            vPortFree(p_mngr_control->p_monitor);

            p_mngr_control->handle_id        = 0;
            p_mngr_control->physical_addr    = 0;
            p_mngr_control->num_buffers      = 0;
            p_mngr_control->use_buffer_total = 0;
            p_mngr_control->num_monitor      = 0;
            p_mngr_control->p_virtual_addr   = NULL;
            p_mngr_control->p_buffers        = NULL;
            p_mngr_control->p_monitor        = NULL;

            hndl_mmngr->p_allocate           = NULL;
            hndl_mmngr->p_deallocate         = NULL;
            hndl_mmngr->p_getConfig          = NULL;
            hndl_mmngr->p_registerMonitor    = NULL;
            hndl_mmngr->p_print              = NULL;

            /* Does not update the status when called osal_memory_deinitialize(). */
            if (OSAL_MMNGR_STATE_NONE == gs_osal_memory_init_state)
            {
                gs_osal_memory_is_initialized = OSAL_MMNGR_STATE_INIT;
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

        semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
        if (pdFAIL == semaphore_ret)
        {
            if (OSAL_RETURN_OK == osal_ret)
            {
                osal_ret = OSAL_RETURN_FAIL;
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
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_internal_mem_close()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_alloc()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_alloc(void * p_ptr_self,
                                                    size_t bytes,
                                                    size_t alignment,
                                                    osal_memory_buffer_handle_t * const p_buffer_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret         = OSAL_RETURN_OK;
    osal_memory_manager_handle_t mngr_handle      = NULL;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control  = NULL;
    st_osal_mem_buf_ptr_self_t   *p_buffer_info   = NULL;
    st_osal_mmngr_config_t       *p_max_config    = NULL;
    size_t                       memory_limit     = 0;
    BaseType_t                   semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_OPEN != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        mngr_handle = (osal_memory_manager_handle_t)p_ptr_self;
        if (NULL == mngr_handle->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *)mngr_handle->p_ptr_self;
            if (OSAL_MEMORY_HANDLE_ID != p_mngr_control->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else if (NULL == p_mngr_control->p_buffers)
            {
               osal_ret = OSAL_RETURN_STATE;
            }
            else if (NULL == p_mngr_control->p_monitor)
            {
               osal_ret = OSAL_RETURN_STATE;
            }
            else
            {
                osal_ret = R_OSAL_DCFG_GetMaxConfigOfMemory(&p_max_config);
                if ((OSAL_RETURN_OK != osal_ret) || (NULL == p_max_config))
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    /* Do nothing */
                }

                if (OSAL_RETURN_OK == osal_ret)
                {
                    if (OSAL_MEMORY_MANAGER_CFG_UNLIMITED == p_mngr_control->config.memory_limit)
                    {
                        memory_limit = p_max_config->memory_limit;
                    }
                    else
                    {
                        memory_limit = p_mngr_control->config.memory_limit;
                    }

                    if ((0U == bytes) || (memory_limit < bytes) || (0U != (bytes & 0x3FU)))
                    {
                        osal_ret = OSAL_RETURN_CONF;
                    }
                    else if ((0U== alignment) || (memory_limit < alignment) || (64U > alignment) || (0U != (alignment & (alignment -1U))))
                    {
                        osal_ret = OSAL_RETURN_CONF;
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
                    semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
                    if (pdPASS != semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        if (p_mngr_control->config.max_allowed_allocations <= p_mngr_control->num_buffers)
                        {
                            osal_ret = OSAL_RETURN_PROHIBITED;
                        }
                        else
                        {
                            /* Do nothing */
                        }

                        if (OSAL_RETURN_OK == osal_ret)
                        {
                            osal_ret = osal_internal_mem_find_free_buffer_info(p_mngr_control, &p_buffer_info);
                            if ((OSAL_RETURN_OK != osal_ret) || (NULL == p_buffer_info))
                            {
                                osal_ret = OSAL_RETURN_FAIL;
                            }
                            else
                            {
                                *p_buffer_obj = &p_buffer_info->mbuff_obj;
                            }
                        }
                        else
                        {
                            /* Do nothing */
                        }

                        if (OSAL_RETURN_OK == osal_ret)
                        {
                            if ((OSAL_MMNGR_ALLOC_MODE_STACK_PROT == p_mngr_control->config.mode) ||
                                (OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT == p_mngr_control->config.mode))
                            {
                                p_buffer_info->chunk_size = ((((OSAL_MMNGR_GUARD_PAGE_SIZE + 
                                                             (alignment - 1)) & ~(alignment - 1)) * 2) + 
                                                             ((bytes + (alignment - 1)) & ~(alignment - 1)));
                            }
                            else
                            {
                                p_buffer_info->chunk_size = ((bytes + (alignment - 1)) & ~(alignment - 1));
                            }

                            if (OSAL_MEMORY_MANAGER_CFG_UNLIMITED != p_mngr_control->config.memory_limit)
                            {
                                osal_ret = osal_internal_mem_alloc_lib(p_mngr_control, p_buffer_info, bytes, (size_t)alignment);
                            }
                            else
                            {
                                osal_ret = osal_internal_mem_alloc_map(p_buffer_info,
                                                                       &p_mngr_control->config, 
                                                                       bytes, 
                                                                       (size_t)alignment);
                            }

                            if (OSAL_RETURN_OK != osal_ret)
                            {
                                *p_buffer_obj = OSAL_MEMORY_BUFFER_HANDLE_INVALID;
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
                            if ((OSAL_MMNGR_ALLOC_MODE_STACK_PROT == p_mngr_control->config.mode) ||
                                (OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT == p_mngr_control->config.mode))
                            {
                                osal_internal_mem_alloc_fill_guard_area(p_buffer_info);
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
                            p_mngr_control->num_buffers      += 1;
                            p_mngr_control->use_buffer_total += p_buffer_info->size;

                            p_buffer_info->handle_id     = OSAL_BUFFER_HANDLE_ID;
                            p_buffer_info->buffer_id     = p_mngr_control->num_buffers;

                            if(OSAL_MEMORY_MANAGER_CFG_UNLIMITED != p_mngr_control->config.memory_limit)
                            {
                                p_buffer_info->lib_mngt_size = ((sizeof(st_meml_buffer_t) - (alignment - 1)) & ~(alignment - 1));
                            }
                            else
                            {
                                p_buffer_info->lib_mngt_size = 0;
                            }

                            p_buffer_info->mbuff_obj.p_ptr_self       = (void *)*p_buffer_obj;
                            p_buffer_info->mbuff_obj.mmngr_owner_hndl = mngr_handle;
                            p_buffer_info->mbuff_obj.p_getCpuPtr      = osal_internal_mem_getcpuptr;
                            p_buffer_info->mbuff_obj.p_getConstCpuPtr = osal_internal_mem_getconstcpuptr;
                            p_buffer_info->mbuff_obj.p_flush          = osal_internal_mem_flush;
                            p_buffer_info->mbuff_obj.p_invalidate     = osal_internal_mem_invalidate;
                            p_buffer_info->mbuff_obj.p_getHwAddr      = osal_internal_mem_gethwaddr;
                            p_buffer_info->mbuff_obj.p_getSize        = osal_internal_mem_getsize;
                            p_buffer_info->mbuff_obj.p_checkBuffer    = osal_internal_mem_checkbuffer;
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
                            p_buffer_info->mbuff_obj.p_print          = osal_internal_mem_debugbufferprint;
#else
                            p_buffer_info->mbuff_obj.p_print          = NULL;
#endif

                            osal_ret = osal_internal_mem_send_monitor_event(&p_mngr_control->inner_mtx,
                                                                            OSAL_MMNGR_EVENT_ALLOCATE,
                                                                            OSAL_RETURN_OK,
                                                                            p_mngr_control,
                                                                            &p_buffer_info->mbuff_obj);
                        }
                        else
                        {
                            /* Do nothing */
                        }

                        semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
                        if (pdFAIL == semaphore_ret)
                        {
                            osal_ret = OSAL_RETURN_FAIL;
                        }
                        else
                        {
                            /* Do nothing */
                        }
                    }
                }
                else
                {
                    /* Do nothing */
                }
            }
        }
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_internal_mem_alloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_find_free_buffer_info()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_find_free_buffer_info(st_osal_mem_mmngr_ptr_self_t *p_mngr_control,
                                                                    st_osal_mem_buf_ptr_self_t **pp_buffer_info)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret   = OSAL_RETURN_OK;
    size_t          loop_count = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    while ((p_mngr_control->config.max_allowed_allocations > loop_count) && (NULL == *pp_buffer_info))
    {
        if (OSAL_BUFFER_HANDLE_ID != p_mngr_control->p_buffers[loop_count].handle_id)
        {
            *pp_buffer_info = &p_mngr_control->p_buffers[loop_count];
        }
        else
        {
            /* Do nothing */
        }
        loop_count++;
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_internal_mem_find_free_buffer_object()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_alloc_lib()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_alloc_lib (st_osal_mem_mmngr_ptr_self_t * p_mngr_control,
                                                         st_osal_mem_buf_ptr_self_t * p_buffer_info,
                                                         size_t bytes,
                                                         size_t alignment)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    uintptr_t       chunk_max = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    chunk_max = (uintptr_t)p_mngr_control->p_virtual_addr + p_mngr_control->size;
    if (0U != p_mngr_control->num_buffers)
    {
        chunk_max -= OSAL_MMNGR_MEM_RESERVE_SIZE;
    }
    else
    {
        /* Do nothing */
    }

    p_buffer_info->p_chunk_va = (uint8_t *)R_MEML_Alloc(&p_mngr_control->meml_handle, p_buffer_info->chunk_size, alignment);

    if (NULL == p_buffer_info->p_chunk_va)
    {
        osal_ret = OSAL_RETURN_MEM;
    }
    else if (chunk_max < ((uintptr_t)p_buffer_info->p_chunk_va + p_buffer_info->chunk_size))
    {
        osal_ret = OSAL_RETURN_MEM;
    }
    else
    {
        p_buffer_info->chunk_pa = p_mngr_control->physical_addr + 
                                  ((uintptr_t)p_buffer_info->p_chunk_va - 
                                  (uintptr_t)p_mngr_control->p_virtual_addr);

        if ((OSAL_MMNGR_ALLOC_MODE_STACK_PROT == p_mngr_control->config.mode) ||
            (OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT == p_mngr_control->config.mode))
        {
            p_buffer_info->buffer_pa   = (((p_buffer_info->chunk_pa + OSAL_MMNGR_GUARD_PAGE_SIZE) + (alignment - 1)) & ~(alignment - 1));
            p_buffer_info->p_buffer_va = (uint8_t*)((uintptr_t)p_buffer_info->p_chunk_va +
                                                    (p_buffer_info->buffer_pa - p_buffer_info->chunk_pa));
            p_buffer_info->size        = bytes;
        }
        else
        {
            p_buffer_info->buffer_pa   = p_buffer_info->chunk_pa;
            p_buffer_info->p_buffer_va = p_buffer_info->p_chunk_va;
            p_buffer_info->size        = bytes;
        }
    }
    
    if (OSAL_RETURN_OK != osal_ret)
    {
        if (NULL != p_buffer_info->p_chunk_va)
        {
            (void)R_MEML_Dealloc(&p_mngr_control->meml_handle, (uintptr_t)p_buffer_info->p_chunk_va);
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
* End of function osal_internal_mem_alloc_lib()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_alloc_map()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_alloc_map (st_osal_mem_buf_ptr_self_t * p_buffer_info,
                                                         st_osal_mmngr_config_t * p_usr_config,
                                                         size_t bytes,
                                                         size_t alignment)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret     = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_buffer_info->p_chunk_va = pvPortMalloc(p_buffer_info->chunk_size);
    if (NULL == p_buffer_info->p_chunk_va)
    {
        osal_ret = OSAL_RETURN_MEM;
    }
    else
    {
        p_buffer_info->chunk_pa  = (uintptr_t)p_buffer_info->p_chunk_va;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if ((OSAL_MMNGR_ALLOC_MODE_STACK_PROT == p_usr_config->mode) ||
            (OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT == p_usr_config->mode))
        {
            p_buffer_info->buffer_pa = (((p_buffer_info->chunk_pa + OSAL_MMNGR_GUARD_PAGE_SIZE) + (alignment - 1)) & ~(alignment - 1));
        }
        else
        {
            p_buffer_info->buffer_pa = p_buffer_info->chunk_pa;
        }

        p_buffer_info->p_buffer_va = (uint8_t*)p_buffer_info->buffer_pa;
        p_buffer_info->size        = bytes;
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_internal_mem_alloc_map()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_alloc_fill_guard_area()
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_mem_alloc_fill_guard_area(st_osal_mem_buf_ptr_self_t * p_buffer_info)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    void            *p_guard_area = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_guard_area = (void *)((uintptr_t)p_buffer_info->p_buffer_va - OSAL_MMNGR_GUARD_PAGE_SIZE);
    (void)memset(p_guard_area, OSAL_MMNGR_GUARD_PAGE_PATTERN, OSAL_MMNGR_GUARD_PAGE_SIZE);

    p_guard_area = (void *)((uintptr_t)p_buffer_info->p_buffer_va + p_buffer_info->size);
    (void)memset(p_guard_area, OSAL_MMNGR_GUARD_PAGE_PATTERN, OSAL_MMNGR_GUARD_PAGE_SIZE);

    return;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_alloc_fill_guard_area()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_dealloc()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_dealloc(void * p_ptr_self, osal_memory_buffer_handle_t buffer_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    osal_memory_manager_handle_t mngr_handle     = NULL;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;
    st_osal_mem_buf_ptr_self_t   *p_buffer_info  = NULL;
    BaseType_t semaphore_ret;
    size_t                       use_size        = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_OPEN != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        mngr_handle = (osal_memory_manager_handle_t)p_ptr_self;
        if ((NULL == mngr_handle->p_ptr_self) || (NULL == buffer_obj->p_ptr_self))
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else if (gs_osal_memory_manager != buffer_obj->mmngr_owner_hndl)
        {
            osal_ret = OSAL_RETURN_HANDLE_NOT_SUPPORTED;
        }
        else
        {
            p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *)mngr_handle->p_ptr_self;
            p_buffer_info  = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_MEMORY_HANDLE_ID != p_mngr_control->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else if (OSAL_BUFFER_HANDLE_ID != p_buffer_info->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else if ((NULL == p_mngr_control->p_buffers) || (NULL == p_mngr_control->p_monitor))
            {
                osal_ret = OSAL_RETURN_STATE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if (((OSAL_MMNGR_ALLOC_MODE_STACK == p_mngr_control->config.mode) ||
                        (OSAL_MMNGR_ALLOC_MODE_STACK_PROT == p_mngr_control->config.mode)) &&
                        (p_mngr_control->num_buffers != p_buffer_info->buffer_id))
                    {
                        osal_ret = OSAL_RETURN_MEM;
                    }
                    else
                    {
                        osal_ret = osal_internal_mem_send_monitor_event(&p_mngr_control->inner_mtx,
                                                                        OSAL_MMNGR_EVENT_DEALLOCATE,
                                                                        OSAL_RETURN_OK,
                                                                        p_mngr_control,
                                                                        buffer_obj);
                        if (OSAL_RETURN_OK == osal_ret)
                        {
                            use_size = p_buffer_info->size;
                            osal_ret = osal_internal_mem_dealloc_proc(p_mngr_control, buffer_obj);
                            if (OSAL_RETURN_OK == osal_ret)
                            {
                                p_mngr_control->num_buffers      -= 1;
                                p_mngr_control->use_buffer_total -= use_size;
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
                    }

                    semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        if (OSAL_RETURN_OK == osal_ret)
                        {
                            osal_ret = OSAL_RETURN_FAIL;
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
                }
            }
        }
    }
    return osal_ret;

}

/***********************************************************************************************************************
* End of function osal_internal_mem_dealloc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_dealloc_proc()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_dealloc_proc(st_osal_mem_mmngr_ptr_self_t *p_mngr_control,
                                                           osal_memory_buffer_handle_t buffer_obj)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t            osal_ret       = OSAL_RETURN_OK;
    e_meml_return_t            meml_ret       = MEML_RETURN_OK;
    st_osal_mem_buf_ptr_self_t *p_buffer_info = NULL;
    BaseType_t                   semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    p_buffer_info  = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;

    semaphore_ret = xSemaphoreTake(p_buffer_info->inner_mtx, pdMS_TO_TICKS(p_buffer_info->inner_timeout));
    if (pdPASS != semaphore_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        /* Already deallocated. */
        if (NULL == p_buffer_info->p_chunk_va)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            if (OSAL_MEMORY_MANAGER_CFG_UNLIMITED != p_mngr_control->config.memory_limit)
            {
                meml_ret = R_MEML_Dealloc(&p_mngr_control->meml_handle, (uintptr_t)p_buffer_info->p_chunk_va);
                if (MEML_RETURN_OK != meml_ret)
                {
                    osal_ret = OSAL_RETURN_MEM;
                }
                else
                {
                    /* Do nothing */
                }
            }
            else
            {
                vPortFree(p_buffer_info->p_chunk_va);
            }

            if (OSAL_RETURN_OK == osal_ret)
            {
                p_buffer_info->handle_id          = 0;
                p_buffer_info->buffer_id          = 0;
                p_buffer_info->size               = 0;
                p_buffer_info->buffer_pa          = 0;
                p_buffer_info->chunk_pa           = 0;
                p_buffer_info->chunk_size         = 0; 
                p_buffer_info->p_chunk_va         = NULL;
                p_buffer_info->p_buffer_va        = NULL;

                buffer_obj->mmngr_owner_hndl = NULL;
            }
            else
            {
                /* Do nothing */
            }
        }

        semaphore_ret = xSemaphoreGive(p_buffer_info->inner_mtx);
        if (pdFAIL == semaphore_ret)
        {
            if (OSAL_RETURN_OK == osal_ret)
            {
                osal_ret = OSAL_RETURN_FAIL;
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
    }

    return osal_ret;
}

/***********************************************************************************************************************
* End of function osal_internal_mem_dealloc_proc()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_getconfig()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getconfig(const void *p_ptr_self, st_osal_mmngr_config_t *const p_config)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    osal_memory_manager_handle_t mngr_handle     = NULL;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;
    BaseType_t                   semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_OPEN != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        mngr_handle = (osal_memory_manager_handle_t)p_ptr_self;
        if (NULL == mngr_handle->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *)mngr_handle->p_ptr_self;
            if (OSAL_MEMORY_HANDLE_ID != p_mngr_control->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    *p_config = p_mngr_control->config;

                    semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}

/***********************************************************************************************************************
* End of function osal_internal_mem_getconfig()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_registermonitor()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_registermonitor(void* p_ptr_self,
                                                              e_osal_mmngr_event_t monitored_event,
                                                              fp_osal_memory_mngr_cb_t monitor_handler,
                                                              void* p_monitor_handler_user_arg)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    osal_memory_manager_handle_t mngr_handle     = NULL;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;
    size_t                       loop_count      = 0;
    BaseType_t                   semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_OPEN != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        mngr_handle = (osal_memory_manager_handle_t)p_ptr_self;
        if (NULL == mngr_handle->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *)mngr_handle->p_ptr_self;
            if (NULL == p_mngr_control->p_monitor)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else if (OSAL_MEMORY_HANDLE_ID != p_mngr_control->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if (p_mngr_control->config.max_registered_monitors_cbs == p_mngr_control->num_monitor)
                    {
                        osal_ret = OSAL_RETURN_PROHIBITED;
                    }
                    else
                    {
                        for (loop_count = 0; p_mngr_control->num_monitor > loop_count; loop_count++)
                        {
                            if ((monitored_event == p_mngr_control->p_monitor[loop_count].event_id) &&
                                (monitor_handler == p_mngr_control->p_monitor[loop_count].p_handler))
                            {
                                osal_ret = OSAL_RETURN_PAR;
                                break;
                            }
                            else
                            {
                                /* Do nothing */
                            }
                        }

                        if (OSAL_RETURN_OK == osal_ret)
                        {
                            p_mngr_control->p_monitor[p_mngr_control->num_monitor].event_id  = monitored_event;
                            p_mngr_control->p_monitor[p_mngr_control->num_monitor].p_handler = monitor_handler;
                            p_mngr_control->p_monitor[p_mngr_control->num_monitor].p_arg     = p_monitor_handler_user_arg;
                            p_mngr_control->num_monitor++;
                        }
                        else
                        {
                            /* Do nothing */
                        }
                    }

                    semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_registermonitor()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_send_monitor_event()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_send_monitor_event (SemaphoreHandle_t * p_mutex_id,
                                                                  e_osal_mmngr_event_t event_id,
                                                                  e_osal_return_t error_code,
                                                                  st_osal_mem_mmngr_ptr_self_t const * p_mngr_control,
                                                                  osal_memory_buffer_handle_t buffer_hndl)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret   = OSAL_RETURN_OK;
    size_t          num        = 0;
    size_t          loop_count = 0;
    BaseType_t      semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num = p_mngr_control->num_monitor;

    semaphore_ret = xSemaphoreGive(*p_mutex_id);
    if (pdFAIL == semaphore_ret)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }
    else
    {
        for (loop_count = 0; loop_count < num; loop_count++)
        {
            if (event_id == p_mngr_control->p_monitor[loop_count].event_id)
            {
                p_mngr_control->p_monitor[loop_count].p_handler(p_mngr_control->p_monitor[loop_count].p_arg,
                                                                event_id,
                                                                error_code,
                                                                buffer_hndl);
            }
            else
            {
                /* Do nothing */
            }
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        semaphore_ret = xSemaphoreTake(*p_mutex_id, pdMS_TO_TICKS(OSAL_MEMORY_INNER_TIMEOUT));
        if (pdPASS != semaphore_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
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
* End of function osal_internal_mem_send_monitor_event()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_debugmmngrprint()
***********************************************************************************************************************/
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
OSAL_STATIC e_osal_return_t osal_internal_mem_debugmmngrprint(const void* p_ptr_self, FILE* p_output)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret             = OSAL_RETURN_OK;
    osal_memory_manager_handle_t mngr_handle          = NULL;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control      = NULL;
    /* PRQA S 5209 1 # This is return value from standard library function. There is no side-effect in this case. */
    int                          fprintf_ret              = 0;
    size_t                       cnt                  = 0;
    char                         bytes_str[20]        = {0};
    char                         memory_limit_str[20] = {0};
    char                         alloc_mode[64]       = {0};

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_STATE_OPEN != gs_osal_memory_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        mngr_handle = (osal_memory_manager_handle_t)p_ptr_self;
        if (NULL == mngr_handle)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *)mngr_handle->p_ptr_self;
            if (OSAL_MEMORY_HANDLE_ID != p_mngr_control->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else if (NULL == p_mngr_control->p_buffers)
            {
                osal_ret = OSAL_RETURN_PAR;
            }
            else
            {
                osal_ret = osal_internal_mem_conv_unit_and_str(p_mngr_control->use_buffer_total,
                                                               bytes_str,
                                                               sizeof(bytes_str));
                if (OSAL_RETURN_OK != osal_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    /* Do nothing */
                }
            }

            if (OSAL_RETURN_OK == osal_ret)
            {
                osal_ret = osal_internal_mem_conv_unit_and_str(p_mngr_control->config.memory_limit,
                                                               memory_limit_str,
                                                               sizeof(memory_limit_str));
                if (OSAL_RETURN_OK != osal_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
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
                fprintf_ret = fprintf(p_output, "Memory Current(Limit):\t\t%s(%s)\n", bytes_str, memory_limit_str);
                if (0 > fprintf_ret)
                {
                    osal_ret = OSAL_RETURN_IO;
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
                fprintf_ret = fprintf(p_output,
                                  "Allocations Current(Limit):\t\t%u(%u)\n",
                                  p_mngr_control->num_buffers,
                                  p_mngr_control->config.max_allowed_allocations);
                if (0 > fprintf_ret)
                {
                    osal_ret = OSAL_RETURN_IO;
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
                fprintf_ret = fprintf(p_output,
                                  "Registered Monitors Current(Limit):\t\t%u(%u)\n",
                                  p_mngr_control->num_monitor,
                                  p_mngr_control->config.max_registered_monitors_cbs);
                if (0 > fprintf_ret)
                {
                    osal_ret = OSAL_RETURN_IO;
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
                switch (p_mngr_control->config.mode)
                {
                    case OSAL_MMNGR_ALLOC_MODE_STACK:
                        (void)strncpy(alloc_mode, "Stack allocator without Underflow/Overflow detection", sizeof(alloc_mode));
                        break;
                    case OSAL_MMNGR_ALLOC_MODE_STACK_PROT:
                        (void)strncpy(alloc_mode, "Stack allocator with Underflow/Overflow detection", sizeof(alloc_mode));
                        break;
                    case OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT:
                        (void)strncpy(alloc_mode, "Free list allocator with Underflow/Overflow detection", sizeof(alloc_mode));
                        break;
                    case OSAL_MMNGR_ALLOC_MODE_FREE_LIST:
                        (void)strncpy(alloc_mode, "Free list allocator without Underflow/Overflow detection", sizeof(alloc_mode));
                        break;
                    default:
                        (void)strncpy(alloc_mode, "Invalid allocator", sizeof(alloc_mode));
                        break;
                }
            }
            else
            {
                /* Do nothing */
            }

            if (OSAL_RETURN_OK == osal_ret)
            {
                fprintf_ret = fprintf(p_output, "Allocation Mode:\t\t%s\n", alloc_mode);
                if (0 > fprintf_ret)
                {
                    osal_ret = OSAL_RETURN_IO;
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
                fprintf_ret = fprintf(p_output,
                                  "Buffer Handle Value; address                                ; \
                                   address (axi domain)                         ; \
                                   sec ; d ; size              ; permissions\n");
                if (0 > fprintf_ret)
                {
                    osal_ret = OSAL_RETURN_IO;
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
                for (cnt = 0; p_mngr_control->num_buffers < cnt; cnt++)
                {
                    if (OSAL_BUFFER_HANDLE_ID == p_mngr_control->p_buffers[cnt].handle_id)
                    {
                        fprintf_ret = fprintf(p_output,
                                          "0x%016X ; 0x%016X -- 0x%016X; 0x%016X -- 0x%016X (MM) ; ns ; 0x%016X ; P:readWrite U:readWrite\n",
                                          (uintptr_t)&p_mngr_control->p_buffers[cnt],
                                          (uintptr_t) p_mngr_control->p_buffers[cnt].p_buffer_va,
                                          ((uintptr_t) (p_mngr_control->p_buffers[cnt].p_buffer_va + 
                                          p_mngr_control->p_buffers[cnt].size)),
                                          p_mngr_control->p_buffers[cnt].buffer_pa,
                                          p_mngr_control->p_buffers[cnt].buffer_pa + 
                                          p_mngr_control->p_buffers[cnt].size,
                                          p_mngr_control->p_buffers[cnt].size);
                        if (0 > fprintf_ret)
                        {
                            osal_ret = OSAL_RETURN_IO;
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
                }
            }
            else
            {
                /* Do nothing */
            }
        }
    }

    return osal_ret;

}
#endif
/***********************************************************************************************************************
* End of function osal_internal_mem_debugmmngrprint()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_conv_unit_and_str()
***********************************************************************************************************************/
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
OSAL_STATIC e_osal_return_t osal_internal_mem_conv_unit_and_str(size_t size, char * p_output, size_t array_size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    /* PRQA S 5209 1 # This is return value from standard library function. There is no side-effect in this case. */
    int             snprintf_ret  = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (OSAL_MMNGR_GB_UNIT < size)
    {
        /* PRQA S 5209 1 # There is no side-effect in this case. */
        snprintf_ret = snprintf(p_output, array_size, "%f GB", ((float)size / (float)OSAL_MMNGR_GB_UNIT));
        if (0 > snprintf_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* Do nothing */
        }
    }
    else if (OSAL_MMNGR_MB_UNIT < size)
    {
        /* PRQA S 5209 1 # There is no side-effect in this case. */
        snprintf_ret = snprintf(p_output, array_size, "%f MB", ((float)size / (float)OSAL_MMNGR_MB_UNIT));
        if (0 > snprintf_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* Do nothing */
        }
    }
    else if (OSAL_MMNGR_KB_UNIT < size)
    {
        /* PRQA S 5209 1 # There is no side-effect in this case. */
        snprintf_ret = snprintf(p_output, array_size, "%f KB", ((float)size / (float)OSAL_MMNGR_KB_UNIT));
        if (0 > snprintf_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        snprintf_ret = snprintf(p_output, array_size, "%uB", size);
        if (0 > snprintf_ret)
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* Do nothing */
        }
    }

    return osal_ret;

}
#endif
/***********************************************************************************************************************
* End of function osal_internal_mem_conv_unit_and_str()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_getcpuptr()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getcpuptr(void* p_ptr_self, void** const pp_cpu_ptr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    BaseType_t                  semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_buffer->inner_mtx, pdMS_TO_TICKS(p_buffer->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    *pp_cpu_ptr = p_buffer->p_buffer_va;

                    semaphore_ret = xSemaphoreGive(p_buffer->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;

}
/***********************************************************************************************************************
* End of function osal_internal_mem_getcpuptr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_getconstcpuptr()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getconstcpuptr(void* p_ptr_self, const void** const pp_const_cpu_ptr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    BaseType_t                  semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_buffer->inner_mtx, pdMS_TO_TICKS(p_buffer->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    *pp_const_cpu_ptr = p_buffer->p_buffer_va;

                    semaphore_ret = xSemaphoreGive(p_buffer->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_getconstcpuptr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_flush()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_flush(void* p_ptr_self, size_t offset, size_t size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    int32_t                     flush_ret    = 0;
    BaseType_t                  semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_buffer->inner_mtx, pdMS_TO_TICKS(p_buffer->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if ((p_buffer->size < offset) || (0U != (offset & 0x3FU)))
                    {
                        osal_ret = OSAL_RETURN_PAR;
                    }
                    else if ((0U == size) || (p_buffer->size < size) || (0U != (size & 0x3FU)))
                    {
                        osal_ret = OSAL_RETURN_PAR;
                    }
                    else
                    {
                        flush_ret = R_OSAL_CacheFlush((size_t)(p_buffer->p_buffer_va + offset), size, (size_t)R_OSAL_CACHE_D);
                        if (0 != flush_ret)
                        {
                            osal_ret = OSAL_RETURN_FAIL;
                        }
                        else
                        {
                            /* Do nothing */
                        }
                    }

                    semaphore_ret = xSemaphoreGive(p_buffer->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_flush()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_invalidate()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_invalidate(void* p_ptr_self, size_t offset, size_t size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    int32_t                     invalidate_ret    = 0;
    BaseType_t                  semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_buffer->inner_mtx, pdMS_TO_TICKS(p_buffer->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if ((p_buffer->size < offset) || (0U != (offset & 0x3FU)))
                    {
                        osal_ret = OSAL_RETURN_PAR;
                    }
                    else if ((0U == size) || (p_buffer->size < size) || (0U != (size & 0x3FU)))
                    {
                        osal_ret = OSAL_RETURN_PAR;
                    }
                    else
                    {
                        invalidate_ret = R_OSAL_CacheInvalidate(((uintptr_t)p_buffer->p_buffer_va + offset), size, (size_t)R_OSAL_CACHE_D);
                        if (0 != invalidate_ret)
                        {
                            osal_ret = OSAL_RETURN_FAIL;
                        }
                        else
                        {
                            /* Do nothing */
                        }
                    }

                    semaphore_ret = xSemaphoreGive(p_buffer->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_invalidate()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_gethwaddr()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_gethwaddr(const void* p_ptr_self,
                                                        osal_axi_bus_id_t axi_id,
                                                        uintptr_t* const p_hw_addr)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    BaseType_t                  semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_buffer->inner_mtx, pdMS_TO_TICKS(p_buffer->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    // Note: If "axi_id" is other than "OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY", IPMMU is not supported,
                    // so it returns the same address as if "axi_id" is "OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY".
                    if (OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY == axi_id)
                    {
                        *p_hw_addr = p_buffer->buffer_pa;
                    }
                    else
                    {
                        *p_hw_addr = p_buffer->buffer_pa;
                    }

                    semaphore_ret = xSemaphoreGive(p_buffer->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_gethwaddr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_getsize()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_getsize(const void* p_ptr_self, size_t* const p_size)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    BaseType_t                  semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_buffer->inner_mtx, pdMS_TO_TICKS(p_buffer->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    *p_size = p_buffer->size;

                    semaphore_ret = xSemaphoreGive(p_buffer->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_getsize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_checkbuffer()
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_internal_mem_checkbuffer(const void* p_ptr_self)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t              osal_ret        = OSAL_RETURN_OK;
    e_osal_return_t              check_ret       = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t  buffer_obj      = NULL;
    st_osal_mem_buf_ptr_self_t   *p_buffer       = NULL;
    st_osal_mem_mmngr_ptr_self_t *p_mngr_control = NULL;
    uint8_t                      *check_addr     = NULL;
    size_t                       loop_count      = 0;
    BaseType_t                   semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else if (NULL == buffer_obj->mmngr_owner_hndl)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else if (NULL == buffer_obj->mmngr_owner_hndl->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer       = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            p_mngr_control = (st_osal_mem_mmngr_ptr_self_t *)buffer_obj->mmngr_owner_hndl->p_ptr_self;
            if (OSAL_BUFFER_HANDLE_ID != p_buffer->handle_id)
            {
                osal_ret = OSAL_RETURN_HANDLE;
            }
            else
            {
                semaphore_ret = xSemaphoreTake(p_mngr_control->inner_mtx, pdMS_TO_TICKS(p_mngr_control->inner_timeout));
                if (pdPASS != semaphore_ret)
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    if ((OSAL_MMNGR_ALLOC_MODE_STACK_PROT != p_mngr_control->config.mode) &&
                        (OSAL_MMNGR_ALLOC_MODE_FREE_LIST_PROT != p_mngr_control->config.mode))
                    {
                        /* Do nothing */
                    }
                    else
                    {
                        check_addr = (uint8_t *)p_buffer->p_buffer_va - OSAL_MMNGR_GUARD_PAGE_SIZE;

                        for (loop_count = 0; OSAL_MMNGR_GUARD_PAGE_SIZE > loop_count; loop_count++)
                        {
                            if (OSAL_MMNGR_GUARD_PAGE_PATTERN != check_addr[loop_count])
                            {
                                osal_ret = osal_internal_mem_send_monitor_event(&p_mngr_control->inner_mtx,
                                                                                OSAL_MMNGR_EVENT_UNDERFLOW_DETECTED,
                                                                                OSAL_RETURN_OK,
                                                                                p_mngr_control,
                                                                                buffer_obj);
                                check_ret = OSAL_RETURN_MEM;
                                break;
                            }
                            else
                            {
                                /* Do nothing */
                            }
                        }

                        check_addr = (uint8_t *)p_buffer->p_buffer_va + p_buffer->size;

                        for (loop_count = 0; OSAL_MMNGR_GUARD_PAGE_SIZE > loop_count; loop_count++)
                        {
                            if (OSAL_MMNGR_GUARD_PAGE_PATTERN != check_addr[loop_count])
                            {
                                osal_ret = osal_internal_mem_send_monitor_event(&p_mngr_control->inner_mtx,
                                                                                OSAL_MMNGR_EVENT_OVERFLOW_DETECTED,
                                                                                OSAL_RETURN_OK,
                                                                                p_mngr_control,
                                                                                buffer_obj);
                                check_ret = OSAL_RETURN_MEM;
                                break;
                            }
                            else
                            {
                                /* Do nothing */
                            }
                        }
                    }

                    semaphore_ret = xSemaphoreGive(p_mngr_control->inner_mtx);
                    if (pdFAIL == semaphore_ret)
                    {
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else
                    {
                        /* Do nothing */
                    }
                }
            }

            if (OSAL_RETURN_OK != check_ret)
            {
                osal_ret = check_ret;
            }
            else
            {
                /* Do nothing */
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_internal_mem_checkbuffer()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_mem_debugbufferprint()
***********************************************************************************************************************/
#if !defined(NDEBUG) || defined(R_OSAL_ENABLE_DEBUG)
OSAL_STATIC e_osal_return_t osal_internal_mem_debugbufferprint(const void* p_ptr_self, FILE* p_output)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t             osal_ret   = OSAL_RETURN_OK;
    osal_memory_buffer_handle_t buffer_obj = NULL;
    st_osal_mem_buf_ptr_self_t  *p_buffer  = NULL;
    /* PRQA S 5209 1 # This is return value from standard library function. There is no side-effect in this case. */
    int                         fprintf_ret    = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL == p_ptr_self)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        buffer_obj = (osal_memory_buffer_handle_t)p_ptr_self;
        if (NULL == buffer_obj->p_ptr_self)
        {
            osal_ret = OSAL_RETURN_HANDLE;
        }
        else
        {
            p_buffer = (st_osal_mem_buf_ptr_self_t *)buffer_obj->p_ptr_self;
            fprintf_ret  = fprintf(p_output,
                               "0x%016X ; 0x%016X -- 0x%016X; 0x%016X -- 0x%016X (MM) ; ns ;  ; 0x%016X ; P:rw U:rw\n",
                               (uintptr_t)p_buffer,
                               (uintptr_t)p_buffer->p_buffer_va,
                               (uintptr_t)p_buffer->p_buffer_va + p_buffer->size,
                               p_buffer->buffer_pa,
                               p_buffer->buffer_pa + p_buffer->size,
                               p_buffer->size);
            if (0 > fprintf_ret)
            {
                osal_ret = OSAL_RETURN_IO;
            }
            else
            {
                /* Do nothing */
            }
                
        }
    }

    return osal_ret;
}
#endif
/***********************************************************************************************************************
* End of function osal_internal_mem_debugbufferprint()
***********************************************************************************************************************/

