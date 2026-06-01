/*************************************************************************************************************
* OSAL Sample App
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License.
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/***********************************************************************************************************************
* File Name :    osal_sample_device_main.c
* Version :      1.0.0
* Product Name : OSAL
* Device(s) :    R-Car
* Description :  Sample application to use OSAL IO/Interrupt/Power API
***********************************************************************************************************************/

#include "osal_sample_device_common.h"

int io_main_task();
int io_extension_main_task();
int int_main_task();
int pm_main_task();
void int_cb_func(osal_device_handle_t dev_handle, uint64_t irq_channel, void *irq_arg);
int device_open(char *device_type, int device_channel, osal_device_handle_t *device_handle);

/**
 * @defgroup osal_sample_device OSAL Application: Sample For OSAL IO Operation
 * @{
 */
/**
 * @brief The main function of OSAL Sample Device application
 * - Call R_OSAL_Initialize and call the device_task function. Then call R_OSAL_Deinitialize
 * - The device_task is divided into 4 main tasks as below:
 *      -# @link io_main_task @endlink describes the function of IO control
 *      -# @link io_extension_main_task @endlink describes the function of IO Extension APIs
 *      -# @link int_main_task @endlink describes the function of Interrupt control
 *      -# @link pm_main_task @endlink describes the function of Power control
 * 
 * @return 0 on success
 * @return -1 on failure
 */
void test_main(void* arg)
{
    (void)arg;
    printf("================== osal_sample_device start ==================\n");
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    bool success = true;

    OSAL_SAMPLE_INFO("initialize osal\n");
    osal_ret = R_OSAL_Initialize();
    if(OSAL_RETURN_OK != osal_ret)
    {
        OSAL_SAMPLE_ERR("R_OSAL_Initialize was failed, osal_ret = %d\n", osal_ret);
        return(-1);
    }

    OSAL_SAMPLE_INFO("execute io_main_task\n");
    app_ret = io_main_task();
    if(0 != app_ret)
    {
        success = false;
        OSAL_SAMPLE_ERR("io_main_task operation was failed\n");
    }
    else
    {
        OSAL_SAMPLE_INFO("io_main_task operation is completed\n");
    }

    OSAL_SAMPLE_INFO("execute io_extension_main_task\n");
    app_ret = io_extension_main_task();
    if(0 != app_ret)
    {
        success = false;
        OSAL_SAMPLE_ERR("io_extension_main_task operation was failed\n");
    }
    else
    {
        OSAL_SAMPLE_INFO("io_extension_main_task operation is completed\n");
    }

    OSAL_SAMPLE_INFO("execute int_main_task\n");
    app_ret = int_main_task();
    if(0 != app_ret)
    {
        success = false;
        OSAL_SAMPLE_ERR("int_main_task operation was failed\n");
    }
    else
    {
        OSAL_SAMPLE_INFO("int_main_task operation is completed\n");
    }

    OSAL_SAMPLE_INFO("execute pm_main_task\n");
    app_ret = pm_main_task();
    if(0 != app_ret)
    {
        success = false;
        OSAL_SAMPLE_ERR("pm_main_task operation was failed\n");
    }
    else
    {
        OSAL_SAMPLE_INFO("pm_main_task operation is completed\n");
    }
    OSAL_SAMPLE_INFO("de-initialize osal\n");
    osal_ret = R_OSAL_Deinitialize();
    if(OSAL_RETURN_OK != osal_ret)
    {
        success = false;
        OSAL_SAMPLE_ERR("R_OSAL_Deinitialize was failed, osal_ret = %d\n", osal_ret);
    }

    if(true == success)
    {
        OSAL_SAMPLE_INFO("osal_sample_device run successfully\n");
    }
    else
    {
        OSAL_SAMPLE_ERR("osal_sample_device was failed\n");
    }

    printf("================== osal_sample_device finish ==================\n");
}

int main(void)
{
    /* Start OS and initial thread */
    /* Note: parameter unused by AutoSAR, see TASK(maintask) */
    R_OSAL_StartOS(test_main);

    return(-1);
}

/**
 * @defgroup io_main_task OSAL IO APIs Usage
 * @{
 */
