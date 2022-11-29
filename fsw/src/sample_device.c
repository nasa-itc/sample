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
** Raw input / output command
*/ 
int32 SAMPLE_RawIO(void)
{
    int32 status = OS_SUCCESS;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;
    SAMPLE_RawIO_cmd_t* raw_cmd = (SAMPLE_RawIO_cmd_t*) SAMPLE_AppData.MsgPtr;

    /* Initialize telemetry */
    SAMPLE_AppData.RawIO.WriteLength = raw_cmd->WriteLength;
    CFE_PSP_MemCpy((void*)SAMPLE_AppData.RawIO.WriteData, raw_cmd->WriteData, sizeof(raw_cmd->WriteData));
    SAMPLE_AppData.RawIO.MillisecondTimeout = raw_cmd->MillisecondTimeout;
    SAMPLE_AppData.RawIO.ReadRequest = raw_cmd->ReadRequest;
    CFE_PSP_MemSet((void*)SAMPLE_AppData.RawIO.ReadData, 0x00, sizeof(SAMPLE_AppData.RawIO.ReadData));
    
    /* Flush existing receive data */
    bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
    if (bytes_available > 0)
    {
        bytes = uart_read_port(SAMPLE_AppData.sampleUart.handle, NULL, bytes_available);
        if (bytes != bytes_available)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart read error, expected %d and returned %d", bytes_available, bytes);
            status = OS_ERROR;
            return status;
        }
    }

    /* Write data */
    bytes = uart_write_port(SAMPLE_AppData.sampleUart.handle, raw_cmd->WriteData, raw_cmd->WriteLength);
    if (bytes != raw_cmd->WriteLength)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart write error, expected %d and returned %d", raw_cmd->WriteLength, bytes);
        status = OS_ERROR;
        return status;
    }

    if (raw_cmd->ReadRequest > 0)
    {
        /* Wait until all data received or timeout occurs */
        bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
        while((bytes_available < raw_cmd->ReadRequest) && (ms_timeout_counter < raw_cmd->MillisecondTimeout))
        {
            ms_timeout_counter++;
            OS_TaskDelay(1);
            bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
        }

        /* Check if timeout occurred */
        if (ms_timeout_counter >= raw_cmd->MillisecondTimeout)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_TIMEOUT_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart timeout error");
            status = OS_ERROR;
            /* Proceed with error */
        }

        /* Limit bytes available */
        if (bytes_available > sizeof(SAMPLE_AppData.RawIO.ReadData))
        {
            bytes_available = sizeof(SAMPLE_AppData.RawIO.ReadData);
        }

        /* Read data */
        bytes = uart_read_port(SAMPLE_AppData.sampleUart.handle, SAMPLE_AppData.RawIO.ReadData, bytes_available);
        if (bytes != bytes_available)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart read error, expected %d and returned %d", bytes_available, bytes);
            status = OS_ERROR;
            /* Proceed with error */
        }
    }

    /* Finalize telemetry */
    SAMPLE_AppData.RawIO.MillisecondDelay = ms_timeout_counter;
    SAMPLE_AppData.RawIO.ReadLength = bytes;

    /* Publish telemetry */
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.RawIO);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.RawIO);

    return status;
}


