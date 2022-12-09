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
int32_t SAMPLE_ReadData(int32_t handle, uint8_t* read_data, uint8_t data_length)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    int32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(handle);
    while((bytes_available < data_length) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(handle);
    }

    if (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT)
    {
        /* Limit bytes available */
        if (bytes_available > data_length)
        {
            bytes_available = data_length;
        }
        
        /* Read data */
        bytes = uart_read_port(handle, read_data, bytes_available);
        if (bytes != bytes_available)
        {
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
int32_t SAMPLE_CommandDevice(int32_t handle, uint8_t cmd_code, uint32_t payload)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    uint8_t read_data[SAMPLE_DEVICE_CMD_LNGTH];

    /* Prepare command */
    SAMPLE_Device_cmd_t device_cmd;
    uint8_t* device_cmd_ptr = (uint8_t*) &device_cmd;
    device_cmd.DeviceHeader = CFE_MAKE_BIG16(SAMPLE_DEVICE_HDR);
    device_cmd.DeviceCmd = cmd_code;
    device_cmd.DevicePayload = CFE_MAKE_BIG32(payload);
    device_cmd.DeviceTrailer = CFE_MAKE_BIG16(SAMPLE_DEVICE_TRAILER);

    /* Flush any prior data */
    status = uart_flush(handle);
    if (status != UART_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(handle, (uint8_t*) &device_cmd, SAMPLE_DEVICE_CMD_LNGTH);
        if (bytes == SAMPLE_DEVICE_CMD_LNGTH)
        {
            status = SAMPLE_ReadData(handle, read_data, SAMPLE_DEVICE_CMD_LNGTH);
            if (status == OS_SUCCESS)
            {
                /* Confirm echoed response */
                bytes = 0;
                while ((bytes < (int32_t) SAMPLE_DEVICE_CMD_LNGTH) && (status == OS_SUCCESS))
                {
                    if (read_data[bytes] != device_cmd_ptr[bytes])
                    {
                        status = OS_ERROR;
                    }
                    bytes++;
                }
            } /* SAMPLE_ReadData */
        } /* uart_write */
    } /* uart_flush*/
    return status;
}


/*
** Request housekeeping command
*/
int32_t SAMPLE_RequestHK(int32_t handle, SAMPLE_Device_HK_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[16];

    /* Command device to send HK */
    status = SAMPLE_CommandDevice(handle, SAMPLE_DEVICE_REQ_HK_CMD, 0);

    /* Read HK data */
    status = SAMPLE_ReadData(handle, read_data, sizeof(read_data));
    if (status == OS_SUCCESS)
    {
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
                OS_printf("  SAMPLE_RequestHK = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
                OS_printf("  Header  = %02x%02x  \n", read_data[0], read_data[1]);
                OS_printf("  Counter = %08x      \n", data->DeviceCounter);
                OS_printf("  Config  = %08x      \n", data->DeviceConfig);
                OS_printf("  Status  = %08x      \n", data->DeviceStatus);
                OS_printf("  Trailer = %02x%02x  \n", read_data[14], read_data[15]);
            #endif
        }
        else
        {
            status = OS_ERROR;
        }
    }
    return status;
}


/*
** Request data command
*/
int32_t SAMPLE_RequestData(int32_t handle, SAMPLE_Device_Data_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[12];
    uint32_t tmp = 0x00000000;

    /* Command device to send HK */
    status = SAMPLE_CommandDevice(handle, SAMPLE_DEVICE_REQ_DATA_CMD, 0);

    /* Read HK data */
    status = SAMPLE_ReadData(handle, read_data, sizeof(read_data));
    if (status == OS_SUCCESS)
    {
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

            tmp  = read_data[6] << 24;
            tmp |= read_data[7] << 16;
            tmp |= read_data[8] << 8;
            tmp |= read_data[9];

            data->DeviceData = (float) tmp;

            #ifdef SAMPLE_CFG_DEBUG
                OS_printf("  SAMPLE_RequestData = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
                OS_printf("  Header  = %02x%02x  \n", read_data[0], read_data[1]);
                OS_printf("  Counter = %08x      \n", data->DeviceCounter);
                OS_printf("  Data    = %08x, %f  \n", (uint32_t) data->DeviceData, data->DeviceData);
                OS_printf("  Trailer = %02x%02x  \n", read_data[10], read_data[11]);
            #endif
        }
        else
        {
            status = OS_ERROR;
        }
    }
    return status;
}
