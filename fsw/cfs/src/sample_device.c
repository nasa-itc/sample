/*******************************************************************************
** File: sample_device.c
**
** Purpose:
**   This file contains the source code for the SAMPLE device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "sample_device.h"


/* 
** Generic read data from device
*/
int32_t SAMPLE_ReadData(uart_info_t* device, uint8_t* read_data, uint8_t data_length)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    int32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(device);
    while((bytes_available < data_length) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(device);
    }

    if (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT)
    {
        /* Limit bytes available */
        if (bytes_available > data_length)
        {
            bytes_available = data_length;
        }
        
        /* Read data */
        bytes = uart_read_port(device, read_data, bytes_available);
        if (bytes != bytes_available)
        {
            #ifdef SAMPLE_CFG_DEBUG
                OS_printf("  SAMPLE_ReadData: Bytes read != to requested! \n");
            #endif
            status = OS_ERROR;
        } /* uart_read */
    }
    else
    {
        status = OS_ERROR;
    } /* ms_timeout_counter */

    return status;
}


/* 
** Generic command to device
** Note that confirming the echoed response is specific to this implementation
*/
int32_t SAMPLE_CommandDevice(uart_info_t* device, uint8_t cmd_code, uint32_t payload)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    uint8_t write_data[SAMPLE_DEVICE_CMD_SIZE];
    uint8_t read_data[SAMPLE_DEVICE_DATA_SIZE];

    /* Prepare command */
    write_data[0] = SAMPLE_DEVICE_HDR_0;
    write_data[1] = SAMPLE_DEVICE_HDR_1;
    write_data[2] = cmd_code;
    write_data[3] = payload >> 24;
    write_data[4] = payload >> 16;
    write_data[5] = payload >> 8;
    write_data[6] = payload;
    write_data[7] = SAMPLE_DEVICE_TRAILER_0;
    write_data[8] = SAMPLE_DEVICE_TRAILER_1;

    /* Flush any prior data */
    status = uart_flush(device);
    if (status == UART_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(device, write_data, SAMPLE_DEVICE_CMD_SIZE);
        #ifdef SAMPLE_CFG_DEBUG
            OS_printf("  SAMPLE_CommandDevice[%d] = ", bytes);
            for (uint32_t i = 0; i < SAMPLE_DEVICE_CMD_SIZE; i++)
            {
                OS_printf("%02x", write_data[i]);
            }
            OS_printf("\n");
        #endif
        if (bytes == SAMPLE_DEVICE_CMD_SIZE)
        {
            status = SAMPLE_ReadData(device, read_data, SAMPLE_DEVICE_CMD_SIZE);
            if (status == OS_SUCCESS)
            {
                /* Confirm echoed response */
                bytes = 0;
                while ((bytes < (int32_t) SAMPLE_DEVICE_CMD_SIZE) && (status == OS_SUCCESS))
                {
                    if (read_data[bytes] != write_data[bytes])
                    {
                        status = OS_ERROR;
                    }
                    bytes++;
                }
            } /* SAMPLE_ReadData */
            else
            {
                #ifdef SAMPLE_CFG_DEBUG
                    OS_printf("SAMPLE_CommandDevice - SAMPLE_ReadData returned %d \n", status);
                #endif
            }
        } 
        else
        {
            #ifdef SAMPLE_CFG_DEBUG
                OS_printf("SAMPLE_CommandDevice - uart_write_port returned %d, expected %d \n", bytes, SAMPLE_DEVICE_CMD_SIZE);
            #endif
        } /* uart_write */
    } /* uart_flush*/
    return status;
}