/**
 * @brief IO main task:<br>
 * (OSAL API UM reference: Chapter 2.7.1: Device open and Chapter 2.7.2: register access)
 * - Open "ims_00" device by using @link device_open @endlink
 * - Change power required state of "ims_00" to OSAL_PM_REQUIRED_STATE_REQUIRED by using R_OSAL_PmSetRequiredState.<br>
 * Now, power and clock of "ims_00" are ON state.
 * - Read/write singal block data with register offset 0x00C0 by using R_OSAL_IoRead32/R_OSAL_IoWrite32 with steps as below:
 *      -# Write data to register by using R_OSAL_IoWrite32
 *      -# Read-back data from register by using R_OSAL_IoRead32
 *      -# the read-back data must be equal to write data.
 * - Do the similar thing for multi-block data with register offset 0x0034 by using R_OSAL_IoBlockRead32/R_OSAL_IoBlockWrite32.
 * - Change power required state to OSAL_PM_REQUIRED_STATE_RELEASED by using R_OSAL_PmSetRequiredState.
 * - Close "ims_00" by using R_OSAL_IoDeviceClose.
 * 
 * @return 0 on success
 * @return -1 on failure
 * @note Please select the Read/Write API that matches with the the characteristics of the platform architecture and OS.
 */
int io_main_task()
{
     /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    osal_device_handle_t ims_handle = OSAL_DEVICE_HANDLE_INVALID;
    uintptr_t offset;
    uint32_t read_data32;
    uint32_t write_data32 = 0x000000FF;
    size_t size = 16;
    size_t count = 0;
    uint32_t read_block_data32[4];
    uint32_t write_block_data32[4] = {0x00000000, 0x00000000, 0x00000040, 0x00000100};

    printf("\n================== io_main_task start ==================\n");
    /* open "ims_00" */
    app_ret = device_open(ims, 0, &ims_handle);
    if(0 != app_ret)
    {
        OSAL_SAMPLE_ERR("ims_00 opening was failed \n");
    }
    else
    {
        OSAL_SAMPLE_INFO("set power required state to OSAL_PM_REQUIRED_STATE_REQUIRED\n");
        offset = 0x00C0;
        osal_ret = R_OSAL_PmSetRequiredState(ims_handle, OSAL_PM_REQUIRED_STATE_REQUIRED, true);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Set required state failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            OSAL_SAMPLE_INFO("write data is 0x000000FF to ims_00 with offset is 0x00C0 by using R_OSAL_IoWrite32\n");
            osal_ret = R_OSAL_IoWrite32(ims_handle, offset, write_data32);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("R_OSAL_IoWrite32 failed, osal_ret = %d\n", osal_ret);
            }
            else
            {
                OSAL_SAMPLE_INFO("read data from ims_00 with offset is 0x00C0 by using R_OSAL_IoRead32. The read data will be 0x000000FF\n");
                osal_ret = R_OSAL_IoRead32(ims_handle, offset, &read_data32);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("R_OSAL_IoRead32 failed, osal_ret = %d\n", osal_ret);
                }
                else if(read_data32 != write_data32)
                {
                    OSAL_SAMPLE_ERR("read data = %ld is not equal to write data = %ld\n", read_data32, write_data32);
                }
                else
                {
                    /* read/write is completed */
                }
            }

            if(0 == app_ret)
            {
                OSAL_SAMPLE_INFO("write 16-byte data {0x00000000, 0x00000000, 0x00000040, 0x00000100} to ims_00 with offset is 0x0034 by using R_OSAL_IoBlockWrite32\n");
                offset = 0x0034;
                osal_ret = R_OSAL_IoBlockWrite32(ims_handle, offset, write_block_data32, size);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("R_OSAL_IoBlockWrite32 failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("read 16-byte data from ims_00 with offset is 0x0034 by using R_OSAL_IoBlockRead32\n");
                    OSAL_SAMPLE_INFO("The read data will be {0x00000000, 0x00000000, 0x00000040, 0x00000100}\n");
                    osal_ret = R_OSAL_IoBlockRead32(ims_handle, offset, read_block_data32, size);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("R_OSAL_IoBlockRead32 failed, osal_ret = %d\n", osal_ret);
                    }
                    else
                    {
                        for(count = 0; count < size/sizeof(write_block_data32[0]); count++)
                        {
                            if(write_block_data32[count] != read_block_data32[count])
                            {
                                OSAL_SAMPLE_ERR("read data = %ld is not equal to write data = %ld\n", read_block_data32[count], write_block_data32[count]);
                                break;
                            }
                        }
                    }
                }
            }

            if(0 == app_ret)
            {
                OSAL_SAMPLE_INFO("set power required state to OSAL_PM_REQUIRED_STATE_RELEASED\n");
                osal_ret = R_OSAL_PmSetRequiredState(ims_handle, OSAL_PM_REQUIRED_STATE_RELEASED, true);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Set release state failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("close device\n");
                    osal_ret = R_OSAL_IoDeviceClose(ims_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Device closing failed, osal_ret = %d\n", osal_ret);
                    }
                }
            }
        }
    }

    printf("================== io_main_task finish ==================\n\n");
    return app_ret;
}
/**@} IO*/

