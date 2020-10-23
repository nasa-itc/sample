/*******************************************************************************
** File: 
**  sample_app.c
**
** Purpose:
**   This file contains the source code for the Sample App.
**
*******************************************************************************/

/*
** Include Files:
*/

//#include "string.h"
//#include "sample_app_events.h"

#include "sample_app.h"
#include "sample_app_version.h"
#include "sample_app_msgids.h"
#include "sample_app_perfids.h"
#include "sample_device.h"
#include "cfe_error.h"

/*
** global data
*/
SAMPLE_AppData_t SAMPLE_AppData;

// Forward declarations
static int32 SAMPLE_AppInit(void);
static void  SAMPLE_ProcessCommandPacket(CFE_SB_MsgPtr_t Msg);
static void  SAMPLE_ProcessGroundCommand(CFE_SB_MsgPtr_t Msg);
static int32 SAMPLE_ReportHousekeeping(const CFE_SB_CmdHdr_t *Msg);
static int32 SAMPLE_ResetCounters(const SAMPLE_ResetCounters_t *Msg);
static int32 SAMPLE_Noop(const SAMPLE_Noop_t *Msg);
static bool SAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t Msg, uint16 ExpectedLength);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Name:  SAMPLE_AppMain()                                                    */
/* Purpose:                                                                   */
/*        Application entry point and main process loop                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SAMPLE_AppMain(void)
{
    int32 status;

    /*
    ** Register the app with Executive services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SAMPLE_APP_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = SAMPLE_AppInit();
    if (status != CFE_SUCCESS)
    {
        RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** SAMPLE Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(SAMPLE_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_RcvMsg(&SAMPLE_AppData.MsgPtr, SAMPLE_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(SAMPLE_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SAMPLE_ProcessCommandPacket(SAMPLE_AppData.MsgPtr);
        }
        else
        {
            CFE_EVS_SendEvent(SAMPLE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE APP: SB Pipe Read Error, App Will Exit");

            RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    RunStatus = CFE_ES_RunStatus_APP_EXIT; // we are wanting to exit... make sure everyone knows it

    status = SAMPLE_DeviceShutdown();
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Shutting Down Device, RC = 0x%08lX\n", (unsigned long)status);
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(SAMPLE_APP_PERF_ID);

    CFE_ES_ExitApp(RunStatus);

} /* End of SAMPLE_AppMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Name:  SAMPLE_AppInit()                                                    */
/*                                                                            */
/* Purpose:                                                                   */
/*        Initialization                                                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static int32 SAMPLE_AppInit(void)
{
    int32 status;

    RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandCounter = 0;
    SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter = 0;

    /*
    ** Initialize app configuration data
    */
    SAMPLE_AppData.PipeDepth = SAMPLE_PIPE_DEPTH;

    /*
    ** Initialize event filter table...
    */
    SAMPLE_AppData.EventFilters[0].EventID = SAMPLE_STARTUP_INF_EID;
    SAMPLE_AppData.EventFilters[0].Mask    = 0x0000;
    SAMPLE_AppData.EventFilters[1].EventID = SAMPLE_COMMAND_ERR_EID;
    SAMPLE_AppData.EventFilters[1].Mask    = 0x0000;
    SAMPLE_AppData.EventFilters[2].EventID = SAMPLE_COMMANDNOP_INF_EID;
    SAMPLE_AppData.EventFilters[2].Mask    = 0x0000;
    SAMPLE_AppData.EventFilters[3].EventID = SAMPLE_COMMANDRST_INF_EID;
    SAMPLE_AppData.EventFilters[3].Mask    = 0x0000;
    SAMPLE_AppData.EventFilters[4].EventID = SAMPLE_INVALID_MSGID_ERR_EID;
    SAMPLE_AppData.EventFilters[4].Mask    = 0x0000;
    SAMPLE_AppData.EventFilters[5].EventID = SAMPLE_LEN_ERR_EID;
    SAMPLE_AppData.EventFilters[5].Mask    = 0x0000;
    SAMPLE_AppData.EventFilters[6].EventID = SAMPLE_PIPE_ERR_EID;
    SAMPLE_AppData.EventFilters[6].Mask    = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(SAMPLE_AppData.EventFilters, SAMPLE_EVENT_COUNTS, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.HkBuf.MsgHdr, SAMPLE_APP_HK_TLM_MID, sizeof(SAMPLE_AppData.HkBuf), true);

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&SAMPLE_AppData.CommandPipe, SAMPLE_AppData.PipeDepth, "SAMPLE_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(SAMPLE_APP_SEND_HK_MID, SAMPLE_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(SAMPLE_APP_CMD_MID, SAMPLE_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    status = SAMPLE_DeviceInit();
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Initializing Device, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    CFE_EVS_SendEvent(SAMPLE_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "SAMPLE App Initialized. Version %d.%d.%d.%d",
                      SAMPLE_APP_MAJOR_VERSION,
                      SAMPLE_APP_MINOR_VERSION,
                      SAMPLE_APP_REVISION,
                      SAMPLE_APP_MISSION_REV);

    return (CFE_SUCCESS);

} /* End of SAMPLE_AppInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SAMPLE_ProcessCommandPacket                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
static void SAMPLE_ProcessCommandPacket(CFE_SB_MsgPtr_t Msg)
{
    CFE_SB_MsgId_t MsgId;

    MsgId = CFE_SB_GetMsgId(Msg);

    switch (MsgId)
    {
        case SAMPLE_APP_CMD_MID:
            SAMPLE_ProcessGroundCommand(Msg);
            break;

        case SAMPLE_APP_SEND_HK_MID:
            SAMPLE_ReportHousekeeping((CFE_SB_CmdHdr_t *)Msg);
            break;

        default:
            CFE_EVS_SendEvent(SAMPLE_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }

    return;

} /* End SAMPLE_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Name:  SAMPLE_ProcessGroundCommand()                                       */
/*                                                                            */
/* Purpose:                                                                   */
/*        SAMPLE ground commands                                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static void SAMPLE_ProcessGroundCommand(CFE_SB_MsgPtr_t Msg)
{
    uint16 CommandCode = CFE_SB_GetCmdCode(Msg);
    SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandCounter++;

    /*
    ** Process "known" SAMPLE app ground commands
    */
    switch (CommandCode)
    {
        case SAMPLE_APP_NOOP_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_Noop_t)))
            {
                SAMPLE_Noop((SAMPLE_Noop_t *)Msg);
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }

            break;

        case SAMPLE_APP_RESET_COUNTERS_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_ResetCounters_t)))
            {
                SAMPLE_ResetCounters((SAMPLE_ResetCounters_t *)Msg);
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }

            break;

        /*
        ** TODO: Edit and add more command codes as appropriate for the application
        */
        case SAMPLE_APP_RESET_DEV_CNTRS_CC:
            SAMPLE_DeviceResetCounters();
            break;

        case SAMPLE_GET_DEV_DATA_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_GetDevData_cmd_t))) {
                SAMPLE_DeviceGetSampleDataCommand();
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }
            break;

        case SAMPLE_CONFIG_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_Config_cmd_t)))
            {
                SAMPLE_DeviceConfigurationCommand(((SAMPLE_Config_cmd_t *)Msg)->MillisecondStreamDelay);
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }
            break;

        case SAMPLE_OTHER_CMD_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_Other_cmd_t)))
            {
                SAMPLE_DeviceOtherCommand();
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }
            break;

        case SAMPLE_RAW_CMD_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_Raw_cmd_t)))
            {
                SAMPLE_DeviceRawCommand(((SAMPLE_Raw_cmd_t *)Msg)->RawCmd, sizeof(((SAMPLE_Raw_cmd_t *)Msg)->RawCmd));
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }
            break;

        case SAMPLE_SEND_DEV_HK_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_SendDevHk_cmd_t))) {
                SAMPLE_ReportDeviceHousekeeping();
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }
            break;

        case SAMPLE_SEND_DEV_DATA_CC:
            if (SAMPLE_VerifyCmdLength(Msg, sizeof(SAMPLE_SendDevData_cmd_t))) {
                SAMPLE_ReportDeviceSampleData();
            } else {
                SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
            break;
    }

    return;

} /* End of SAMPLE_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SAMPLE_ReportHousekeeping                                          */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
static int32 SAMPLE_ReportHousekeeping(const CFE_SB_CmdHdr_t *Msg)
{
    SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandCounter++;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(&SAMPLE_AppData.HkBuf.MsgHdr);
    CFE_SB_SendMsg(&SAMPLE_AppData.HkBuf.MsgHdr);

    return CFE_SUCCESS;

} /* End of SAMPLE_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Name:  SAMPLE_Noop                                                         */
/*                                                                            */
/* Purpose:                                                                   */
/*        SAMPLE NOOP command                                                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static int32 SAMPLE_Noop(const SAMPLE_Noop_t *Msg)
{

    CFE_EVS_SendEvent(SAMPLE_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command");

    return CFE_SUCCESS;

} /* End of SAMPLE_Noop */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SAMPLE_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
static int32 SAMPLE_ResetCounters(const SAMPLE_ResetCounters_t *Msg)
{

    SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandCounter = 0;
    SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter = 0;

    CFE_EVS_SendEvent(SAMPLE_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");

    return CFE_SUCCESS;

} /* End of SAMPLE_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Name:  SAMPLE_VerifyCmdLength()                                            */
/*                                                                            */
/* Purpose:                                                                   */
/*        Verify command packet length                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
static bool SAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t Msg, uint16 ExpectedLength)
{
    bool result = true;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(Msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId(Msg);
        uint16         CommandCode = CFE_SB_GetCmdCode(Msg);

        CFE_EVS_SendEvent(SAMPLE_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
                          (unsigned int)CFE_SB_MsgIdToValue(MessageID), CommandCode, ActualLength, ExpectedLength);

        result = false;

        SAMPLE_AppData.HkBuf.HkTlm.Payload.CommandErrorCounter++;
    }

    return (result);

} /* End of SAMPLE_VerifyCmdLength() */
