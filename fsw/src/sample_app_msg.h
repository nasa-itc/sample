/*******************************************************************************
** File:
**  sample_app_msg.h
**
** Purpose:
**  Define Sample App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _SAMPLE_APP_MSG_H_
#define _SAMPLE_APP_MSG_H_

#include "osapi.h" // for types used below
#include "cfe_sb.h" // for CFE_SB_CMD_HDR_SIZE, CFE_SB_TLM_HDR_SIZE


/*
** SAMPLE App command codes
*/
#define SAMPLE_APP_NOOP_CC            0
#define SAMPLE_APP_RESET_COUNTERS_CC  1
#define SAMPLE_GET_DEV_DATA_CC        2
#define SAMPLE_CONFIG_CC              3
#define SAMPLE_OTHER_CMD_CC           4
#define SAMPLE_RAW_CMD_CC             5
#define SAMPLE_APP_RESET_DEV_CNTRS_CC 6
#define SAMPLE_SEND_DEV_HK_CC         7
#define SAMPLE_SEND_DEV_DATA_CC       8

/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    uint8 CmdHeader[CFE_SB_CMD_HDR_SIZE];

} SAMPLE_NoArgsCmd_t;

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which_open_mode
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef SAMPLE_NoArgsCmd_t SAMPLE_Noop_t;
typedef SAMPLE_NoArgsCmd_t SAMPLE_ResetCounters_t;
typedef SAMPLE_NoArgsCmd_t SAMPLE_Process_t;

typedef SAMPLE_NoArgsCmd_t SAMPLE_GetDevData_cmd_t;
typedef SAMPLE_NoArgsCmd_t SAMPLE_Other_cmd_t;
typedef SAMPLE_NoArgsCmd_t SAMPLE_SendDevHk_cmd_t;
typedef SAMPLE_NoArgsCmd_t SAMPLE_SendDevData_cmd_t;

/*
** SAMPLE write configuration command
*/
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint32   MillisecondStreamDelay;

} SAMPLE_Config_cmd_t;

/*
** SAMPLE raw command
*/
typedef struct
{
    uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint8    RawCmd[5];
} SAMPLE_Raw_cmd_t;

/*************************************************************************/
/*
** Type definition (SAMPLE App housekeeping)
*/

typedef struct
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
} OS_PACK SAMPLE_HkTlm_Payload_t;

typedef struct
{
    uint8                  TlmHeader[CFE_SB_TLM_HDR_SIZE];
    SAMPLE_HkTlm_Payload_t Payload;

} OS_PACK SAMPLE_HkTlm_t;

#endif /* _SAMPLE_APP_MSG_H_ */

/************************/
/*  End of File Comment */
/************************/