/**
 * @brief Device opening function
 * - Get the number of device which related to "ims" device type by using R_OSAL_IoGetNumOfDevices
 * - Get the required size of the buffer which use to store list of "ims" devices by using R_OSAL_IoGetDeviceList
 * - Get the list of "ims" devices by using R_OSAL_IoGetDeviceList
 * - Convert the device list to an array of device ID type
 * - Open "ims_00" which is the first element of the device ID array above by using R_OSAL_IoDeviceOpen
 * 
 * @param[in] device_type Device type
 * @param[in] device_channel Device channel
 * @param[in, out] device_handle To set the address of osal_device_handle_t
 * @return 0 on success
 * @return -1 on failure
 */
int device_open(char *device_type, int device_channel, osal_device_handle_t *device_handle)
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    size_t numOfDevice = 0;
    size_t requiredSize = 0;
    size_t numOfByte = 0;
    size_t i = 0;
    size_t j = 0;
    char *buffer;
    osal_device_handle_t local_handle = OSAL_DEVICE_HANDLE_INVALID;

    OSAL_SAMPLE_INFO("Get the number of devices\n");
    osal_ret = R_OSAL_IoGetNumOfDevices(device_type, &numOfDevice);
    if(OSAL_RETURN_OK != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("Get number of device failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        osal_ret = R_OSAL_IoGetDeviceList(device_type, NULL, 0, &requiredSize);
        if(OSAL_RETURN_OK_RQST != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Get required size of device list buffer failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            buffer = (char *)malloc(requiredSize);
            if(NULL == buffer)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("Malloc failed\n");
            }
            else
            {
                OSAL_SAMPLE_INFO("Get the list of device identifier\n");
                osal_ret = R_OSAL_IoGetDeviceList(device_type, buffer, requiredSize, &numOfByte);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Get device list buffer failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    char *deviceList[numOfDevice + 1];
                    deviceList[0] = &buffer[0];
                    for (i = 0; i < numOfByte; i++)
                    {
                        if (buffer[i] == '\n')
                        {
                            j++;
                            buffer[i] = '\0';
                            deviceList[j] = &buffer[i+1];
                        }
                    }

                    if(j > numOfDevice)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Device parsing error \n");
                    }
                    else
                    {
                        OSAL_SAMPLE_INFO("open device\n");
                        osal_ret = R_OSAL_IoDeviceOpen(deviceList[device_channel], &local_handle);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Device opening was failed, osal_ret = %d\n", osal_ret);
                        }
                        else
                        {
                            *device_handle = local_handle;
                        }
                    }
                }

                free(buffer);
            }
        }
    }

    if(0 != app_ret)
    {
        *device_handle = OSAL_DEVICE_HANDLE_INVALID;
    }

    return app_ret;
}

/**
 * @defgroup io_extension_main_task OSAL IO Extension APIs Usage
 * @{
 */
/**
 * @brief IO Extension APIs task:
 * - Get the AXI bus name list with the steps as below:
 *      -# Get the required size of buffer which use to store the AXI bus name list by using R_OSAL_IoGetAxiBusNameList
 *      -# Allocate a buffer with required size above.
 *      -# Get the AXI bus name list by using R_OSAL_IoGetAxiBusNameList.<br>
 *        The AXI bus name list will be stored in the allocated buffer.
 * - Convert the AXI bus name list to an array of AXI bus name type<br>
 * The array element which related to index OSAL_AXI_BUS_ID_INVALID need to be equal with "invalid".<br>
 * The array element which related to index OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY need to be equal with "mm(ipa)".
 * - Get the AIX bus ID of "ims_00" with steps as below:
 *      -# Using R_OSAL_IoGetAxiBusIdFromDeviceName directly with device ID "ims_00"
 *      -# Using R_OSAL_IoGetDeviceAxiBusId with device handle of "ims_00":
 *          - Open "ims_00" device by using @link device_open @endlink
 *          - Get the AXI bus ID by using R_OSAL_IoGetDeviceAxiBusId
 *      -# Two AIX bus ID got by R_OSAL_IoGetAxiBusIdFromDeviceName and R_OSAL_IoGetDeviceAxiBusId need to be the same.
 * - Get the AXI bus name with steps as below:
 *      -# Get the required size of buffer which use to store the AXI bus name by using R_OSAL_IoGetAxiBusName
 *      -# Allocate a buffer with required size above.
 *      -# Get the AXI bus name by using R_OSAL_IoGetAxiBusName.<br>
 *        The AXI bus name will be stored in the allocated buffer.
 *      -# Get the value of AIX bus name array element which index is the AIX bus ID of "ims_00".
 *      -# The AXI bus name got by R_OSAL_IoGetAxiBusName and the array element value above need to be the same.
 * - Close "ims_00" by using R_OSAL_IoDeviceClose.
 * 
 * @return 0 on success
 * @return -1 on failure
 */
