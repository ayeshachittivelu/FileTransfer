/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_io.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper IO Manager for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"
#include <string.h>

/*******************************************************************************************************************//**
 * @var gs_osal_io_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_io_is_initialized = false;

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Functions_IO IO Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            Compare strings
* @param[in]        str1  To set the string for compare
* @param[in]        str2  To set the string for compare
* @param[in]        len  To set the maximum buffer length for compare
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_compare_str(const char *str1, const char *str2, size_t len);

/*******************************************************************************************************************//**
* @brief            Compare device type
* @param[in]        from_info  To set the device1 type for compare
* @param[in]        deviceType  To set the device2 type for compare
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_compare_device_type(const char * from_info, const char *  deviceType);

/*******************************************************************************************************************//**
* @brief            Compare device type
* @param[in]        from_info  To set the device1 id for compare
* @param[in]        id  To set the device2 id for compare
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_compare_device_id(const char * from_info, const char * id);

/*******************************************************************************************************************//**
* @brief            Check device io
* @param[in]        handle  To set the handle
* @param[in]        offset  To set the offset
* @param[in]        align  To set the align
* @param[in]        size  To set the size
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_io(osal_device_handle_t handle, uintptr_t offset, size_t align, size_t size);

/*******************************************************************************************************************//**
* @brief            Check interrupt using status
* @param[in]        handle  To set the handle
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrnal_check_interrupt_using(osal_device_handle_t handle);

/*******************************************************************************************************************//**
* @brief            Check io address
* @param[in]        start  To set the starting address
* @param[in]        range  To set the available range
* @param[in]        num  To set the number
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_CONF
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_io_check_address(uintptr_t start, size_t range, uint64_t num);

/*******************************************************************************************************************//**
* @brief            Search the device control corresponding to the ID.
* @param[in]        id The value of device id.
* @param[in, out]   control valid device control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_device_find_control(const char * id, st_osal_device_control_t **control);

/*******************************************************************************************************************//**
* @brief            Set device control.
* @param[in]        id The value of device id.
* @param[in, out]   device_control Setting target control.
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_BUSY
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_CONF
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_device_set_control(const char * id, st_osal_device_control_t *device_control);
/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_IoGetNumOfDevices()
* XOS1_OSAL_CD_CD_600
* [Covers: XOS1_OSAL_UD_UD_258]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_224]
* [Covers: XOS1_OSAL_UD_UD_227]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoGetNumOfDevices(const char * p_deviceType, size_t * const p_numOfDevices)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    st_osal_device_control_t *device_control = NULL;
    uint32_t count = 0;
    size_t type_count = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if ((NULL == p_deviceType) || (NULL == p_numOfDevices))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        *p_numOfDevices = 0;

        num_of_info = R_OSAL_DCFG_GetNumOfDevice();

        while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
        {
            api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &device_control);
            if ((OSAL_RETURN_OK != api_ret) || (NULL == device_control) || (NULL == device_control->dev_info))
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                api_ret = osal_compare_device_type(device_control->dev_info->type, p_deviceType);
                if (OSAL_RETURN_OK == api_ret)
                {
                    type_count++;
                }
            }

            count++;
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            *p_numOfDevices = type_count;
        }
    }

    OSAL_DEBUG("End (%u), types:%zu \n", osal_ret, type_count);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoGetNumOfDevices()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoGetDeviceList()
* XOS1_OSAL_CD_CD_603
* [Covers: XOS1_OSAL_UD_UD_259]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_116]
* [Covers: XOS1_OSAL_UD_UD_225]
* [Covers: XOS1_OSAL_UD_UD_228]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoGetDeviceList(const char* p_device_type_name, char* p_buffer,
                                       size_t buffer_size, size_t* const p_num_of_byte)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    st_osal_device_control_t *device_control = NULL;
    uint32_t count = 0;
    size_t local_num_of_byte = 0;
    size_t len = 0;
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if ((NULL == p_device_type_name) ||(NULL == p_num_of_byte))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (OSAL_DEVICE_TYPE_NAME_MAX_LENGTH < (strnlen(p_device_type_name, OSAL_DEVICE_TYPE_NAME_MAX_LENGTH) + 1))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((NULL == p_buffer) && (0 != buffer_size))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((NULL == p_buffer) && (0 == buffer_size))
    {
        osal_ret = OSAL_RETURN_OK_RQST;
    }
    else if ((NULL != p_buffer) && (0 == buffer_size))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else  /* osal_ret = OSAL_RETURN_OK */
    {
        (void)memset(p_buffer, 0x00, buffer_size);
    }

    if ((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret))
    {
        *p_num_of_byte = 0;

        num_of_info = R_OSAL_DCFG_GetNumOfDevice();

        while((count < num_of_info) && ((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret)))
        {
            api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &device_control);
            if ((OSAL_RETURN_OK != api_ret) || (NULL == device_control) || (NULL == device_control->dev_info))
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                api_ret = osal_compare_device_type(device_control->dev_info->type, p_device_type_name);
                if (OSAL_RETURN_OK == api_ret)
                {
                    len = strlen(device_control->dev_info->id);
                    if (OSAL_RETURN_OK == osal_ret)
                    {
                        if (buffer_size >= local_num_of_byte + len + 1)
                        {
                            strcat(p_buffer, device_control->dev_info->id);
                            local_num_of_byte += len;
                            p_buffer[local_num_of_byte++] = '\n';

                        }
                        else
                        {
                            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                            osal_ret = OSAL_RETURN_PAR;
                        }
                    }
                    else if (OSAL_RETURN_OK_RQST == osal_ret)
                    {
                        local_num_of_byte += len + 1;
                    }
                }
            }
            count++;
        }

        if (OSAL_RETURN_OK == osal_ret)
        {
            p_buffer[local_num_of_byte - 1] = '\0';
            *p_num_of_byte = local_num_of_byte;
        }
	else if(OSAL_RETURN_OK_RQST == osal_ret)
        {
            *p_num_of_byte = local_num_of_byte;
        }

        if (((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret)) && (0 == local_num_of_byte))
        {
            osal_ret = OSAL_RETURN_UNKNOWN_NAME;
        }
    }

    OSAL_DEBUG("End (%u), numlist:%zu \n", osal_ret, index);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoGetDeviceList()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoDeviceOpen()
