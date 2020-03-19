/*******************************************************************************
** File: sample_app.h
**
** Purpose:
**   This is the header file for the SAMPLE device.
**
*******************************************************************************/
#ifndef _SAMPLE_DEVICE_H_
#define _SAMPLE_DEVICE_H_

/*
** Required header files.
*/
#include "sample_app.h"

/*
** Type definitions
** TODO: Make specific to your application
*/
#define SAMPLE_DEVICE_HDR          0xDEAD

#define SAMPLE_DEVICE_CFG_CMD      0x01
#define SAMPLE_DEVICE_OTHER_CMD    0x02

#define SAMPLE_DEVICE_TRAILER      0xBEEF


/*
** SAMPLE write configuration command
*/
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32   MillisecondStreamDelay;

} SAMPLE_Config_cmd_t;


/*
** SAMPLE device command
*/
typedef struct 
{
    uint16  DeviceHeader;
    uint8   DeviceCmd;
    uint32  DevicePayload;
    uint16  DeviceTrailer;

} SAMPLE_Device_cmd_t;
#define SAMPLE_DEVICE_CMD_LNGTH sizeof ( SAMPLE_Device_cmd_t )


/*
** SAMPLE device streamed telemetry definition
*/
typedef struct
{
    uint16  DeviceHeader;
    uint32  DeviceCounter;
    uint32  DeviceData;
    uint16  DeviceTrailer;

} SAMPLE_Device_Stream_tlm_t;
#define SAMPLE_DEVICE_STREAM_LNGTH sizeof ( SAMPLE_Device_Stream_tlm_t )


/*
** Prototypes
*/
void  SAMPLE_RawIO(void);
int32 SAMPLE_CommandDevice(uint8* cmd);
void  SAMPLE_Configuration(void);
int32 SAMPLE_DeviceTask(void);


#endif /* _SAMPLE_DEVICE_H_ */
