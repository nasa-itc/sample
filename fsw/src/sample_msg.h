/*******************************************************************************
** File:
**   sample_msg.h
**
** Purpose:
**  Define SAMPLE application commands and telemetry messages
**
*******************************************************************************/
#ifndef _SAMPLE_MSG_H_
#define _SAMPLE_MSG_H_

#include "cfe.h"

/*
** SAMPLE command codes
*/
#define SAMPLE_NOOP_CC                 0
#define SAMPLE_RESET_COUNTERS_CC       1
#define SAMPLE_RAW_CC                  2
/* TODO: Edit and add application dependent command codes */
#define SAMPLE_CONFIG_CC               3


/*
** Generic "no arguments" command type definition
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} SAMPLE_NoArgs_cmd_t;


/*
** Raw IO command type definition
*/
typedef struct 
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint8    WriteLength;
    uint8    WriteData[256];
    uint8    MillisecondTimeout;
    uint8    ReadRequest;

} SAMPLE_RawIO_cmd_t;


/*
** SAMPLE write configuration command
*/
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32   MillisecondStreamDelay;

} SAMPLE_Config_cmd_t;


/*
** Raw IO telemetry type definition
*/
typedef struct
{
    /* All telemetry requires a header used to identify it */
    uint8   TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8   WriteLength;
    uint8   WriteData[256];
    uint8   MillisecondTimeout;
    uint8   MillisecondDelay;
    uint8   ReadRequest;
    uint8   ReadLength;
    uint8   ReadData[256];

} OS_PACK SAMPLE_RawIO_tlm_t;
#define SAMPLE_RAWIO_TLM_LNGTH sizeof ( SAMPLE_RawIO_tlm_t )


/*
** SAMPLE device sample definition
*/
typedef struct
{
    uint32 count;
    float  data;
} SAMPLE_Device_sample_t;


/*
** SAMPLE device telemetry definition
*/
typedef struct 
{
    uint8   TlmHeader[CFE_SB_TLM_HDR_SIZE];
    SAMPLE_Device_sample_t sample;

} OS_PACK SAMPLE_Device_tlm_t;
#define SAMPLE_DEVICE_TLM_LNGTH sizeof ( SAMPLE_Device_tlm_t )


/*
** SAMPLE device telemetry pack definition
*/
typedef struct 
{
    uint8   TlmHeader[CFE_SB_TLM_HDR_SIZE];
    SAMPLE_Device_sample_t sample[SAMPLE_DEVICE_TLM_PACK];

} OS_PACK SAMPLE_DevicePack_tlm_t;
#define SAMPLE_DEVICE_PACK_TLM_LNGTH sizeof ( SAMPLE_DevicePack_tlm_t )


/*
** SAMPLE housekeeping type definition
*/
typedef struct 
{
    uint8   TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8   CommandErrorCount;
    uint8   CommandCount;
  
    /*
    ** TODO: Edit and add specific telemetry values to this struct
    */
    uint32  MillisecondStreamDelay;

} OS_PACK SAMPLE_Hk_tlm_t;
#define SAMPLE_HK_TLM_LNGTH sizeof ( SAMPLE_Hk_tlm_t )

#endif /* _SAMPLE_MSG_H_ */