/*
** Request housekeeping command
*/
int32_t SAMPLE_RequestHK(uart_info_t* device, SAMPLE_Device_HK_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[SAMPLE_DEVICE_HK_SIZE];

    /* Command device to send HK */
    status = SAMPLE_CommandDevice(device, SAMPLE_DEVICE_REQ_HK_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read HK data */
        status = SAMPLE_ReadData(device, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {
            #ifdef SAMPLE_CFG_DEBUG
                OS_printf("  SAMPLE_RequestHK = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
            #endif

            /* Verify data header and trailer */
            if ((read_data[0]  == SAMPLE_DEVICE_HDR_0)     && 
                (read_data[1]  == SAMPLE_DEVICE_HDR_1)     && 
                (read_data[14] == SAMPLE_DEVICE_TRAILER_0) && 
                (read_data[15] == SAMPLE_DEVICE_TRAILER_1) )
            {
                data->DeviceCounter  = read_data[2] << 24;
                data->DeviceCounter |= read_data[3] << 16;
                data->DeviceCounter |= read_data[4] << 8;
                data->DeviceCounter |= read_data[5];

                data->DeviceConfig  = read_data[6] << 24;
                data->DeviceConfig |= read_data[7] << 16;
                data->DeviceConfig |= read_data[8] << 8;
                data->DeviceConfig |= read_data[9];

                data->DeviceStatus  = read_data[10] << 24;
                data->DeviceStatus |= read_data[11] << 16;
                data->DeviceStatus |= read_data[12] << 8;
                data->DeviceStatus |= read_data[13];

                #ifdef SAMPLE_CFG_DEBUG
                    OS_printf("  Header  = 0x%02x%02x  \n", read_data[0], read_data[1]);
                    OS_printf("  Counter = 0x%08x      \n", data->DeviceCounter);
                    OS_printf("  Config  = 0x%08x      \n", data->DeviceConfig);
                    OS_printf("  Status  = 0x%08x      \n", data->DeviceStatus);
                    OS_printf("  Trailer = 0x%02x%02x  \n", read_data[14], read_data[15]);
                #endif
            }
            else
            {
                #ifdef SAMPLE_CFG_DEBUG
                    OS_printf("  SAMPLE_RequestHK: SAMPLE_ReadData reported error %d \n", status);
                #endif 
                status = OS_ERROR;
            }
        } /* SAMPLE_ReadData */
    }
    else
    {
        #ifdef SAMPLE_CFG_DEBUG
            OS_printf("  SAMPLE_RequestHK: SAMPLE_CommandDevice reported error %d \n", status);
        #endif 
    }
    return status;
}


/*
** Request data command
*/
int32_t SAMPLE_RequestData(uart_info_t* device, SAMPLE_Device_Data_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[SAMPLE_DEVICE_DATA_SIZE];

    /* Command device to send HK */
    status = SAMPLE_CommandDevice(device, SAMPLE_DEVICE_REQ_DATA_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read HK data */
        status = SAMPLE_ReadData(device, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {
            #ifdef SAMPLE_CFG_DEBUG
                OS_printf("  SAMPLE_RequestData = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
            #endif

            /* Verify data header and trailer */
            if ((read_data[0]  == SAMPLE_DEVICE_HDR_0)     && 
                (read_data[1]  == SAMPLE_DEVICE_HDR_1)     && 
                (read_data[12] == SAMPLE_DEVICE_TRAILER_0) && 
                (read_data[13] == SAMPLE_DEVICE_TRAILER_1) )
            {
                data->DeviceCounter  = read_data[2] << 24;
                data->DeviceCounter |= read_data[3] << 16;
                data->DeviceCounter |= read_data[4] << 8;
                data->DeviceCounter |= read_data[5];

                data->DeviceDataX  = read_data[6] << 8;
                data->DeviceDataX |= read_data[7];

                data->DeviceDataY  = read_data[8] << 8;
                data->DeviceDataY |= read_data[9];
                
                data->DeviceDataZ  = read_data[10] << 8;
                data->DeviceDataZ |= read_data[11];

                #ifdef SAMPLE_CFG_DEBUG
                    OS_printf("  Header  = 0x%02x%02x  \n", read_data[0], read_data[1]);
                    OS_printf("  Counter = 0x%08x      \n", data->DeviceCounter);
                    OS_printf("  Data X  = 0x%04x, %d  \n", data->DeviceDataX, data->DeviceDataX);
                    OS_printf("  Data Y  = 0x%04x, %d  \n", data->DeviceDataY, data->DeviceDataY);
                    OS_printf("  Data Z  = 0x%04x, %d  \n", data->DeviceDataZ, data->DeviceDataZ);
                    OS_printf("  Trailer = 0x%02x%02x  \n", read_data[12], read_data[13]);
                #endif
            }
        } 
        else
        {
            #ifdef SAMPLE_CFG_DEBUG
                OS_printf("  SAMPLE_RequestData: Invalid data read! \n");
            #endif 
            status = OS_ERROR;
        } /* SAMPLE_ReadData */
    }
    else
    {
        #ifdef SAMPLE_CFG_DEBUG
            OS_printf("  SAMPLE_RequestData: SAMPLE_CommandDevice reported error %d \n", status);
        #endif 
    }
    return status;
}