* XOS1_OSAL_CD_CD_607
* [Covers: XOS1_OSAL_UD_UD_260]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_117]
* [Covers: XOS1_OSAL_UD_UD_041]
* [Covers: XOS1_OSAL_UD_UD_226]
* [Covers: XOS1_OSAL_UD_UD_261]
* [Covers: XOS1_OSAL_UD_UD_229]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoDeviceOpen(const char* p_id, osal_device_handle_t* const p_handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    st_osal_device_control_t *device_control = NULL;
    BaseType_t semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%s, %p)\n", p_id, (void *)p_handle);

    if ((NULL == p_id) || (NULL == p_handle))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        *p_handle = NULL;
        osal_ret = osal_device_find_control(p_id, &device_control);
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (NULL == device_control)
        {
            OSAL_DEBUG("OSAL_RETURN_ID\n");
            osal_ret = OSAL_RETURN_ID;
        }
        else if (NULL == device_control->inner_mtx)
        {
            OSAL_DEBUG("OSAL_RETURN_STATE\n");
            osal_ret = OSAL_RETURN_STATE;
        }
        else
        {
            ;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        semaphore_ret = xSemaphoreTake(device_control->inner_mtx, pdMS_TO_TICKS(device_control->inner_timeout));
        if (pdPASS != semaphore_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            osal_ret = osal_device_set_control(p_id, device_control);
            if (OSAL_RETURN_OK == osal_ret)
            {
                *p_handle = device_control;
            }
            semaphore_ret = xSemaphoreGive(device_control->inner_mtx);
            if (pdFAIL == semaphore_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
    }

    OSAL_DEBUG("End (%u), handle:%p \n", osal_ret, (void *)device_control);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoDeviceOpen()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoDeviceClose()
* XOS1_OSAL_CD_CD_612
* [Covers: XOS1_OSAL_UD_UD_262]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_120]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoDeviceClose(osal_device_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    e_pma_return_t pma_ret;
    BaseType_t semaphore_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p)\n", (void *)handle);

    if (false == gs_osal_io_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((false == handle->active) || (OSAL_DEVICE_HANDLE_ID != handle->handle_id) || (NULL == handle->dev_info))
    {
        OSAL_DEBUG("OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else
    {
        semaphore_ret = xSemaphoreTake(handle->inner_mtx, pdMS_TO_TICKS(handle->inner_timeout));
        if (pdFAIL == semaphore_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_BUSY\n");
            osal_ret = OSAL_RETURN_BUSY;
        }
        else
        {
            if (true == handle->pm_wait_state)
            {
                api_ret = OSAL_RETURN_BUSY;
            }
            else
            {
                api_ret = osal_interrnal_check_interrupt_using(handle);
            }

            if (OSAL_RETURN_OK != api_ret)
            {
                osal_ret = api_ret;
            }
            else
            {
                if ((NULL != handle->pma_handle) && (OSAL_PMA_ID_NONE != (uint32_t)handle->dev_info->pm_id))
                {
                    pma_ret = R_PMA_UnlockHwa(handle->pma_handle, handle->dev_info->pm_id);
                    if(R_PMA_ERR_BUS_IF_CHK_NG == pma_ret)
                    {
                       OSAL_DEBUG("OSAL_RETURN_DEV\n");
                       osal_ret = OSAL_RETURN_DEV;
                    }
                    else if (R_PMA_RESULT_OK != pma_ret)
                    {
                        OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                        osal_ret = OSAL_RETURN_FAIL;
                    }
                    else /* (R_PMA_RESULT_OK == pma_ret */
                    {
                        ;
                    }
                }

                handle->active = false;
            }
            
            semaphore_ret = xSemaphoreGive(handle->inner_mtx);
            if (pdFAIL == semaphore_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoDeviceClose()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoRead8()
* XOS1_OSAL_CD_CD_614
* [Covers: XOS1_OSAL_UD_UD_263]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_121]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoRead8(osal_device_handle_t handle, uintptr_t offset, uint8_t *const p_data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint8_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu %p)\n", (void *)handle, (uint64_t)offset, (void *)p_data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint8_t), sizeof(uint8_t));

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint8_t *)handle->dev_info->address;
        offset = offset / sizeof(uint8_t);
        *p_data = buf[offset];
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoRead8()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoRead16()
* XOS1_OSAL_CD_CD_615
* [Covers: XOS1_OSAL_UD_UD_264]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_122]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoRead16(osal_device_handle_t handle, uintptr_t offset, uint16_t *const p_data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint16_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p)\n", (void *)handle, (uint64_t)offset, (void *)p_data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint16_t), sizeof(uint16_t));

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    
    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint16_t *)handle->dev_info->address;
        offset = offset / sizeof(uint16_t);
        *p_data = buf[offset];
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoRead16()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoRead32()
* XOS1_OSAL_CD_CD_617
* [Covers: XOS1_OSAL_UD_UD_265]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_123]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoRead32(osal_device_handle_t handle, uintptr_t offset, uint32_t *const p_data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint32_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p)\n", (void *)handle, (uint64_t)offset, (void *)p_data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint32_t), sizeof(uint32_t));

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint32_t *)handle->dev_info->address;
        offset = offset / sizeof(uint32_t);
        *p_data = buf[offset];
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoRead32()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoRead64()
* XOS1_OSAL_CD_CD_619
* [Covers: XOS1_OSAL_UD_UD_266]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_124]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoRead64(osal_device_handle_t handle, uintptr_t offset, uint64_t *const p_data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint64_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p)\n", (void *)handle, (uint64_t)offset, (void *)p_data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint64_t), sizeof(uint64_t));

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint64_t *)handle->dev_info->address;
        offset = offset / sizeof(uint64_t);
        *p_data = buf[offset];
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoRead64()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoWrite8()
* XOS1_OSAL_CD_CD_621
* [Covers: XOS1_OSAL_UD_UD_267]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_125]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoWrite8(osal_device_handle_t handle, uintptr_t offset, uint8_t data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint8_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %u)\n", (void *)handle, (uint64_t)offset, data);
    osal_ret = osal_check_io(handle, offset, sizeof(uint8_t), sizeof(uint8_t));

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint8_t *)handle->dev_info->address;
        offset = offset / sizeof(uint8_t);
        buf[offset] = data;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoWrite8()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoWrite16()
