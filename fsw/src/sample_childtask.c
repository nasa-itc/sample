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
    uint32 bytes = 0;
    uint32 bytes_available = 0;
    uint32 pack_count = 0;
    uint8 stream_data[SAMPLE_DEVICE_STREAM_LNGTH];
    uint32 tmp_count = 0;
    uint16 tmp_data[3] = {0, 0, 0};
    uint8 find_sync = 0;

    /* Maximum bytes searched before reporting error for unfound sync header */
    const uint8 max_bytes_before_error = SAMPLE_DEVICE_STREAM_LNGTH * 2;
    uint8 bytes_searched_for_sync = 0;

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
    ** Note: This is for an arbitrary device that sends a packet containing a header, 3 vectors, and a trailer
    **       Look at the sample_sim/sample_hardware_hardware_model.cpp for details on the data passed over the uart here
    */
    while (SAMPLE_GetRunStatus() == CFE_ES_APP_RUN)
    {
        
        /* Search for the sync header in the uart stream*/
        while(!find_sync) 
        {
            /* Take the shared device mutex to check for new streaming data */
            if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
            {
                /* Find the first sync byte */
                if ((uart_read_port(SAMPLE_AppData.SampleUart.handle, stream_data, 1) == 1) && (stream_data[0] == SAMPLE_DEVICE_HDR_0))
                {
                    /* Check if this is really the sync header or just matched the first byte */
                    if (uart_read_port(SAMPLE_AppData.SampleUart.handle, stream_data + 1, SAMPLE_DEVICE_HDR_LEN - 1) == (SAMPLE_DEVICE_HDR_LEN - 1))
                    {
                        if (stream_data[1] == SAMPLE_DEVICE_HDR_1)
                        {
                            /* Found the sync header, now read the rest of the bytes */
                            find_sync = 1;
                            bytes_searched_for_sync = 0;

                        }
                        else
                        {
                            OS_printf("Sync Header did not match, still searching for sync\n");
                        }
                    }
                    else
                    {
                        /* Uart read failed, increment error */
                        SAMPLE_AppData.HkTelemetryPkt.DeviceStreamErrorCount++;
                    } 
                }
                else 
                { 
                    if (bytes_searched_for_sync++ > max_bytes_before_error)
                    {
                        /* Haven't found a sync within a search for max_bytes_before_error tries so increment error */
                        SAMPLE_AppData.HkTelemetryPkt.DeviceStreamErrorCount++;
                        CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Streaming Uart read error, searched %d and have not sync'd", bytes_searched_for_sync);
                    }
                    
                }

                #ifdef SAMPLE_DEBUG
                    OS_printf(" Streaming device bytes searched for sync = %d\n", bytes_searched_for_sync);
                #endif

                OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
                OS_TaskDelay(1); /* Dont hog the CPU and yield to the main app for commands */
            }
        }
        
        /* Take the shared device mutex to check for new streaming data */
        if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
        {
            /* Sync was found so check for the data and read */
            bytes_available = uart_bytes_available(SAMPLE_AppData.SampleUart.handle);
            if (bytes_available >= SAMPLE_DEVICE_STREAM_LNGTH - SAMPLE_DEVICE_HDR_LEN)
            {
                /* Read data */
                bytes = uart_read_port(SAMPLE_AppData.SampleUart.handle, stream_data + SAMPLE_DEVICE_HDR_LEN, SAMPLE_DEVICE_STREAM_LNGTH - SAMPLE_DEVICE_HDR_LEN);
                /* Check if uart read succeded with the correct number of bytes */
                if (bytes != SAMPLE_DEVICE_STREAM_LNGTH - SAMPLE_DEVICE_HDR_LEN)
                {
                    CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Streaming Uart read error, expected %d and returned %d", SAMPLE_DEVICE_STREAM_LNGTH - SAMPLE_DEVICE_HDR_LEN, bytes);
                    /* Drop data */
                }
                else
                {
                    /* Check device trailer to see if it matches. Note: for a typical device this step could be a CRC, or non-existent */
                    if ((stream_data[SAMPLE_DEVICE_STREAM_LNGTH - 2] == SAMPLE_DEVICE_TRAILER_0) && (stream_data[SAMPLE_DEVICE_STREAM_LNGTH - 1] == SAMPLE_DEVICE_TRAILER_1))
                    {
                        
                        /* Interpret stream */
                        tmp_count  = stream_data[2] << 24;
                        tmp_count |= stream_data[3] << 16;
                        tmp_count |= stream_data[4] << 8;
                        tmp_count |= stream_data[5];

                        tmp_data[0]  = stream_data[6] << 8;
                        tmp_data[0] |= stream_data[7];
                        tmp_data[1]  = stream_data[8] << 8;
                        tmp_data[1] |= stream_data[9];
                        tmp_data[2]  = stream_data[10] << 8;
                        tmp_data[2] |= stream_data[11];
                        
                        /* Copy data into latest packet */
                        SAMPLE_AppData.DeviceSinglePkt.sample.count = tmp_count;
                        SAMPLE_AppData.DeviceSinglePkt.sample.data[0]  = tmp_data[0];
                        SAMPLE_AppData.DeviceSinglePkt.sample.data[1]  = tmp_data[1];
                        SAMPLE_AppData.DeviceSinglePkt.sample.data[2]  = tmp_data[2];

                        /* Copy data into packed telemetry */
                        SAMPLE_AppData.DeviceMultiPkt.sample[pack_count].count = tmp_count;
                        SAMPLE_AppData.DeviceMultiPkt.sample[pack_count].data[0]  = tmp_data[0];
                        SAMPLE_AppData.DeviceMultiPkt.sample[pack_count].data[1]  = tmp_data[1];
                        SAMPLE_AppData.DeviceMultiPkt.sample[pack_count].data[2]  = tmp_data[2];

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
                    else
                    {
                        SAMPLE_AppData.HkTelemetryPkt.DeviceStreamErrorCount++;
                        CFE_EVS_SendEvent(SAMPLE_UART_READ_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Streaming Uart read error, trailer error, received trailer = %02x%02x", stream_data[SAMPLE_DEVICE_STREAM_LNGTH - 2], stream_data[SAMPLE_DEVICE_STREAM_LNGTH - 1]);
                    }

                    #ifdef SAMPLE_DEBUG
                        OS_printf("  Stream = ");
                        for (uint32_t i = 0; i < SAMPLE_DEVICE_STREAM_LNGTH; i++)
                        {
                            OS_printf("%02x", stream_data[i]);
                        }
                        OS_printf("\n");
                        OS_printf("  Header  = %02x%02x  \n", stream_data[0], stream_data[1]);
                        OS_printf("  Counter = %08x      \n", tmp_count);
                        OS_printf("  Data [0]   = %08x \n", tmp_data[0]);
                        OS_printf("  Data [1]   = %08x \n", tmp_data[1]);
                        OS_printf("  Data [2]   = %08x \n", tmp_data[2]);
                        OS_printf("  Trailer = %02x%02x  \n", stream_data[12], stream_data[13]);
                    #endif 
                }
            }

            OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
            /* Reset the loop and sync again */
            find_sync = 0;
        }

        /* Delay between loops */
        OS_TaskDelay(SAMPLE_DEVICE_MS_LOOP_DELAY);
    }
    return status;
}