int io_extension_main_task()
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    osal_axi_bus_id_t ims_axi_id[2] = {OSAL_AXI_BUS_ID_INVALID, OSAL_AXI_BUS_ID_INVALID};
    osal_device_handle_t ims_handle = OSAL_DEVICE_HANDLE_INVALID;
    size_t required_size_list = 0;
    size_t numOfByte_list = 0;
    size_t required_size_name = 0;
    size_t numOfByte_name = 0;
    size_t i = 0;
    size_t j = 0;
    char *axi_name_list[100];
    char *buffer;

    printf("\n================== io_extension_main_task start ==================\n");
    /* Get the required size of AXI bus name list */
    osal_ret = R_OSAL_IoGetAxiBusNameList(NULL, 0, &required_size_list);
    if(OSAL_RETURN_OK_RQST != osal_ret)
    {
        app_ret = -1;
        OSAL_SAMPLE_ERR("Get required size of AXI bus name list failed, osal_ret = %d\n", osal_ret);
    }
    else
    {
        buffer = (char *)malloc(required_size_list);
        if(NULL == buffer)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Malloc failed\n");
        }
        else
        {
            OSAL_SAMPLE_INFO("Get the string list of all axi bus names\n");
            osal_ret = R_OSAL_IoGetAxiBusNameList(buffer, required_size_list, &numOfByte_list);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("Get AXI bus name list failed, osal_ret = %d\n", osal_ret);
            }
            else if(required_size_list != numOfByte_list)
            {
                OSAL_SAMPLE_ERR("R_OSAL_IoGetAxiBusNameList operation failed \n");
            }
            else
            {
                /* R_OSAL_IoGetAxiBusNameList work normally */
                axi_name_list[0] = &buffer[0];
                for (i = 0; i < numOfByte_list; i++)
                {
                    if (buffer[i] == '\n')
                    {
                        j++;
                        buffer[i] = '\0';
                        axi_name_list[j] = &buffer[i+1];
                    }
                }

                if(strcmp(axi_name_list[OSAL_AXI_BUS_ID_INVALID], "invalid") || strcmp(axi_name_list[OSAL_AXI_BUS_ID_IPA_MAIN_MEMORY], "mm(ipa)"))
                {
                    OSAL_SAMPLE_ERR("Content of AXI bus name list is wrong \n");
                }
            }
        }
    }

    if(0 == app_ret)
    {
        OSAL_SAMPLE_INFO("Get AXI bus ID of ims_00 by using R_OSAL_IoGetAxiBusIdFromDeviceName\n");
        osal_ret = R_OSAL_IoGetAxiBusIdFromDeviceName("imr_00", ims_axi_id);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Get AXI Bus ID from device name ims_00 failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            app_ret = device_open(ims, 0, &ims_handle);
            if(0 != app_ret)
            {
                OSAL_SAMPLE_ERR("ims_00 opening was failed \n");
            }
            else
            {
                OSAL_SAMPLE_INFO("Get AXI bus ID of ims_00 by using R_OSAL_IoGetDeviceAxiBusId\n");
                osal_ret = R_OSAL_IoGetDeviceAxiBusId(ims_handle, ims_axi_id + 1);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Get AXI Bus ID from device handle of ims_00 failed, osal_ret = %d\n", osal_ret);
                }
                else if(ims_axi_id[0] != ims_axi_id[1])
                {
                    OSAL_SAMPLE_ERR("These AXI IDs of ims_00 are not same, first ID = %ld, second ID = %ld\n", (unsigned long)ims_axi_id[0], (unsigned long)ims_axi_id[1]);
                }
                else
                {
                    /* R_OSAL_IoGetAxiBusIdFromDeviceName and R_OSAL_IoGetDeviceAxiBusId can work normally.
                     * Get the required size of ims_00 AXI bus name */
                    osal_ret = R_OSAL_IoGetAxiBusName(ims_axi_id[0], NULL, 0, &required_size_name);
                    if(OSAL_RETURN_OK_RQST != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Get required size of AXI bus name buffer failed, osal_ret = %d\n", osal_ret);
                    }
                    else
                    {
                        /* Get ims_00 AXI bus name */
                        char ims_axi_name[required_size_name];
                        OSAL_SAMPLE_INFO("Get ims_00 AXI bus name\n");
                        osal_ret = R_OSAL_IoGetAxiBusName(ims_axi_id[0], ims_axi_name, required_size_name, &numOfByte_name);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Get ims_00 AXI bus name failed, osal_ret = %d\n", osal_ret);
                        }
                        else if(required_size_name != numOfByte_name)
                        {
                            OSAL_SAMPLE_ERR("R_OSAL_IoGetAxiBusName operation failed \n");
                        }
                        else
                        {
                            /* R_OSAL_IoGetAxiBusName work normally */
                            if(0 != strcmp(ims_axi_name, axi_name_list[ims_axi_id[0]]))
                            {
                                OSAL_SAMPLE_ERR("AXI bus name (%s) is not correct in comparison with AXI bus name list (%s)\n", ims_axi_name, axi_name_list[ims_axi_id[0]]);
                            }
                        }
                    }
                }

                if(0 == app_ret)
                {
                    OSAL_SAMPLE_INFO("close ims_00\n");
                    osal_ret = R_OSAL_IoDeviceClose(ims_handle);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Device closing failed, osal_ret = %d\n", osal_ret);
                    }
                }
            }
        }
        free(buffer);
    }

    printf("================== io_extension_main_task finish ==================\n\n");
    return app_ret;
}
/**@} IO EXTENSION*/