* XOS1_OSAL_CD_CD_622
* [Covers: XOS1_OSAL_UD_UD_268]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_126]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoWrite16(osal_device_handle_t handle, uintptr_t offset, uint16_t data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint16_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %u)\n", (void *)handle, (uint64_t)offset, data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint16_t), sizeof(uint16_t));

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint16_t *)handle->dev_info->address;
        offset = offset / sizeof(uint16_t);
        buf[offset] = data;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoWrite16()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoWrite32()
* XOS1_OSAL_CD_CD_624
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_127]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoWrite32(osal_device_handle_t handle, uintptr_t offset, uint32_t data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint32_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %u)\n", (void *)handle, (uint64_t)offset, data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint32_t), sizeof(uint32_t));

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint32_t *)handle->dev_info->address;
        offset = offset / sizeof(uint32_t);
        buf[offset] = data;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoWrite32()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoWrite64()
* XOS1_OSAL_CD_CD_626
* [Covers: XOS1_OSAL_UD_UD_270]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_128]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoWrite64(osal_device_handle_t handle, uintptr_t offset, uint64_t data)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint64_t *buf;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lul, %lu)\n", (void *)handle, (uint64_t)offset, data);

    osal_ret = osal_check_io(handle, offset, sizeof(uint64_t), sizeof(uint64_t));

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint64_t *)handle->dev_info->address;
        offset = offset / sizeof(uint64_t);
        buf[offset] = data;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoWrite64()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockRead8()