/* 
** Generic command to device
*/
int32 SAMPLE_CommandDevice(uint8* cmd)
{
    int32 status = OS_SUCCESS;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;
    uint8_t read_data[SAMPLE_DEVICE_CMD_LNGTH];

    /* Write data */
    bytes = uart_write_port(SAMPLE_AppData.sampleUart.handle, cmd, SAMPLE_DEVICE_CMD_LNGTH);
    if (bytes != SAMPLE_DEVICE_CMD_LNGTH)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command uart write error, expected %d and returned %d", SAMPLE_DEVICE_CMD_LNGTH, bytes);
        status = OS_ERROR;
        return status;
    }

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
    while((bytes_available < SAMPLE_DEVICE_CMD_LNGTH) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
    }

    /* Check if timeout occurred */
    if (ms_timeout_counter >= SAMPLE_CFG_MS_TIMEOUT)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_TIMEOUT_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command Uart timeout error");
        status = OS_ERROR;
        return status;
    }

    /* Limit bytes available */
    if (bytes_available > SAMPLE_DEVICE_CMD_LNGTH)
    {
        bytes_available = SAMPLE_DEVICE_CMD_LNGTH;
    }
    
    /* Read data */
    bytes = uart_read_port(SAMPLE_AppData.sampleUart.handle, read_data, bytes_available);
    if (bytes != bytes_available)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command Uart read error, expected %d and returned %d", bytes_available, bytes);
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
int32 SAMPLE_Configuration(void)
{
    int32 status = OS_SUCCESS;
    SAMPLE_Config_cmd_t* cfg_cmd = (SAMPLE_Config_cmd_t*) SAMPLE_AppData.MsgPtr;
    SAMPLE_Device_cmd_t device_cmd;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint32_t device_config = CFE_MAKE_BIG32(cfg_cmd->DeviceCfg);

    /* Prepare data to send */
    device_cmd.DeviceHeader = CFE_MAKE_BIG16(SAMPLE_DEVICE_HDR);
    device_cmd.DeviceCmd = SAMPLE_DEVICE_CFG_CMD;
    device_cmd.DevicePayload = cfg_cmd->DeviceCfg;
    device_cmd.DeviceTrailer = CFE_MAKE_BIG16(SAMPLE_DEVICE_TRAILER);
    
    /* Send command */
    status = SAMPLE_CommandDevice((uint8*) &device_cmd);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_COMMANDCONFIG_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Configuration command failed");
        return status;
    }

    /* Capture configuration in HK message */
    SAMPLE_AppData.HkTelemetryPkt.DeviceCfg = device_config;

    return status;
}


/*
** Request data command
*/
int32 SAMPLE_RequestData(void)
{
    int32 status = OS_SUCCESS;
    SAMPLE_Device_cmd_t device_cmd;
    uint8_t ms_timeout_counter = 0;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t stream_data[SAMPLE_DEVICE_STREAM_LNGTH];
    uint32_t tmp_count = 0;
    uint32_t tmp_data = 0;

    /* Prepare data to send */
    device_cmd.DeviceHeader = CFE_MAKE_BIG16(SAMPLE_DEVICE_HDR);
    device_cmd.DeviceCmd = SAMPLE_DEVICE_REQ_DATA_CMD;
    device_cmd.DevicePayload = 0;
    device_cmd.DeviceTrailer = CFE_MAKE_BIG16(SAMPLE_DEVICE_TRAILER);

    /* Send command */
    status = SAMPLE_CommandDevice((uint8*) &device_cmd);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_COMMANDREQDATA_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Request data command failed");
        return status;
    }

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
    while((bytes_available < SAMPLE_DEVICE_CMD_LNGTH) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(SAMPLE_AppData.sampleUart.handle);
    }

    /* Check if timeout occurred */
    if (ms_timeout_counter >= SAMPLE_CFG_MS_TIMEOUT)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_TIMEOUT_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Request data Uart timeout error");
        status = OS_ERROR;
        return status;
    }

    /* Limit bytes available */
    if (bytes_available > SAMPLE_DEVICE_CMD_LNGTH)
    {
        bytes_available = SAMPLE_DEVICE_CMD_LNGTH;
    }

    /* Read data */
    bytes = uart_read_port(SAMPLE_AppData.sampleUart.handle, stream_data, SAMPLE_DEVICE_STREAM_LNGTH);
    if (bytes != SAMPLE_DEVICE_STREAM_LNGTH)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Request data Uart read error, expected %d and returned %d", SAMPLE_DEVICE_STREAM_LNGTH, bytes);
        /* Drop data */
    }
    else
    {
        /* Verify data header and trailer */
        if ((stream_data[0] != SAMPLE_DEVICE_HDR_0)      || 
            (stream_data[1] != SAMPLE_DEVICE_HDR_1)      || 
            (stream_data[10] != SAMPLE_DEVICE_TRAILER_0) || 
            (stream_data[11] != SAMPLE_DEVICE_TRAILER_1) )
        {
            CFE_EVS_SendEvent(SAMPLE_COMMANDREQDATA_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Request data packet error with header 0x%02x%02x and trailer 0x%02x%02x", stream_data[0], stream_data[1], stream_data[10], stream_data[11]);
        }
        else
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
            SAMPLE_AppData.DevicePkt.sample.count = tmp_count;
            SAMPLE_AppData.DevicePkt.sample.data  = (float) tmp_data;

            /* Publish telemetry packet */
            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePkt);
            CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePkt);
        }

        #ifdef SAMPLE_DEBUG
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
