/*******************************************************************************
** File: 
**  sample_app.h
**
** Purpose:
**   This file is main header file for the Sample application.
**
*******************************************************************************/
#ifndef _SAMPLE_APP_H_
#define _SAMPLE_APP_H_

/*
** Required header files.
*/
#include "sample_app_msg.h"
#include "sample_app_events.h"
#include "cfe_sb.h"
#include "cfe_evs.h"

/***********************************************************************/
#define SAMPLE_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

/************************************************************************
** Type Definitions
*************************************************************************/

/*
 * Buffer to hold telemetry data prior to sending
 * Defined as a union to ensure proper alignment for a CFE_SB_Msg_t type
 */
typedef union
{
    CFE_SB_Msg_t   MsgHdr;
    SAMPLE_HkTlm_t HkTlm;
} SAMPLE_HkBuffer_t;

/*
** Global Data
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet...
    */
    SAMPLE_HkBuffer_t HkBuf;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;
    CFE_SB_MsgPtr_t MsgPtr;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[16];
    uint16 PipeDepth;

    CFE_EVS_BinFilter_t EventFilters[SAMPLE_EVENT_COUNTS];

} SAMPLE_AppData_t;

/****************************************************************************/
/*
** Function prototypes.
**
** Note: Except for the entry point (SAMPLE_AppMain), these
**       functions are not called from any other source module.
*/
void  SAMPLE_AppMain(void);

#endif /* _sample_app_h_ */

/************************/
/*  End of File Comment */
/************************/
