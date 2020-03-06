/*******************************************************************************
** File:
**   sample_app_msg.h
**
** Purpose:
**  Define SAMPLE App Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _SAMPLE_APP_MSG_H_
#define _SAMPLE_APP_MSG_H_

/*
** SAMPLE App command codes
*/
#define SAMPLE_APP_NOOP_CC                 0
#define SAMPLE_APP_RESET_COUNTERS_CC       1
/* todo - add application dependent command codes here */



/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} SAMPLE_NoArgsCmd_t;


/*
** Type definition (SAMPLE STF1 App housekeeping)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              CommandErrorCount;
    uint8              CommandCount;
  
    /*
    ** todo - add app specific telemetry values to this struct
    */

} SAMPLE_Hk_tlm_t;

#define SAMPLE_APP_HK_TLM_LNGTH  sizeof ( SAMPLE_Hk_tlm_t )

#endif 
