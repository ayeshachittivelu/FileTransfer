/*************************************************************************************************************
* OSAL Wrapper
* Copyright (c) [2020-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    r_osal_interrupt.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  OSAL wrapper Interrupt Manager for FreeRTOS
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * includes
***********************************************************************************************************************/
#include "target/freertos/r_osal_common.h"
#include "interrupts.h"
/*******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Defines_Interrupt Interrupt Manager Private macro definitions
 *
 * @{
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @def ISR_PRIORITY_LIMIT
 * ISR Driver maximum priority.
***********************************************************************************************************************/
#define ISR_PRIORITY_LIMIT (OSAL_INTERRUPT_PRIORITY_HIGHEST)

/*******************************************************************************************************************//**
 * @def ISR_SPI_OFFSET
 * Offset value for SPI number
***********************************************************************************************************************/
#define ISR_SPI_OFFSET (32U)

/** @} */

/*******************************************************************************************************************//**
 * @var gs_osal_interrupt_is_initialized
 * Initialize flag
***********************************************************************************************************************/
OSAL_STATIC bool gs_osal_interrupt_is_initialized = false;

/******************************************************************************************************************//**
 * @ingroup OSAL_WRAPPER_FREERTOS
 * @defgroup OSAL_Private_Functions_Interrupt Interrupt Manager Private function definitions
 *
 * @{
 *********************************************************************************************************************/

/*******************************************************************************************************************//**
* @brief            check interrupt handler
* @param[in]        handle  To set handle
* @param[in]        irq_channel  To set the irq channel
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_HANDLE
* @retval           OSAL_RETURN_STATE
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_interrup(osal_device_handle_t handle, uint64_t irq_channel);

/*******************************************************************************************************************//**
* @brief            Get irq interrupt info
* @param[in]        irq  To set irq number
* @param[out]       info  To set the addoress of information
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_PAR
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrnal_irqtointerruptinfo(uint64_t irq, st_osal_interrupt_control_t **info);

/*******************************************************************************************************************//**
* @brief            interrupt callback function
* @param[in]        irq  To set irq number
* @return           none
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_int_callback(void * data);

/*******************************************************************************************************************//**
* @brief            check interrupt irq number
* @param[in]        irq_number  To set the irq channel
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_CONF
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrupt_check_irq_number(uint64_t irq_number);

/*******************************************************************************************************************//**
* @brief            priority range check for FreeRTOS configurator
* @param[in]        int_priority  interrupt priority
* @return           e_osal_return_t
* @retval           OSAL_RETURN_OK
* @retval           OSAL_RETURN_FAIL
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrupt_check_int_priority(uint16_t int_priority);

/*******************************************************************************************************************//**
* @brief            Enter critical section
* @param[in,out]    pxHigherPriorityTaskWoken for taskENTER_CRITICAL_FROM_ISR
* @return           void
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_enter_critical(uint32_t *pxHigherPriorityTaskWoken);

/*******************************************************************************************************************//**
* @brief            Exit critical section
* @param[in,out]    pxHigherPriorityTaskWoken for taskEXIT_CRITICAL_FROM_ISR
* @return           void
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_exit_critical(uint32_t *pxHigherPriorityTaskWoken);

/** @} */

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptGetNumOfIrqChannels()
* XOS1_OSAL_CD_CD_500
* [Covers: XOS1_OSAL_UD_UD_256]
* [Covers: XOS1_OSAL_UD_UD_273]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_110]
* [Covers: XOS1_OSAL_UD_UD_217]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptGetNumOfIrqChannels(osal_device_handle_t handle, size_t *const p_numOfChannels)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %p)\n", (void *)handle, (void *)p_numOfChannels);

    if (false == gs_osal_interrupt_is_initialized)
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
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (NULL == p_numOfChannels)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        *p_numOfChannels = handle->dev_info->irq_channels;
    }

    OSAL_DEBUG("End (%u), numOfChannels:%zu \n", osal_ret, *p_numOfChannels);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_InterruptGetNumOfIrqChannels()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptRegisterIsr()