/**
 * @defgroup int_main_task OSAL Interrupt APIs Usage
 * @{
 */
/**
 * @brief Interrupt main task:<br>
 * (OSAL API UM reference: Chapter 2.6.1: Register interrupt and Chapter 2.6.3: API restriction for interrupt context)
 * - Open "ims_00" device by using @link device_open @endlink
 * - Get the number of interrupt channel of "ims_00" by using R_OSAL_InterruptGetNumOfIrqChannels
 * - Register a callback function @link int_cb_func @endlink for channel 0 with interrupt priority is OSAL_INTERRUPT_PRIORITY_TYPE10 by using R_OSAL_InterruptRegisterIsr
 * - Change the interrupt priority from OSAL_INTERRUPT_PRIORITY_TYPE10 to OSAL_INTERRUPT_PRIORITY_TYPE11 by using R_OSAL_InterruptSetIsrPriority
 * - Enable interrupt which related to interrupt channel 0 by using R_OSAL_InterruptEnableIsr
 * - Check the context in current thread by using R_OSAL_InterruptIsISRContext.<br>
 * The expected value of "isISR" is false.
 * - Disable interrupt by using R_OSAL_InterruptDisableIsr
 * - Unregister interrupt by using R_OSAL_InterruptUnregisterIsr
 * - Close "ims_00" by using R_OSAL_IoDeviceClose.
 * 
 * @return 0 on success
 * @return -1 on failure
 */