* XOS1_OSAL_CD_CD_628
* [Covers: XOS1_OSAL_UD_UD_271]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_129]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockRead8(osal_device_handle_t handle, uintptr_t offset, uint8_t *p_data, size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint8_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lul, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint8_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint8_t *)handle->dev_info->address;
        offset = offset / sizeof(uint8_t);
        for(index = 0; index < (size/sizeof(uint8_t)); index++)
        {
            p_data[index] = buf[offset + index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockRead8()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockRead16()
* XOS1_OSAL_CD_CD_630
* [Covers: XOS1_OSAL_UD_UD_272]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_039]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockRead16(osal_device_handle_t handle, uintptr_t offset, uint16_t *p_data, size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint16_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lul, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint16_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint16_t *)handle->dev_info->address;
        offset = offset / sizeof(uint16_t);
        for(index = 0; index < (size/sizeof(uint16_t)); index++)
        {
            p_data[index] = buf[offset + index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockRead16()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockRead32()
* XOS1_OSAL_CD_CD_632
* [Covers: XOS1_OSAL_UD_UD_273]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_040]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockRead32(osal_device_handle_t handle, uintptr_t offset, uint32_t *p_data, size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint32_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint32_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint32_t *)handle->dev_info->address;
        offset = offset / sizeof(uint32_t);
        for(index = 0; index < (size/sizeof(uint32_t)); index++)
        {
            p_data[index] = buf[offset + index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockRead32()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockRead64()
* XOS1_OSAL_CD_CD_634
* [Covers: XOS1_OSAL_UD_UD_274]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_130]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockRead64(osal_device_handle_t handle, uintptr_t offset, uint64_t *p_data, size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint64_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint64_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint64_t *)handle->dev_info->address;
        offset = offset / sizeof(uint64_t);
        for(index = 0; index < (size/sizeof(uint64_t)); index++)
        {
            p_data[index] = buf[offset + index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockRead64()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockWrite8()
* XOS1_OSAL_CD_CD_636
* [Covers: XOS1_OSAL_UD_UD_275]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_131]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockWrite8(osal_device_handle_t handle, uintptr_t offset, const uint8_t *p_data,
                                     size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint8_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint8_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint8_t *)handle->dev_info->address;
        offset = offset / sizeof(uint8_t);
        for(index = 0; index < (size/sizeof(uint8_t)); index++)
        {
            buf[offset + index] = p_data[index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockWrite8()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockWrite16()
* XOS1_OSAL_CD_CD_638
* [Covers: XOS1_OSAL_UD_UD_276]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_132]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockWrite16(osal_device_handle_t handle, uintptr_t offset, const uint16_t *p_data,
                                      size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint16_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint16_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint16_t *)handle->dev_info->address;
        offset = offset / sizeof(uint16_t);
        for(index = 0; index < (size/sizeof(uint16_t)); index++)
        {
            buf[offset + index] = p_data[index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockWrite16()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockWrite32()
* XOS1_OSAL_CD_CD_640
* [Covers: XOS1_OSAL_UD_UD_277]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_133]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockWrite32(osal_device_handle_t handle, uintptr_t offset, const uint32_t *p_data,
                                      size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint32_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint32_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint32_t *)handle->dev_info->address;
        offset = offset / sizeof(uint32_t);
        for(index = 0; index < (size/sizeof(uint32_t)); index++)
        {
            buf[offset + index] = p_data[index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockWrite32()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoBlockWrite64()
* XOS1_OSAL_CD_CD_642
* [Covers: XOS1_OSAL_UD_UD_278]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_134]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoBlockWrite64(osal_device_handle_t handle, uintptr_t offset, const uint64_t *p_data,
                                      size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    volatile uint64_t *buf;
    uint32_t index;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p, %zu)\n", (void *)handle, (uint64_t)offset, (void *)p_data, size);

    osal_ret = osal_check_io(handle, offset, sizeof(uint64_t), size);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == p_data))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        buf = (uint64_t *)handle->dev_info->address;
        offset = offset / sizeof(uint64_t);
        for(index = 0; index < (size/sizeof(uint64_t)); index++)
        {
            buf[offset + index] = p_data[index];
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoBlockWrite64()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoGetDeviceAxiBusId()
***********************************************************************************************************************/
/* PRQA S 3673 1 # Implemented in accordance with the requirement. */
e_osal_return_t R_OSAL_IoGetDeviceAxiBusId(osal_device_handle_t device, osal_axi_bus_id_t* const p_axi_id)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == device)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (OSAL_DEVICE_HANDLE_ID != device->handle_id)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (device->active == false)
    {
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_axi_id)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        *p_axi_id = device->axi_bus_id;
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoGetDeviceAxiBusId()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoGetAxiBusIdFromDeviceName()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoGetAxiBusIdFromDeviceName(const char* p_device_name, osal_axi_bus_id_t* const p_axi_id)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t           osal_ret        = OSAL_RETURN_OK;
    e_osal_return_t           api_ret         = OSAL_RETURN_OK;
    int32_t                   int_ret         = 0;
    st_osal_device_control_t  *device_control = NULL;
    size_t                    dev_total_num   = 0;
    size_t                    dev_count       = 0;
    bool                      find_flg        = false;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if ((NULL == p_device_name) || (NULL == p_axi_id))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        dev_total_num = R_OSAL_DCFG_GetNumOfDevice();
        for (dev_count = 0; dev_count < dev_total_num; dev_count++)
        {
            if ((false == find_flg) && (OSAL_RETURN_OK == osal_ret))
            {
                api_ret = R_OSAL_DCFG_GetDeviceInfo((uint32_t)dev_count, &device_control);
                if ((OSAL_RETURN_OK != api_ret) || (NULL == device_control) || (NULL == device_control->dev_info))
                {
                    osal_ret = OSAL_RETURN_FAIL;
                }
                else
                {
                    int_ret = strncmp(device_control->dev_info->id, p_device_name, OSAL_DEVICE_NAME_MAX_LENGTH);
                    if (0 == int_ret)
                    {
                        find_flg = true;
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

        if ((false == find_flg) && (OSAL_RETURN_OK == osal_ret))
        {
            osal_ret = OSAL_RETURN_UNKNOWN_NAME;
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
        *p_axi_id = device_control->axi_bus_id;
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoGetAxiBusIdFromDeviceName()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoGetAxiBusNameList()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoGetAxiBusNameList(char* p_buffer, size_t buffer_size, size_t* const p_num_of_byte)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t           osal_ret         = OSAL_RETURN_OK;
    e_osal_return_t           api_ret          = OSAL_RETURN_OK;
    st_osal_axi_bus_control_t *axi_bus_control = NULL;
    size_t                    loop_count       = 0;
    size_t                    total_length     = 0;
    size_t                    local_length     = 0;
    const char                ch[2]            = "\n";
    const char                eos[2]           = "\0";

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_num_of_byte)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((NULL == p_buffer) && (0 != buffer_size))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((NULL == p_buffer) && (0 == buffer_size))
    {
        osal_ret = OSAL_RETURN_OK_RQST;
    }
    else
    {
        /* Do nothing */
    }

    if ((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetAxiBusInfo(&axi_bus_control);
        if ((OSAL_RETURN_OK == api_ret) && (NULL != axi_bus_control))
        {
            for (loop_count = 0; loop_count < axi_bus_control->num; loop_count++)
            {
                total_length += strnlen(axi_bus_control->axi_bus_list[loop_count].axi_bus_name,
                                        OSAL_AXI_BUS_NAME_MAX_LENGTH) + 1;
            }
        }
        else
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
    }
    else
    {
        /* Do nothing */
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (total_length <= buffer_size)
        {
            (void)memset(p_buffer, 0x00, buffer_size);
            for (loop_count = 0; loop_count < axi_bus_control->num; loop_count++)
            {
                (void)strncat(p_buffer,
                              axi_bus_control->axi_bus_list[loop_count].axi_bus_name,
                              OSAL_AXI_BUS_NAME_MAX_LENGTH);
                local_length += strnlen(axi_bus_control->axi_bus_list[loop_count].axi_bus_name, OSAL_AXI_BUS_NAME_MAX_LENGTH);
                if ((axi_bus_control->num - 1) != loop_count)
                {
                    p_buffer[local_length] = ch[0];
                    local_length++;
                }
                else
                {
                    p_buffer[local_length] = eos[0];
                }
            }
        }
        else
        {
            osal_ret = OSAL_RETURN_PAR;
        }
    }
    else
    {
        /* Do nothing */
    }

    if ((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret))
    {
        *p_num_of_byte = total_length;
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoGetAxiBusNameList()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_IoGetAxiBusName()
***********************************************************************************************************************/
e_osal_return_t R_OSAL_IoGetAxiBusName(osal_axi_bus_id_t axi_id, char* p_buffer, size_t buffer_size,
                                       size_t* const p_num_of_byte)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t           osal_ret         = OSAL_RETURN_OK;
    e_osal_return_t           api_ret          = OSAL_RETURN_OK;
    st_osal_axi_bus_control_t *axi_bus_control = NULL;
    size_t                    name_length      = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_num_of_byte)
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((NULL == p_buffer) && (0 != buffer_size))
    {
        osal_ret = OSAL_RETURN_PAR;
    }
    else if ((NULL == p_buffer) && (0 == buffer_size))
    {
        osal_ret = OSAL_RETURN_OK_RQST;
    }
    else
    {
        /* Do nothing */
    }

    if ((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetAxiBusInfo(&axi_bus_control);
        if ((OSAL_RETURN_OK == api_ret) && (NULL != axi_bus_control))
        {
            if (axi_bus_control->num <= axi_id)
            {
                osal_ret = OSAL_RETURN_PAR;
            }
            else
            {
                name_length = strnlen(axi_bus_control->axi_bus_list[axi_id].axi_bus_name, OSAL_AXI_BUS_NAME_MAX_LENGTH);
            }
        }
        else
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
    }
    else
    {
        /* Do nothing */
    }

    if(OSAL_RETURN_OK == osal_ret)
    {
        if ((name_length + 1) <= buffer_size)
        {
            (void)strncpy(p_buffer, axi_bus_control->axi_bus_list[axi_id].axi_bus_name, name_length);
            p_buffer[name_length] = '\0';
        }
        else
        {
            osal_ret = OSAL_RETURN_PAR;
        }
    }
    else
    {
        /* Do nothing */
    }

    if ((OSAL_RETURN_OK == osal_ret) || (OSAL_RETURN_OK_RQST == osal_ret))
    {
        *p_num_of_byte = name_length + 1;
    }
    else
    {
        /* Do nothing */
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_IoGetAxiBusName()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_io_initialize()
* XOS1_OSAL_CD_CD_644
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_149]
* [Covers: XOS1_OSAL_UD_UD_150]
* [Covers: XOS1_OSAL_UD_UD_151]
* [Covers: XOS1_OSAL_UD_UD_152]
* [Covers: osal_io_initialize]
***********************************************************************************************************************/
e_osal_return_t osal_io_initialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    int32_t int_ret;
    size_t num_of_info = 0;
    uint32_t count = 0;
    st_osal_axi_bus_control_t *axi_bus_control = NULL;
    st_osal_device_control_t *control = NULL;
    SemaphoreHandle_t mtx;
    size_t          loop_count = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (true == gs_osal_io_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_io_is_initialized = true;
        num_of_info = R_OSAL_DCFG_GetNumOfDevice();
        osal_ret = R_OSAL_DCFG_GetAxiBusInfo(&axi_bus_control);

        if (NULL == axi_bus_control->axi_bus_list)
        {
            axi_bus_control->axi_bus_list = pvPortMalloc(sizeof(st_osal_axi_bus_list_t) * num_of_info);
            if (NULL == axi_bus_control->axi_bus_list)
            {
                osal_ret = OSAL_RETURN_MEM;
            }
            else
            {
                (void)strncpy(axi_bus_control->axi_bus_list[0].axi_bus_name, "invalid", OSAL_AXI_BUS_NAME_MAX_LENGTH);
                (void)strncpy(axi_bus_control->axi_bus_list[1].axi_bus_name, "mm(ipa)", OSAL_AXI_BUS_NAME_MAX_LENGTH);
                axi_bus_control->num = 2;
            }
         }
         else
         {
             osal_ret = OSAL_RETURN_STATE;
         }
    }

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->dev_info))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            api_ret = osal_io_check_address(control->dev_info->address, control->dev_info->range, count);
            if (OSAL_RETURN_OK != api_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else
            {
                mtx = xSemaphoreCreateBinary();
                if (NULL == mtx)
                {
                    OSAL_DEBUG("OSAL_RETURN_MEM\n");
                    osal_ret = OSAL_RETURN_MEM;
                }

                if ((OSAL_RETURN_OK == osal_ret) && (NULL != control->dev_info->axi_bus_name))
                {
                    for (loop_count = 0; loop_count < axi_bus_control->num; loop_count++)
                    {
                        int_ret = strncmp(axi_bus_control->axi_bus_list[loop_count].axi_bus_name,
                                          control->dev_info->axi_bus_name,
                                          OSAL_AXI_BUS_NAME_MAX_LENGTH);
                        if (0 == int_ret)
                        {
                            control->axi_bus_id = loop_count;
                            break;
                        }
                        else
                        {
                            /* Do nothing */
                        }
                    }

                    if (loop_count == axi_bus_control->num)
                    {
                        control->axi_bus_id = axi_bus_control->num;
                        (void)strncpy(axi_bus_control->axi_bus_list[axi_bus_control->num].axi_bus_name,
                                      control->dev_info->axi_bus_name,
                                      OSAL_AXI_BUS_NAME_MAX_LENGTH);
                        axi_bus_control->num++;
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
                    /* control initialize*/
                    control->handle_id = OSAL_DEVICE_HANDLE_ID;
                    control->inner_mtx = mtx;
                    (void)xSemaphoreGive(mtx);
                    control->active = false;
                }
            }
        }

        count++;
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_io_deinitialize();
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_io_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_io_deinitialize()
* XOS1_OSAL_CD_CD_646
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_007]
* [Covers: XOS1_OSAL_UD_UD_331]
***********************************************************************************************************************/
e_osal_return_t osal_io_deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_axi_bus_control_t *axi_bus_control = NULL;
    st_osal_device_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_io_is_initialized = false;

    osal_ret = R_OSAL_DCFG_GetAxiBusInfo(&axi_bus_control);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL != axi_bus_control))
    {
        if (NULL != axi_bus_control->axi_bus_list)
        {
            vPortFree(axi_bus_control->axi_bus_list);
            axi_bus_control->axi_bus_list = NULL;
        }

        num_of_info = R_OSAL_DCFG_GetNumOfDevice();
    }

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (NULL != control->inner_mtx)
            {
                vSemaphoreDelete(control->inner_mtx);
                control->inner_mtx = NULL;
                control->axi_bus_id = OSAL_AXI_BUS_ID_INVALID;
            }

            control->handle_id = 0;
            control->active = false;
            control->pm_wait_state = false;
        }
        count++;
    }

    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_io_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_io_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_io_deinitialize_pre(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_device_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_io_is_initialized = false;

    num_of_info = R_OSAL_DCFG_GetNumOfDevice();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (true == control->active)
            {
                osal_ret = OSAL_RETURN_BUSY;
            }
        }
        count++;
    }

    if (OSAL_RETURN_BUSY == osal_ret)
    {
        gs_osal_io_is_initialized = true;
    }
    else
    {
        /* Do nothing */
    }

    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_io_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_io_set_is_init()
***********************************************************************************************************************/
void osal_io_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_io_is_initialized = true;

    return;
}
/***********************************************************************************************************************
* End of function osal_io_set_is_init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_compare_str()
* XOS1_OSAL_CD_CD_647
* [Covers: XOS1_OSAL_UD_UD_258]
* [Covers: XOS1_OSAL_UD_UD_259]
* [Covers: XOS1_OSAL_UD_UD_260]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_compare_str(const char *str1, const char *str2, size_t len)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    uint32_t count = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    while ((count < len) && ('\0' != str1[count]) && ('\0' != str2[count]) && (OSAL_RETURN_OK == osal_ret))
    {
        if (str1[count] != str2[count])
        {
            osal_ret = OSAL_RETURN_PAR;
        }
        count++;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        if (count == len)
        {
            osal_ret = OSAL_RETURN_PAR;
        }
        else
        {
            if (('\0' != str1[count]) || ('\0' != str2[count]))
            {
                osal_ret = OSAL_RETURN_PAR;
            }
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_compare_str()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_compare_device_type()
* XOS1_OSAL_CD_CD_648
* [Covers: XOS1_OSAL_UD_UD_258]
* [Covers: XOS1_OSAL_UD_UD_259]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_compare_device_type(const char * from_info, const char * deviceType)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_compare_str(from_info, deviceType, OSAL_DEVICE_TYPE_NAME_MAX_LENGTH);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_compare_device_type()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_compare_device_id()
* XOS1_OSAL_CD_CD_649
* [Covers: OS1_OSAL_UD_UD_260]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_compare_device_id(const char * from_info, const char * id)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    osal_ret = osal_compare_str(from_info, id, OSAL_DEVICE_NAME_MAX_LENGTH);

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_compare_device_id()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_check_io()
* XOS1_OSAL_CD_CD_650
* [Covers: XOS1_OSAL_UD_UD_263]
* [Covers: XOS1_OSAL_UD_UD_264]
* [Covers: XOS1_OSAL_UD_UD_265]
* [Covers: XOS1_OSAL_UD_UD_266]
* [Covers: XOS1_OSAL_UD_UD_267]
* [Covers: XOS1_OSAL_UD_UD_268]
* [Covers: XOS1_OSAL_UD_UD_315]
* [Covers: XOS1_OSAL_UD_UD_270]
* [Covers: XOS1_OSAL_UD_UD_271]
* [Covers: XOS1_OSAL_UD_UD_272]
* [Covers: XOS1_OSAL_UD_UD_273]
* [Covers: XOS1_OSAL_UD_UD_274]
* [Covers: XOS1_OSAL_UD_UD_275]
* [Covers: XOS1_OSAL_UD_UD_276]
* [Covers: XOS1_OSAL_UD_UD_277]
* [Covers: XOS1_OSAL_UD_UD_278]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_io(osal_device_handle_t handle, uintptr_t offset, size_t align, size_t size)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/

    if (false == gs_osal_io_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((false == handle->active) || (NULL == handle->dev_info) || (OSAL_DEVICE_HANDLE_ID != handle->handle_id))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == (void *)handle->dev_info->address)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (0U != (handle->dev_info->address & (align - 1U)))   /* check align */
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((handle->dev_info->range < offset) || (handle->dev_info->range < size))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else if (0U != (offset & (align - 1U)))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        if ( offset <= (offset + size))
        {
            if (handle->dev_info->range < (offset + size))
            {
                OSAL_DEBUG("OSAL_RETURN_PAR\n");
                osal_ret = OSAL_RETURN_PAR;
            }
        }
        else
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_io()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrnal_check_interrupt_using()
* XOS1_OSAL_CD_CD_651
* [Covers: XOS1_OSAL_UD_UD_262]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrnal_check_interrupt_using(osal_device_handle_t handle)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    st_osal_interrupt_control_t *control = NULL;
    uint32_t count = 0;
    size_t num_of_info;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_RCFG_GetNumOfInterruptThread();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetInterruptThreadInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (handle == control->deviceHandle)
            {
                if (true == control->irq_use)
                {
                    OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                    osal_ret = OSAL_RETURN_BUSY;
                }
            }
        }
        count++;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrnal_check_interrupt_using()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_io_check_address()
* XOS1_OSAL_CD_CD_652
* [Covers: XOS1_OSAL_UD_UD_325]
* [Covers: XOS1_OSAL_UD_UD_336]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_io_check_address(uintptr_t start, size_t range, uint64_t num)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_device_control_t *device_control = NULL;
    uintptr_t check_start;
    uintptr_t check_end;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if ((NULL != (void *)start) && (0U != range))
    {
        num_of_info = R_OSAL_DCFG_GetNumOfDevice();

        while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
        {
            if (num != count)
            {
                api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &device_control);
                if ((OSAL_RETURN_OK != api_ret) || (NULL == device_control) || (NULL == device_control->dev_info))
                {
                    OSAL_DEBUG("OSAL_RETURN_CONF\n");
                    osal_ret = OSAL_RETURN_CONF;
                }
                else
                {
                    if ((NULL != (void *)device_control->dev_info->address) && (0U != device_control->dev_info->range))
                    {
                        check_start = device_control->dev_info->address;
                        check_end = device_control->dev_info->address + device_control->dev_info->range - 1U;

                        if ( ((start <= check_start) && (check_start <= (start + range - 1U))) ||
                             ((start <= check_end) && (check_end <= (start + range - 1U))) ||
                            ((check_start <= start) && (start <= check_end)) )
                        {
                            OSAL_DEBUG("OSAL_RETURN_CONF\n");
                            osal_ret = OSAL_RETURN_CONF;
                        }
                    }
                }
            }

            count++;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_io_check_address()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_device_find_control()
* XOS1_OSAL_CD_CD_653
* [Covers: XOS1_OSAL_UD_UD_260]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_device_find_control(const char * id, st_osal_device_control_t **control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_device_control_t *device_control = NULL;
    st_osal_device_control_t *local_control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_DCFG_GetNumOfDevice();
    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret) && (NULL == local_control))
    {
        api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &device_control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == device_control) || (NULL == device_control->dev_info))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            api_ret = osal_compare_device_id(device_control->dev_info->id, id);
            if (OSAL_RETURN_OK == api_ret)
            {
                local_control = device_control;
                OSAL_DEBUG("find device id: handle(%p)\n", (void *)device_control);
            }
        }
        count++;
    }

    *control = local_control;

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_device_find_control()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_device_set_control()
* XOS1_OSAL_CD_CD_654
* [Covers: XOS1_OSAL_UD_UD_260]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_device_set_control(const char * id, st_osal_device_control_t *device_control)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count;
    st_osal_interrupt_control_t  *interrupt_control = NULL;
    uint32_t find_interrupt_count = 0;
    e_pma_return_t pma_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/

    if (false == gs_osal_io_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (true == device_control->active)
    {
        OSAL_DEBUG("OSAL_RETURN_BUSY\n");
        osal_ret = OSAL_RETURN_BUSY;
    }
    else
    {
        /* Check and set handle for interrupt */
        count = 0;
        num_of_info = R_OSAL_RCFG_GetNumOfInterruptThread();
        while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
        {
            api_ret = R_OSAL_RCFG_GetInterruptThreadInfo(count, &interrupt_control);
            if ((OSAL_RETURN_OK != api_ret) || (NULL == interrupt_control) || (NULL == interrupt_control->usr_config))
            {
                OSAL_DEBUG("OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else
            {
                api_ret = osal_compare_device_id(interrupt_control->usr_config->id, id);
                if (OSAL_RETURN_OK == api_ret)
                {
                    interrupt_control->deviceHandle = device_control;
                    interrupt_control->irq_use = false;
                    find_interrupt_count++;
                }
            }
            count++;
        }

        if (device_control->dev_info->irq_channels != find_interrupt_count)
        {
            OSAL_DEBUG("OSAL_RETURN_CONF\n");
            osal_ret = OSAL_RETURN_CONF;
        }
        else
        {
            /* Check and lock PM_ID for pma */
            if ((OSAL_PMA_ID_NONE != (uint32_t)device_control->dev_info->pm_id) &&
                (NULL != device_control->pma_handle))
            {
                pma_ret = R_PMA_GetLockHwa(device_control->pma_handle, device_control->dev_info->pm_id);
                if (R_PMA_RESULT_OK != pma_ret)
                {
                    OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                    osal_ret = OSAL_RETURN_FAIL;
                }
            }

            OSAL_DEBUG("Activate handle\n");
            device_control->active = true;
        }
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_device_set_control()
***********************************************************************************************************************/