* XOS1_OSAL_CD_CD_502
* [Covers: XOS1_OSAL_UD_UD_185]
* [Covers: XOS1_OSAL_UD_UD_218]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_033]
* [Covers: XOS1_OSAL_UD_UD_034]
* [Covers: XOS1_OSAL_UD_UD_111]
* [Covers: XOS1_OSAL_UD_UD_112]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptRegisterIsr(osal_device_handle_t handle, uint64_t irq_channel,
                                            e_osal_interrupt_priority_t priority_value,
                                            p_osal_isr_func_t irqHandlerFunction, void *irq_arg)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    uint64_t spi_number;
    st_osal_interrupt_control_t *control = NULL;
    uint16_t isr_priority;
    uint32_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    (void)priority_value;   /* unuse argument */
    OSAL_DEBUG("Start: (%p, %lu, %d, %p, %p)\n", (void *)handle, irq_channel,
                                                 (int32_t)priority_value, (void *)irqHandlerFunction,
                                                 (void *)irq_arg);

    osal_ret = osal_check_interrup(handle, irq_channel);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == irqHandlerFunction))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        spi_number = handle->dev_info->interrupt[irq_channel].irq_number;
        api_ret = osal_interrnal_irqtointerruptinfo(spi_number, &control);
        if (OSAL_RETURN_OK != api_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else if ((NULL == control) || (NULL == control->usr_config))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            osal_internal_enter_critical(&pxHigherPriorityTaskWoken);

            if ((true == control->irq_enable) || (NULL != control->irq_func))
            {
                OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                osal_ret = OSAL_RETURN_BUSY;
            }
            else
             {
                control->irq_func = irqHandlerFunction;
                control->irq_arg = irq_arg;
                control->irq_channel = irq_channel;

                if (ISR_PRIORITY_LIMIT < ((uint16_t)(control->usr_config->int_priority)))
                {
                    isr_priority = (uint16_t)0x00;
                }
                else
                {
                    /* [7:4] of priority field bits */
                    isr_priority = (uint16_t)(((uint16_t)ISR_PRIORITY_LIMIT - (uint16_t)(control->usr_config->int_priority)) << 4);
                }
                /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
                Irq_SetupEntry((unsigned int)spi_number + ISR_SPI_OFFSET, osal_internal_int_callback, (void *)control);
                /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
                Irq_SetPriority((unsigned int)spi_number + ISR_SPI_OFFSET, isr_priority);
                control->irq_use = true;
            }

            osal_internal_exit_critical(&pxHigherPriorityTaskWoken);
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_InterruptRegisterIsr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptUnregisterIsr()
* XOS1_OSAL_CD_CD_507
* [Covers: XOS1_OSAL_UD_UD_186]
* [Covers: XOS1_OSAL_UD_UD_219]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_036]
* [Covers: XOS1_OSAL_UD_UD_037]
* [Covers: XOS1_OSAL_UD_UD_038]
* [Covers: XOS1_OSAL_UD_UD_257]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptUnregisterIsr(osal_device_handle_t handle, uint64_t irq_channel,
                                              p_osal_isr_func_t irqHandlerFunction)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    uint64_t spi_number;
    st_osal_interrupt_control_t *control = NULL;
    uint32_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %p)\n", (void *)handle, irq_channel, (void *)irqHandlerFunction);

    osal_ret = osal_check_interrup(handle, irq_channel);

    if ((OSAL_RETURN_OK == osal_ret) && (NULL == irqHandlerFunction))
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        spi_number = handle->dev_info->interrupt[irq_channel].irq_number;
        api_ret = osal_interrnal_irqtointerruptinfo(spi_number, &control);
        if (OSAL_RETURN_OK != api_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else if (NULL == control)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            osal_internal_enter_critical(&pxHigherPriorityTaskWoken);

            if (irqHandlerFunction !=  control->irq_func)
            {
                OSAL_DEBUG("OSAL_RETURN_PAR\n");
                osal_ret = OSAL_RETURN_PAR;
            }
            else if (true == control->irq_enable)
            {
                OSAL_DEBUG("OSAL_RETURN_BUSY\n");
                osal_ret = OSAL_RETURN_BUSY;
            }
            else
            {
                /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
                Irq_RemoveEntry((unsigned int)spi_number + ISR_SPI_OFFSET);
                control->irq_func = NULL;
                control->irq_use = false;
            }

            osal_internal_exit_critical(&pxHigherPriorityTaskWoken);
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_InterruptUnregisterIsr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptEnableIsr()
* XOS1_OSAL_CD_CD_512
* [Covers: XOS1_OSAL_UD_UD_187]
* [Covers: XOS1_OSAL_UD_UD_220]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_029]
* [Covers: XOS1_OSAL_UD_UD_031]
* [Covers: XOS1_OSAL_UD_UD_032]
* [Covers: XOS1_OSAL_UD_UD_306]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptEnableIsr(osal_device_handle_t handle, uint64_t irq_channel)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    uint64_t spi_number;
    st_osal_interrupt_control_t *control = NULL;
    uint32_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu)\n", (void *)handle, irq_channel);

    osal_ret = osal_check_interrup(handle, irq_channel);

    if (OSAL_RETURN_OK == osal_ret)
    {
        spi_number = handle->dev_info->interrupt[irq_channel].irq_number;
        api_ret = osal_interrnal_irqtointerruptinfo(spi_number, &control);
        if (OSAL_RETURN_OK != api_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else if (NULL == control)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            ;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_internal_enter_critical(&pxHigherPriorityTaskWoken);

        if (NULL == control->irq_func)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
            Irq_Enable((unsigned int)spi_number + ISR_SPI_OFFSET);
            control->irq_enable = true;
        }

        osal_internal_exit_critical(&pxHigherPriorityTaskWoken);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_InterruptEnableIsr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptDisableIsr()
* XOS1_OSAL_CD_CD_517
* [Covers: XOS1_OSAL_UD_UD_188]
* [Covers: XOS1_OSAL_UD_UD_221]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_027]
* [Covers: XOS1_OSAL_UD_UD_028]
* [Covers: XOS1_OSAL_UD_UD_113]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptDisableIsr(osal_device_handle_t handle, uint64_t irq_channel)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    uint64_t spi_number;
    st_osal_interrupt_control_t *control = NULL;
    uint32_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu)\n", (void *)handle, irq_channel);

    osal_ret = osal_check_interrup(handle, irq_channel);

    if (OSAL_RETURN_OK == osal_ret)
    {
        spi_number = handle->dev_info->interrupt[irq_channel].irq_number;
        api_ret = osal_interrnal_irqtointerruptinfo(spi_number, &control);
        if (OSAL_RETURN_OK != api_ret)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else if (NULL == control)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            ;
        }
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        osal_internal_enter_critical(&pxHigherPriorityTaskWoken);

        if (NULL == control->irq_func)
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
            Irq_Disable((unsigned int)spi_number + ISR_SPI_OFFSET);
            control->irq_enable = false;
        }

        osal_internal_exit_critical(&pxHigherPriorityTaskWoken);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}