int int_main_task()
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    osal_device_handle_t ims_handle = OSAL_DEVICE_HANDLE_INVALID;
    size_t numOfChannel = 0;
    bool isISR = true;

    printf("\n================== int_main_task start ==================\n");
    /* open "ims_00" */
    app_ret = device_open(ims, 0, &ims_handle);
    if(0 != app_ret)
    {
        OSAL_SAMPLE_ERR("ims_00 opening was failed \n");
    }
    else
    {
        OSAL_SAMPLE_INFO("get the number of interrupt channel\n");
        osal_ret = R_OSAL_InterruptGetNumOfIrqChannels(ims_handle, &numOfChannel);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Get number of interrupt channel failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            if(0 < numOfChannel)
            {
                OSAL_SAMPLE_INFO("Register a callback handler with interrupt channel 0\n");
                osal_ret = R_OSAL_InterruptRegisterIsr(ims_handle, IRQ_CHANNEL_0, OSAL_INTERRUPT_PRIORITY_TYPE10, int_cb_func , NULL);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Function callback register failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("Change the priority from OSAL_INTERRUPT_PRIORITY_TYPE10 to OSAL_INTERRUPT_PRIORITY_TYPE11\n");
                    osal_ret = R_OSAL_InterruptSetIsrPriority(ims_handle, IRQ_CHANNEL_0, OSAL_INTERRUPT_PRIORITY_TYPE11);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Interrupt priority change failed, osal_ret = %d\n", osal_ret);
                    }
                    else
                    {
                        OSAL_SAMPLE_INFO("Enable registered interrupt\n");
                        osal_ret = R_OSAL_InterruptEnableIsr(ims_handle, IRQ_CHANNEL_0);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Interrupt enable failed, osal_ret = %d\n", osal_ret);
                        }
                        else
                        {
                            OSAL_SAMPLE_INFO("Check the current context\n");
                            osal_ret = R_OSAL_InterruptIsISRContext(&isISR);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("Interrupt context checking failed, osal_ret = %d\n", osal_ret);
                            }
                            else if(true == isISR)
                            {
                                OSAL_SAMPLE_ERR("R_OSAL_InterruptIsISRContext operation failed\n");
                            }
                            else
                            {
                                /* R_OSAL_InterruptIsISRContext work normally */
                            }
                        }

                        if(0 == app_ret)
                        {
                            OSAL_SAMPLE_INFO("Disable enabled interrupt\n");
                            osal_ret = R_OSAL_InterruptDisableIsr(ims_handle, IRQ_CHANNEL_0);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("Interrupt disable failed, osal_ret = %d\n", osal_ret);
                            }
                            else
                            {
                                OSAL_SAMPLE_INFO("Unregister registered interrupt\n");
                                osal_ret = R_OSAL_InterruptUnregisterIsr(ims_handle, IRQ_CHANNEL_0, int_cb_func);
                                if(OSAL_RETURN_OK != osal_ret)
                                {
                                    app_ret = -1;
                                    OSAL_SAMPLE_ERR("Interrupt unregister failed, osal_ret = %d\n", osal_ret);
                                }
                                else
                                {
                                    OSAL_SAMPLE_INFO("close device\n");
                                    osal_ret = R_OSAL_IoDeviceClose(ims_handle);
                                    if(OSAL_RETURN_OK != osal_ret)
                                    {
                                        app_ret = -1;
                                        OSAL_SAMPLE_ERR("Can not close ims_00, osal_ret = %d\n", osal_ret);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    printf("================== int_main_task finish ==================\n\n");
    return app_ret;
}

/**
 * @brief The dummy interrupt callback function
 * 
 * @param[in] dev_handle the handle of device
 * @param[in] irq_channel the interrupt channel
 * @param[in] irq_arg the callback function argument which is a void pointer type
 */
void int_cb_func(osal_device_handle_t dev_handle, uint64_t irq_channel, void *irq_arg)
{
    (void)dev_handle;
    (void)irq_channel;
    (void)irq_arg;
}
/**@} INT*/

/**
 * @defgroup pm_main_task OSAL Power APIs Usage
 * @{
 */
/**
 * @brief Power management main task:<br>
 * (OSAL API UM reference: Chapter 2.8.1: Power control, Chapter 2.8.3: Power State Control and Chapter 2.8.4: Power State timing chart)
 * - Open "ims_00" device by using @link device_open @endlink
 * - Get the current power required status by using R_OSAL_PmGetRequiredState
 * - If current power required state is not OSAL_PM_REQUIRED_STATE_REQUIRED<br>
 * Set the power required status to OSAL_PM_REQUIRED_STATE_REQUIRED by using R_OSAL_PmSetRequiredState with "applyImmediate" is true.
 * - Get the current power reset status by using R_OSAL_PmGetResetState.
 * - If current power reset status is not OSAL_PM_RESET_STATE_APPLIED<br>
 * Set the power reset status to OSAL_PM_RESET_STATE_APPLIED by using R_OSAL_PmSetResetState
 * - Get the current power status by using R_OSAL_PmGetState.<br>
 * The expected power status is OSAL_PM_STATE_RESET
 * - Set the power reset status to OSAL_PM_RESET_STATE_RELEASED by using R_OSAL_PmSetResetState
 * - Get again the current power status by using R_OSAL_PmGetState.<br>
 * The expected power status now is OSAL_PM_STATE_READY
 * - Set power policy to OSAL_PM_POLICY_PG by using R_OSAL_PmSetPolicy with "applyImmediate" is true.
 * - Set power required state to OSAL_PM_REQUIRED_STATE_RELEASED by using R_OSAL_PmSetRequiredState with "applyImmediate" is false.
 * - Wait power state becomes OSAL_PM_STATE_PG by using R_OSAL_PmWaitForState.<br>
 * Expected the return code of this functiuon is OSAL_RETURN_TIME (Power of "ims_00" is always ON) or OSAL_RETURN_OK (On SIL env, the simulator does not use actual "ims_00")
 * - Get current power policy by using R_OSAL_PmGetPolicy.
 * - Set power policy to OSAL_PM_POLICY_CG by using R_OSAL_PmSetPolicy with "applyImmediate" is false.
 * - Wait power state become OSAL_PM_STATE_CG by using R_OSAL_PmWaitForState.
 * - Set power policy to OSAL_PM_POLICY_HP using R_OSAL_PmSetPolicy with "applyImmediate" is true.
 * - Get the current power status by using R_OSAL_PmGetState.<br>
 * The expected power status is OSAL_PM_STATE_ENABLED.
 * - Close "ims_00" by using R_OSAL_IoDeviceClose
 * 
 * @return 0 on success
 * @return -1 on failure
 */
int pm_main_task()
{
    /* local variable */
    e_osal_return_t osal_ret = OSAL_RETURN_OK;
    int app_ret = 0;
    osal_device_handle_t ims_handle = OSAL_DEVICE_HANDLE_INVALID;
    e_osal_pm_required_state_t requiredState = OSAL_PM_REQUIRED_STATE_INVALID;
    e_osal_pm_reset_state_t resetState = OSAL_PM_RESET_STATE_INVALID;
    e_osal_pm_state_t powerState = OSAL_PM_STATE_INVALID;
    e_osal_pm_policy_t powerPolicy = OSAL_PM_POLICY_INVALID;

    printf("\n================== pm_main_task start ==================\n");
    /* open "ims_00" */
    app_ret = device_open("ivdp1c", 0, &ims_handle);
    if(0 != app_ret)
    {
        OSAL_SAMPLE_ERR("ims_00 opening was failed \n");
    }
    else
    {
        OSAL_SAMPLE_INFO("Get the current power required status\n");
        osal_ret = R_OSAL_PmGetRequiredState(ims_handle, &requiredState);
        if(OSAL_RETURN_OK != osal_ret)
        {
            app_ret = -1;
            OSAL_SAMPLE_ERR("Get the power required status failed, osal_ret = %d\n", osal_ret);
        }
        else
        {
            if(OSAL_PM_REQUIRED_STATE_REQUIRED != requiredState)
            {
                OSAL_SAMPLE_INFO("Set the power required status to OSAL_PM_REQUIRED_STATE_REQUIRED\n");
                osal_ret = R_OSAL_PmSetRequiredState(ims_handle, OSAL_PM_REQUIRED_STATE_REQUIRED, true);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Set the power required status failed, osal_ret = %d\n", osal_ret);
                }
            }

            if(0 == app_ret)
            {
                OSAL_SAMPLE_INFO("Get the current power reset status\n");
                osal_ret = R_OSAL_PmGetResetState(ims_handle, &resetState);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Get the power reset status failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    if(OSAL_PM_RESET_STATE_APPLIED != resetState)
                    {
                        OSAL_SAMPLE_INFO("Set the power reset status to OSAL_PM_RESET_STATE_APPLIED\n");
                        osal_ret = R_OSAL_PmSetResetState(ims_handle, OSAL_PM_RESET_STATE_APPLIED);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("apply reset failed, osal_ret = %d\n", osal_ret);
                        }
                    }

                    if(0 == app_ret)
                    {
                        OSAL_SAMPLE_INFO("Get the current power status. Expect it's OSAL_PM_STATE_RESET\n");
                        osal_ret = R_OSAL_PmGetState(ims_handle, &powerState);
                        if(OSAL_RETURN_OK != osal_ret)
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Get power state failed, osal_ret = %d\n", osal_ret);
                        }
                        else if(OSAL_PM_STATE_RESET != powerState)
                        {
                            OSAL_SAMPLE_ERR("Operation failed\n");
                        }
                        else
                        {
                            OSAL_SAMPLE_INFO("Set the power reset status to OSAL_PM_RESET_STATE_RELEASED\n");
                            osal_ret = R_OSAL_PmSetResetState(ims_handle, OSAL_PM_RESET_STATE_RELEASED);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("Release reset failed, osal_ret = %d\n", osal_ret);
                            }
                            else
                            {
                                OSAL_SAMPLE_INFO("Get the current power status. Expect it's OSAL_PM_STATE_READY\n");
                                osal_ret = R_OSAL_PmGetState(ims_handle, &powerState);
                                if(OSAL_RETURN_OK != osal_ret)
                                {
                                    app_ret = -1;
                                    OSAL_SAMPLE_ERR("Get power state failed, osal_ret = %d\n", osal_ret);
                                }
                                else if(OSAL_PM_STATE_READY != powerState)
                                {
                                    OSAL_SAMPLE_ERR("Operation failed\n");
                                }
                                else
                                {
                                    /* PM APIs operate normally */
                                }
                            }
                        }
                    }
                }
            }

            if(0 == app_ret)
            {
                OSAL_SAMPLE_INFO("Set power policy to OSAL_PM_POLICY_PG\n");
                osal_ret = R_OSAL_PmSetPolicy(ims_handle, OSAL_PM_POLICY_PG, true);
                if(OSAL_RETURN_OK != osal_ret)
                {
                    app_ret = -1;
                    OSAL_SAMPLE_ERR("Set power policy failed, osal_ret = %d\n", osal_ret);
                }
                else
                {
                    OSAL_SAMPLE_INFO("Set power required state to OSAL_PM_REQUIRED_STATE_RELEASED\n");
                    osal_ret = R_OSAL_PmSetRequiredState(ims_handle, OSAL_PM_REQUIRED_STATE_RELEASED, false);
                    if(OSAL_RETURN_OK != osal_ret)
                    {
                        app_ret = -1;
                        OSAL_SAMPLE_ERR("Set the power required status failed, osal_ret = %d\n", osal_ret);
                    }
                    else
                    {
                        OSAL_SAMPLE_INFO("Wait power state become OSAL_PM_STATE_PG\n");
                        osal_ret = R_OSAL_PmWaitForState(ims_handle, OSAL_PM_STATE_PG, TIMEOUT);
                        if((OSAL_RETURN_TIME != osal_ret) && (OSAL_RETURN_OK != osal_ret))
                        {
                            app_ret = -1;
                            OSAL_SAMPLE_ERR("Operation failed, osal_ret = %d\n", osal_ret);
                        }
                        else
                        {
                            OSAL_SAMPLE_INFO("Get current power policy\n");
                            osal_ret = R_OSAL_PmGetPolicy(ims_handle, &powerPolicy);
                            if(OSAL_RETURN_OK != osal_ret)
                            {
                                app_ret = -1;
                                OSAL_SAMPLE_ERR("Get power policy failed, osal_ret = %d\n", osal_ret);
                            }
                            else
                            {
                                OSAL_SAMPLE_INFO("Set power policy to OSAL_PM_POLICY_CG\n");
                                osal_ret = R_OSAL_PmSetPolicy(ims_handle, OSAL_PM_POLICY_CG, false);
                                if(OSAL_RETURN_OK != osal_ret)
                                {
                                    app_ret = -1;
                                    OSAL_SAMPLE_ERR("Set power policy failed, osal_ret = %d\n", osal_ret);
                                }
                                else
                                {
                                    OSAL_SAMPLE_INFO("Wait power state become OSAL_PM_STATE_CG\n");
                                    osal_ret = R_OSAL_PmWaitForState(ims_handle, OSAL_PM_STATE_CG, TIMEOUT);
                                    if(OSAL_RETURN_OK != osal_ret)
                                    {
                                        app_ret = -1;
                                        OSAL_SAMPLE_ERR("Can not achieve the power state, osal_ret = %d\n", osal_ret);
                                    }
                                    else
                                    {
                                        OSAL_SAMPLE_INFO("Set power policy to OSAL_PM_POLICY_HP\n");
                                        osal_ret = R_OSAL_PmSetPolicy(ims_handle, OSAL_PM_POLICY_HP, true);
                                        if(OSAL_RETURN_OK != osal_ret)
                                        {
                                            app_ret = -1;
                                            OSAL_SAMPLE_ERR("Set power policy failed, osal_ret = %d\n", osal_ret);
                                        }
                                        else
                                        {
                                            OSAL_SAMPLE_INFO("Get the current power status. Expect it's OSAL_PM_STATE_ENABLED\n");
                                            osal_ret = R_OSAL_PmGetState(ims_handle, &powerState);
                                            if(OSAL_RETURN_OK != osal_ret)
                                            {
                                                app_ret = -1;
                                                OSAL_SAMPLE_ERR("Get power state failed, osal_ret = %d\n", osal_ret);
                                            }
                                            else if(OSAL_PM_STATE_ENABLED != powerState)
                                            {
                                                OSAL_SAMPLE_ERR("Operation failed\n");
                                            }
                                            else
                                            {
                                                /* PM APIs operate normally */
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if(0 == app_ret)
        {
            OSAL_SAMPLE_INFO("close device\n");
            osal_ret = R_OSAL_IoDeviceClose(ims_handle);
            if(OSAL_RETURN_OK != osal_ret)
            {
                app_ret = -1;
                OSAL_SAMPLE_ERR("Can not close ims_00, osal_ret = %d\n", osal_ret);
            }
        }
    }

    printf("================== pm_main_task finish ==================\n\n");
    return app_ret;
}
/**@} power*/

/**@} sample device*/
