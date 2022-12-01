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
** Generic command to device
*/
int32_t SAMPLE_CommandDevice(int32_t handle, uint8_t* cmd)
{
    int32_t status = OS_SUCCESS;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;
    uint8_t read_data[SAMPLE_DEVICE_CMD_LNGTH];

    /* Write data */
    bytes = uart_write_port(handle, cmd, SAMPLE_DEVICE_CMD_LNGTH);
    if (bytes != SAMPLE_DEVICE_CMD_LNGTH)
    {
        status = OS_ERROR;
        return status;
    }

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(handle);
    while((bytes_available < SAMPLE_DEVICE_CMD_LNGTH) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(handle);
    }

    /* Check if timeout occurred */
    if (ms_timeout_counter >= SAMPLE_CFG_MS_TIMEOUT)
    {
        status = OS_ERROR;
        return status;
    }

    /* Limit bytes available */
    if (bytes_available > SAMPLE_DEVICE_CMD_LNGTH)
    {
        bytes_available = SAMPLE_DEVICE_CMD_LNGTH;
    }
    
    /* Read data */
    bytes = uart_read_port(handle, read_data, bytes_available);
    if (bytes != bytes_available)
    {
        status = OS_ERROR;
        return OS_ERROR;
    }

    /* Confirm echoed response */
    while ((bytes < SAMPLE_DEVICE_CMD_LNGTH) && (status == OS_SUCCESS))
    {
        if (read_data[bytes] != cmd[bytes])
        {
            status = OS_ERROR;
        }
    }
    return status;
}


/*
** Configuration command
*/
int32_t SAMPLE_Configuration(int32_t handle, uint32_t config)
{
    int32_t status = OS_SUCCESS;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;

    /* Prepare data to send */
    SAMPLE_Device_cmd_t device_cmd;
    device_cmd.DeviceHeader = CFE_MAKE_BIG16(SAMPLE_DEVICE_HDR);
    device_cmd.DeviceCmd = SAMPLE_DEVICE_CFG_CMD;
    device_cmd.DevicePayload = CFE_MAKE_BIG32(config);
    device_cmd.DeviceTrailer = CFE_MAKE_BIG16(SAMPLE_DEVICE_TRAILER);
    
    /* Send command */
    status = SAMPLE_CommandDevice(handle, (uint8_t*) &device_cmd);
    return status;
}


/*
** Request data command
*/
int32_t SAMPLE_RequestData(int32_t handle, SAMPLE_Device_Data_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t ms_timeout_counter = 0;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t stream_data[SAMPLE_DEVICE_DATA_LNGTH];
    uint32_t tmp_count = 0;
    uint32_t tmp_data = 0;

    /* Prepare data to send */
    SAMPLE_Device_cmd_t device_cmd;
    device_cmd.DeviceHeader = CFE_MAKE_BIG16(SAMPLE_DEVICE_HDR);
    device_cmd.DeviceCmd = SAMPLE_DEVICE_REQ_DATA_CMD;
    device_cmd.DevicePayload = 0;
    device_cmd.DeviceTrailer = CFE_MAKE_BIG16(SAMPLE_DEVICE_TRAILER);

    /* Send command */
    status = SAMPLE_CommandDevice(handle, (uint8_t*) &device_cmd);
    if (status != OS_SUCCESS)
    {
        return status;
    }

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(handle);
    while((bytes_available < SAMPLE_DEVICE_CMD_LNGTH) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(handle);
    }

    /* Check if timeout occurred */
    if (ms_timeout_counter >= SAMPLE_CFG_MS_TIMEOUT)
    {
        status = OS_ERROR;
        return status;
    }

    /* Limit bytes available */
    if (bytes_available > SAMPLE_DEVICE_CMD_LNGTH)
    {
        bytes_available = SAMPLE_DEVICE_CMD_LNGTH;
    }

    /* Read data */
    bytes = uart_read_port(handle, stream_data, SAMPLE_DEVICE_DATA_LNGTH);
    if (bytes != SAMPLE_DEVICE_DATA_LNGTH)
    {
        /* Drop data */
    }
    else
    {
        /* Verify data header and trailer */
        if ((stream_data[0]  == SAMPLE_DEVICE_HDR_0)     && 
            (stream_data[1]  == SAMPLE_DEVICE_HDR_1)     && 
            (stream_data[10] == SAMPLE_DEVICE_TRAILER_0) && 
            (stream_data[11] == SAMPLE_DEVICE_TRAILER_1) )
        {
            /* Interpret stream */
            tmp_count  = stream_data[2] << 24;
            tmp_count |= stream_data[3] << 16;
            tmp_count |= stream_data[4] << 8;
            tmp_count |= stream_data[5];

            tmp_data   = stream_data[6] << 24;
            tmp_data  |= stream_data[7] << 16;
            tmp_data  |= stream_data[8] << 8;
            tmp_data  |= stream_data[9];

            /* Copy data into latest packet */
            data->DeviceCounter = tmp_count;
            data->DeviceData = (float) tmp_data;
        }

        #ifdef SAMPLE_CFG_DEBUG
            OS_printf("  Stream = ");
            for (uint32_t i = 0; i < bytes_available; i++)
            {
                OS_printf("%02x", stream_data[i]);
            }
            OS_printf("\n");
            OS_printf("  Header  = %02x%02x  \n", stream_data[0], stream_data[1]);
            OS_printf("  Counter = %08x      \n", tmp_count);
            OS_printf("  Data    = %08x , %f \n", tmp_data, (float) tmp_data);
            OS_printf("  Trailer = %02x%02x  \n", stream_data[10], stream_data[11]);
        #endif
    }

    return status;
}