/***********************************************************************************************************************
* End of function R_OSAL_InterruptDisableIsr()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptIsISRContext()
* XOS1_OSAL_CD_CD_521
* [Covers: XOS1_OSAL_UD_UD_189]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_114]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptIsISRContext(bool *const p_is_isr)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p)\n", (void *)p_is_isr);
 
    if (false == gs_osal_interrupt_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == p_is_isr)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        R_OSAL_Internal_GetISRContext(p_is_isr);
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_InterruptIsISRContext()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function R_OSAL_InterruptSetIsrPriority()
* XOS1_OSAL_CD_CD_523
* [Covers: XOS1_OSAL_UD_UD_190]
* [Covers: XOS1_OSAL_UD_UD_223]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_035]
* [Covers: XOS1_OSAL_UD_UD_115]
***********************************************************************************************************************/
e_osal_return_t R_OSAL_InterruptSetIsrPriority(osal_device_handle_t handle, uint64_t irq_channel,
                                               e_osal_interrupt_priority_t priority_value)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;
    e_osal_return_t api_ret;
    uint64_t spi_number;
    st_osal_interrupt_control_t *control = NULL;
    uint16_t isr_priority;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start: (%p, %lu, %d)\n", (void *)handle, irq_channel, (int32_t)priority_value);

    osal_ret = osal_check_interrup(handle, irq_channel);

    if (OSAL_RETURN_OK == osal_ret)
    {
        if ((OSAL_INTERRUPT_PRIORITY_HIGHEST < priority_value) ||
            (OSAL_INTERRUPT_PRIORITY_LOWEST > priority_value))
        {
            OSAL_DEBUG("OSAL_RETURN_PAR\n");
            osal_ret = OSAL_RETURN_PAR;
        }
        else
        {
            spi_number = handle->dev_info->interrupt[irq_channel].irq_number;
            api_ret = osal_interrnal_irqtointerruptinfo(spi_number, &control);
            if (OSAL_RETURN_OK != api_ret)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else if (NULL == control)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                taskENTER_CRITICAL()
                isr_priority = (uint16_t)(((uint16_t)ISR_PRIORITY_LIMIT - (uint16_t)priority_value) << 4);
                /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
                Irq_SetPriority((unsigned int)spi_number + ISR_SPI_OFFSET, isr_priority);
                taskEXIT_CRITICAL()
            }
        }
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function R_OSAL_InterruptSetIsrPriority()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrupt_initialize()
* XOS1_OSAL_CD_CD_526
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_017]
* [Covers: XOS1_OSAL_UD_UD_019]
* [Covers: XOS1_OSAL_UD_UD_155]
* [Covers: XOS1_OSAL_UD_UD_200]
* [Covers: XOS1_OSAL_UD_UD_326]
***********************************************************************************************************************/
e_osal_return_t osal_interrupt_initialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info = 0;
    uint32_t count = 0;
    st_osal_interrupt_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (true == gs_osal_interrupt_is_initialized)
    {
        osal_ret = OSAL_RETURN_STATE;
    }
    else
    {
        gs_osal_interrupt_is_initialized = true;
        num_of_info = R_OSAL_RCFG_GetNumOfInterruptThread();
    }

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetInterruptThreadInfo(count, &control);

        if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->usr_config))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            /* check interrupt configuration */
            if (((int32_t)OSAL_INTERRUPT_PRIORITY_HIGHEST < (int32_t)control->usr_config->int_priority)  ||
                ((int32_t)OSAL_INTERRUPT_PRIORITY_LOWEST > (int32_t)control->usr_config->int_priority))
            {
                OSAL_DEBUG("OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else if(OSAL_RETURN_OK != osal_interrupt_check_int_priority((uint16_t)control->usr_config->int_priority))
            {
                OSAL_DEBUG("OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else if(OSAL_RETURN_OK != osal_interrupt_check_irq_number(control->usr_config->irq_number))
            {
                OSAL_DEBUG("OSAL_RETURN_CONF\n");
                osal_ret = OSAL_RETURN_CONF;
            }
            else /* Configuration check PASS */
            {
                control->active = false;
                control->handle_id = OSAL_INTERRUPT_HANDLE_ID;
                control->irq_enable = false;
                control->irq_use = false;
            }
        }
        
        count++;
    }

    if (OSAL_RETURN_OK != osal_ret)
    {
        (void)osal_interrupt_deinitialize();
    }
    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrupt_initialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrupt_deinitialize()
* XOS1_OSAL_CD_CD_531
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_009]
* [Covers: XOS1_OSAL_UD_UD_333]
***********************************************************************************************************************/
e_osal_return_t osal_interrupt_deinitialize(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_interrupt_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_interrupt_is_initialized = false;

    num_of_info = R_OSAL_RCFG_GetNumOfInterruptThread();
    OSAL_DEBUG("Get num_of_info: %zu\n", num_of_info);

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetInterruptThreadInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control) ||
            (NULL == control->usr_config))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            taskENTER_CRITICAL();
            if (true == control->irq_enable)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                if (NULL != control->irq_func)
                {
                    /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
                    Irq_RemoveEntry((unsigned int)control->usr_config->irq_number + ISR_SPI_OFFSET);
                }

                if (OSAL_RETURN_OK == osal_ret)
                {
                    control->active = false;
                    control->irq_use = false;
                }
            }
            taskEXIT_CRITICAL();
        }
        count++;
    }

    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrupt_deinitialize()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrupt_deinitialize_pre()
