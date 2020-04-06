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

    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
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
                    OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
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
                OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
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
        }

        /* Finalize telemetry */
        SAMPLE_AppData.RawIO.MillisecondDelay = ms_timeout_counter;
        SAMPLE_AppData.RawIO.ReadLength = bytes;

        /* Publish telemetry */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.RawIO);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.RawIO);

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
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

    if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(SAMPLE_AppData.SampleUart.handle, cmd, SAMPLE_DEVICE_CMD_LNGTH);
        if (bytes != SAMPLE_DEVICE_CMD_LNGTH)
        {
            CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Command uart write error, expected %d and returned %d", SAMPLE_DEVICE_CMD_LNGTH, bytes);
            status = OS_ERROR;
            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
            return status;
        }

        /* Wait until all data received or timeout occurs */
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
        CFE_EVS_SendEvent(SAMPLE_COMMANDCONFIG_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Configuration command failed");
        return status;
    }

    /* Capture configuration in HK message */
    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        SAMPLE_AppData.HkTelemetryPkt.MillisecondStreamDelay = ms_stream_delay;

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return status;
}


/*
** Sample device task loop
*/
int32 SAMPLE_DeviceTask(void)
{
    int32 status = CFE_SUCCESS;
    uint32_t bytes = 0;
    uint32_t bytes_available = 0;
    uint32_t pack_count = 0;
    uint8_t stream_data[SAMPLE_DEVICE_STREAM_LNGTH];
    uint32_t tmp_count = 0;
    uint32_t tmp_data = 0;

    /*
    ** Register the device task with Executive Services
    */
    status = CFE_ES_RegisterChildTask();
    if(status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_DEVICE_REG_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Register device task error %d", status);
        CFE_ES_ExitChildTask();
        return status;
    }
    else
    {
        CFE_EVS_SendEvent(SAMPLE_DEVICE_REG_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Device task registration complete");
    }

    /*
    ** Device Run Loop
    */
    while (SAMPLE_GetRunStatus() == CFE_ES_APP_RUN)
    {
        /* Check for streaming data */
        if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
        {
            /* Wait until all data received or timeout occurs */
            bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
            if (bytes_available >= SAMPLE_DEVICE_STREAM_LNGTH)
            {
                /* Read data */
                bytes = uart_read_port(SAMPLE_AppData.SampleUart.handle, stream_data, SAMPLE_DEVICE_STREAM_LNGTH);
                if (bytes != SAMPLE_DEVICE_STREAM_LNGTH)
                {
                    CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Streaming Uart read error, expected %d and returned %d", SAMPLE_DEVICE_STREAM_LNGTH, bytes);
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
                        CFE_EVS_SendEvent(SAMPLE_DEVICE_STREAM_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Streaming packet error with header 0x%02x%02x and trailer 0x%02x%02x", stream_data[0], stream_data[1], stream_data[10], stream_data[11]);
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

                        /* Copy data into packed telemetry */
                        SAMPLE_AppData.DevicePack.sample[pack_count].count = tmp_count;
                        SAMPLE_AppData.DevicePack.sample[pack_count].data  = (float) tmp_data;

                        /* Increment pack counter */
                        pack_count++;
                        if (pack_count == SAMPLE_DEVICE_TLM_PACK)
                        {
                            pack_count = 0;
                            /* Publish packed telemetry */
                            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePack);
                            CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePack);
                        }
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
            }

            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
        }

        /* Delay between loops */
        OS_TaskDelay(SAMPLE_DEVICE_MS_LOOP_DELAY);
    }
    return status;
}
