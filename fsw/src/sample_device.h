/*******************************************************************************
** File:
**  sample_device.h
**
** Purpose:
**   This file is the header file for the Sample device
**
**
*******************************************************************************/

#ifndef _SAMPLE_DEVICE_H_
#define _SAMPLE_DEVICE_H_

#include "sample_device_msg.h"

/*
 * Buffers to hold telemetry data prior to sending
 * Defined as a union to ensure proper alignment for a CFE_SB_Msg_t type
 */

typedef union
{
    CFE_SB_Msg_t         MsgHdr;
    SAMPLE_DeviceHkTlm_t HkTlm;
} SAMPLE_DeviceHkBuffer_t;

typedef union
{
    CFE_SB_Msg_t             MsgHdr;
    SAMPLE_DeviceSampleTlm_t SampleTlm;
} SAMPLE_DeviceSampleBuffer_t;

/*
** Run Status variable used in the main processing loop.  If the device is asynchronous, this Status
** variable is also used in the device child processing loop.
*/
extern uint32 RunStatus;

/****************************************************************************/
/*
** Function prototypes.
**
*/
int32 SAMPLE_DeviceInit(void);
int32 SAMPLE_DeviceShutdown(void);
void  SAMPLE_DeviceResetCounters(void);

void  SAMPLE_DeviceGetSampleDataCommand(void);
void  SAMPLE_DeviceConfigurationCommand(uint32_t millisecond_stream_delay);
void  SAMPLE_DeviceOtherCommand(void);
void  SAMPLE_DeviceRawCommand(const uint8 cmd[], const uint32_t cmd_length);

void SAMPLE_ReportDeviceHousekeeping(void);
void SAMPLE_ReportDeviceSampleData(void);

#endif

/************************/
/*  End of File Comment */
/************************/