***********************************************************************************************************************/
e_osal_return_t osal_interrupt_deinitialize_pre(void)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    st_osal_interrupt_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    gs_osal_interrupt_is_initialized = false;

    num_of_info = R_OSAL_RCFG_GetNumOfInterruptThread();
    OSAL_DEBUG("Get num_of_info: %zu\n", num_of_info);

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret))
    {
        api_ret = R_OSAL_RCFG_GetInterruptThreadInfo(count, &control);
        if ((OSAL_RETURN_PAR == api_ret) || (NULL == control) ||
            (NULL == control->usr_config))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            taskENTER_CRITICAL();
            if (true == control->irq_enable)
            {
                OSAL_DEBUG("OSAL_RETURN_FAIL\n");
                osal_ret = OSAL_RETURN_FAIL;
            }
            else
            {
                if (true == control->irq_use)
                {
                    osal_ret = OSAL_RETURN_BUSY;
                }
            }
            taskEXIT_CRITICAL();
        }
        count++;
    }

    if (OSAL_RETURN_BUSY == osal_ret)
    {
        gs_osal_interrupt_is_initialized = true;
    }
    else
    {
        /* Do nothing */
    }

    OSAL_DEBUG("End\n");
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrupt_deinitialize_pre()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrupt_set_is_init()
***********************************************************************************************************************/
void osal_interrupt_set_is_init(void)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    gs_osal_interrupt_is_initialized = true;

    return;
}
/***********************************************************************************************************************
* End of function osal_interrupt_set_is_init()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrnal_irqtointerruptinfo()
* XOS1_OSAL_CD_CD_535
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_185]
* [Covers: XOS1_OSAL_UD_UD_186]
* [Covers: XOS1_OSAL_UD_UD_187]
* [Covers: XOS1_OSAL_UD_UD_188]
* [Covers: XOS1_OSAL_UD_UD_190]
* [Covers: XOS1_OSAL_UD_UD_317]
* [Covers: XOS1_OSAL_UD_UD_318]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrnal_irqtointerruptinfo (uint64_t irq, st_osal_interrupt_control_t **info)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    size_t num_of_info;
    e_osal_return_t api_ret;
    st_osal_interrupt_control_t *control = NULL;
    st_osal_interrupt_control_t *local_control = NULL;
    uint32_t count = 0;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    OSAL_DEBUG("Start\n");

    if (NULL == info)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        *info = NULL;
    }

    num_of_info = R_OSAL_RCFG_GetNumOfInterruptThread();

    while((count < num_of_info) && (OSAL_RETURN_OK == osal_ret) && (NULL == local_control))
    {
        api_ret = R_OSAL_RCFG_GetInterruptThreadInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->usr_config))
        {
            OSAL_DEBUG("OSAL_RETURN_FAIL\n");
            osal_ret = OSAL_RETURN_FAIL;
        }
        else
        {
            if (irq == control->usr_config->irq_number)
            {
                local_control = control;
            }
        }

        count++;
    }

    if (OSAL_RETURN_OK == osal_ret)
    {
        *info = local_control;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrnal_irqtointerruptinfo()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_int_callback()
* XOS1_OSAL_CD_CD_532
* [Covers: XOS1_OSAL_UD_UD_185]
* [Covers: XOS1_OSAL_UD_UD_292]
* [Covers: XOS1_OSAL_UD_UD_293]
* [Covers: XOS1_OSAL_UD_UD_317]
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_int_callback(void * data)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    st_osal_interrupt_control_t *control = (st_osal_interrupt_control_t *)data;
    uint32_t pxHigherPriorityTaskWoken = pdFAIL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (NULL != control)
    {
        osal_internal_enter_critical(&pxHigherPriorityTaskWoken);
        /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
        Irq_Disable((unsigned int)control->usr_config->irq_number + ISR_SPI_OFFSET);
        osal_internal_exit_critical(&pxHigherPriorityTaskWoken);

        control->irq_func(control->deviceHandle, control->irq_channel, control->irq_arg);

        osal_internal_enter_critical(&pxHigherPriorityTaskWoken);
        if (true == control->irq_enable)
        {
            /* PRQA S 4461 1 # The 1st parameter must be (unsigned int) and there is no side side-effect in this case. */
            Irq_Enable((unsigned int)control->usr_config->irq_number + ISR_SPI_OFFSET);
        }
        osal_internal_exit_critical(&pxHigherPriorityTaskWoken);
    }
}

