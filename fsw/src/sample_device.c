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
    int32 status;
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
    
    /* Device protocol work */
    if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
    {
        /* Flush existing receive data */
        bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
        if (bytes_available > 0)
        {
            bytes = uart_read_port(SAMPLE_AppData.SampleUart.handle, NULL, bytes_available);
            if (bytes != bytes_available)
            {
                CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart read error, expected %d and returned %d", bytes_available, bytes);
                OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
                status = OS_ERROR;
                return status;
            }
        }

        /* Write data */
        bytes = uart_write_port(SAMPLE_AppData.SampleUart.handle, raw_cmd->WriteData, raw_cmd->WriteLength);
        if (bytes != raw_cmd->WriteLength)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart write error, expected %d and returned %d", raw_cmd->WriteLength, bytes);
            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
            status = OS_ERROR;
            return status;
        }

        if (raw_cmd->ReadRequest > 0)
        {
            /* Wait until all data received or timeout occurs */
            bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
            while((bytes_available < raw_cmd->ReadRequest) && (ms_timeout_counter < raw_cmd->MillisecondTimeout))
            {
                ms_timeout_counter++;
                OS_TaskDelay(1);
                bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
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
            bytes = uart_read_port(SAMPLE_AppData.SampleUart.handle, SAMPLE_AppData.RawIO.ReadData, bytes_available);
            if (bytes != bytes_available)
            {
                CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: RawIO Uart read error, expected %d and returned %d", bytes_available, bytes);
                status = OS_ERROR;
                /* Proceed with error */
            }
        }

        OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
        

        /* Finalize telemetry */
        SAMPLE_AppData.RawIO.MillisecondDelay = ms_timeout_counter;
        SAMPLE_AppData.RawIO.ReadLength = bytes;

        /* Publish telemetry */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.RawIO);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.RawIO);

    }
    return status;
}


/* 
** Generic command to device
*/
int32 SAMPLE_CommandDevice(uint8* cmd)
{
    int32 status;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;
    uint8_t read_data[SAMPLE_DEVICE_CMD_LNGTH];

    if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(SAMPLE_AppData.SampleUart.handle, cmd, SAMPLE_DEVICE_CMD_LNGTH);
        if (bytes != SAMPLE_DEVICE_CMD_LNGTH)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_WRITE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command uart write error, expected %d and returned %d", SAMPLE_DEVICE_CMD_LNGTH, bytes);
            status = OS_ERROR;
            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
            return status;
        }

        /* Wait until all data received or timeout occurs, in this sample the simulator is expected to respond */
        bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
        while((bytes_available < SAMPLE_DEVICE_CMD_LNGTH) && (ms_timeout_counter < SAMPLE_CFG_MS_TIMEOUT))
        {
            ms_timeout_counter++;
            OS_TaskDelay(1);
            bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
        }

        /* Check if timeout occurred */
        if (ms_timeout_counter >= SAMPLE_CFG_MS_TIMEOUT)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_TIMEOUT_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command Uart timeout error");
            status = OS_ERROR;
            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
            return status;
        }

        /* Limit bytes available */
        if (bytes_available > SAMPLE_DEVICE_CMD_LNGTH)
        {
            bytes_available = SAMPLE_DEVICE_CMD_LNGTH;
        }
        
        /* Read data */
        bytes = uart_read_port(SAMPLE_AppData.SampleUart.handle, read_data, bytes_available);
        if (bytes != bytes_available)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command Uart read error, expected %d and returned %d", bytes_available, bytes);
            status = OS_ERROR;
            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
            return OS_ERROR;
        }

        OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
    }

    /* 
    ** In this example, the device responds to commands with an echo so confirm echoed response 
    ** Note: In most practical cases the device will have some other means of confirmation
    */
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
    int32 status;
    SAMPLE_Config_cmd_t* cfg_cmd = (SAMPLE_Config_cmd_t*) SAMPLE_AppData.MsgPtr;
    SAMPLE_Device_cmd_t device_cmd;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint8_t ms_timeout_counter = SAMPLE_CFG_MS_TIMEOUT;
    uint32_t ms_stream_delay = CFE_MAKE_BIG32(cfg_cmd->MillisecondStreamDelay);

    /* Prepare data to send */
    device_cmd.DeviceHeader = CFE_MAKE_BIG16(SAMPLE_DEVICE_HDR);
    device_cmd.DeviceCmd = SAMPLE_DEVICE_CFG_CMD;
    device_cmd.DevicePayload = cfg_cmd->MillisecondStreamDelay;
    device_cmd.DeviceTrailer = CFE_MAKE_BIG16(SAMPLE_DEVICE_TRAILER);
    
    /* Send command */
    status = SAMPLE_CommandDevice((uint8*) &device_cmd);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_COMMANDCONFIG_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Configuration command failed: CMD -> %d, Payload -> %d, Status -> %d",
            device_cmd.DeviceCmd, device_cmd.DevicePayload, status);
        return status;
    }

    /* Capture configuration in HK message */
    if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
    {
        SAMPLE_AppData.HkTelemetryPkt.MillisecondStreamDelay = ms_stream_delay;

        OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
    }
    return status;
}