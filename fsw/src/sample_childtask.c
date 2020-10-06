/*******************************************************************************
** File: sample_childtask.c
**
** Purpose:
**   This file contains the source code for the SAMPLE childtask.
**
*******************************************************************************/

/*
** Include Files
*/
#include "sample_childtask.h"


/*
** Sample device task loop
*/
int32 SAMPLE_DeviceTask(void)
{
    int32 status;
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
                        SAMPLE_AppData.DeviceSinglePkt.sample.count = tmp_count;
                        SAMPLE_AppData.DeviceSinglePkt.sample.data  = (float) tmp_data;

                        /* Copy data into packed telemetry */
                        SAMPLE_AppData.DeviceMultiPkt.sample[pack_count].count = tmp_count;
                        SAMPLE_AppData.DeviceMultiPkt.sample[pack_count].data  = (float) tmp_data;

                        /* Increment pack counter */
                        pack_count++;
                        if (pack_count == SAMPLE_DEVICE_MULTI_TLM_COUNT)
                        {
                            pack_count = 0;
                            /* Publish packed telemetry */
                            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DeviceMultiPkt);
                            CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DeviceMultiPkt);
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