/***********************************************************************************************************************
* End of function osal_internal_int_callback()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_check_interrup()
* XOS1_OSAL_CD_CD_533
* [Covers: XOS1_OSAL_UD_UD_185]
* [Covers: XOS1_OSAL_UD_UD_186]
* [Covers: XOS1_OSAL_UD_UD_187]
* [Covers: XOS1_OSAL_UD_UD_188]
* [Covers: XOS1_OSAL_UD_UD_190]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_check_interrup(osal_device_handle_t handle, uint64_t irq_channel)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (false == gs_osal_interrupt_is_initialized)
    {
        OSAL_DEBUG("OSAL_RETURN_STATE\n");
        osal_ret = OSAL_RETURN_STATE;
    }
    else if (NULL == handle)
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if ((false == handle->active) || (false == handle->dev_info->interrupt) ||
        (OSAL_DEVICE_HANDLE_ID != handle->handle_id))
    {
        OSAL_DEBUG("OSAL_RETURN_HANDLE\n");
        osal_ret = OSAL_RETURN_HANDLE;
    }
    else if (irq_channel >= handle->dev_info->irq_channels)
    {
        OSAL_DEBUG("OSAL_RETURN_PAR\n");
        osal_ret = OSAL_RETURN_PAR;
    }
    else
    {
        osal_ret = OSAL_RETURN_OK;
    }

    OSAL_DEBUG("End (%u)\n", osal_ret);
    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_check_interrup()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrupt_check_irq_number()
* XOS1_OSAL_CD_CD_536
* [Covers: XOS1_OSAL_UD_UD_326]
* [Covers: XOS1_OSAL_UD_UD_335]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrupt_check_irq_number(uint64_t irq_number)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_CONF;
    e_osal_return_t api_ret;
    size_t num_of_info;
    uint32_t count = 0;
    uint32_t interupt_count;
    st_osal_device_control_t *control = NULL;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    num_of_info = R_OSAL_DCFG_GetNumOfDevice();

    while((count < num_of_info) && (OSAL_RETURN_OK != osal_ret))
    {
        api_ret = R_OSAL_DCFG_GetDeviceInfo(count, &control);
        if ((OSAL_RETURN_OK != api_ret) || (NULL == control) || (NULL == control->dev_info))
        {
            osal_ret = OSAL_RETURN_FAIL;
        }
        else if (NULL != control->dev_info->interrupt)
        {
            for (interupt_count = 0; interupt_count < control->dev_info->irq_channels; interupt_count++)
            {
                if (irq_number == control->dev_info->interrupt[interupt_count].irq_number)
                {
                    osal_ret = OSAL_RETURN_OK;
                }
            }
        }
        else /* next control check */
        {
            ;
        }
        count++;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrupt_check_irq_number()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_interrupt_check_irq_number()
