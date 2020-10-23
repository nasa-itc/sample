/*******************************************************************************
** File:
**  sample_device_msg.h
**
** Purpose:
**  Define Sample Device Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _SAMPLE_DEVICE_MSG_H_
#define _SAMPLE_DEVICE_MSG_H_

#include "osapi.h" // for types used below
#include "cfe_sb.h" // for CFE_SB_TLM_HDR_SIZE

/*************************************************************************/
/*
** Type definition (SAMPLE Device housekeeping)
*/

typedef struct
{
    uint8 CommandErrorCounter;
    uint8 GetDataCmdCounter;
    uint8 CfgCmdCounter;
    uint8 OtherCmdCounter;
    uint8 RawCmdCounter;
} OS_PACK SAMPLE_DeviceCmdData_t;

typedef struct
{
    uint8 CfgRespCounter;
    uint8 OtherRespCounter;
    uint8 RawRespCounter;
    uint32 UnknownResponseCounter;
    uint32 DeviceSampleDataCounter;
    uint32 MillisecondStreamDelay;
} OS_PACK SAMPLE_DeviceRespHkData_t;

typedef struct
{
    SAMPLE_DeviceCmdData_t    SAMPLE_DeviceCmdData;
    SAMPLE_DeviceRespHkData_t SAMPLE_DeviceRespHkData;
} OS_PACK SAMPLE_DeviceHkTlm_Payload_t;

typedef struct
{
    uint8                        TlmHeader[CFE_SB_TLM_HDR_SIZE];
    SAMPLE_DeviceHkTlm_Payload_t Payload;

} OS_PACK SAMPLE_DeviceHkTlm_t;

/*************************************************************************/
/*
** Type definition (SAMPLE Device Data)
*/

typedef struct
{
    uint32 SampleProcessedTimeSeconds;
    uint32 SampleProcessedTimeSubseconds;
    uint32 SamplesSent;
    uint16 SampleDataX;
    uint16 SampleDataY;
    uint16 SampleDataZ;
} OS_PACK SAMPLE_DeviceRespSampleData_t;

typedef struct
{
    SAMPLE_DeviceRespSampleData_t SAMPLE_DeviceRespSampleData;
} OS_PACK SAMPLE_SampleTlm_Payload_t;

typedef struct
{
    uint8                      TlmHeader[CFE_SB_TLM_HDR_SIZE];
    SAMPLE_SampleTlm_Payload_t Payload;

} OS_PACK SAMPLE_DeviceSampleTlm_t;

#endif /* _SAMPLE_DEVICE_MSG_H_ */

/************************/
/*  End of File Comment */
/************************/