* XOS1_OSAL_CD_CD_537
* [Covers: XOS1_OSAL_UD_UD_326]
***********************************************************************************************************************/
OSAL_STATIC e_osal_return_t osal_interrupt_check_int_priority(uint16_t int_priority)
{

    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    e_osal_return_t osal_ret = OSAL_RETURN_OK;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    if (ISR_PRIORITY_LIMIT < int_priority)
    {
        osal_ret = OSAL_RETURN_FAIL;
    }

    return osal_ret;
}
/***********************************************************************************************************************
* End of function osal_interrupt_check_irq_number()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_enter_critical()
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_enter_critical(uint32_t *pxHigherPriorityTaskWoken)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    bool is_isr = false;


    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_GetISRContext(&is_isr);

    if (true == is_isr)
    {
        *pxHigherPriorityTaskWoken = taskENTER_CRITICAL_FROM_ISR();
    }
    else
    {
        taskENTER_CRITICAL();
    }
}
/***********************************************************************************************************************
* End of function osal_internal_enter_critical()
***********************************************************************************************************************/

/***********************************************************************************************************************
* Start of function osal_internal_exit_critical()
***********************************************************************************************************************/
OSAL_STATIC void osal_internal_exit_critical(uint32_t *pxHigherPriorityTaskWoken)
{
    /*------------------------------------------------------------------------------------------------------------------
    Local variables
    ------------------------------------------------------------------------------------------------------------------*/
    bool is_isr = false;

    /*------------------------------------------------------------------------------------------------------------------
    Function body
    ------------------------------------------------------------------------------------------------------------------*/
    R_OSAL_Internal_GetISRContext(&is_isr);

    if (true == is_isr)
    {
        taskEXIT_CRITICAL_FROM_ISR(*pxHigherPriorityTaskWoken);
        portYIELD_FROM_ISR((BaseType_t) *pxHigherPriorityTaskWoken);
    }
    else
    {
        taskEXIT_CRITICAL();
    }
}
/***********************************************************************************************************************
* End of function osal_internal_exit_critical()
***********************************************************************************************************************/

