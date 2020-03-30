/*******************************************************************************
** File: sample_app.c
**
** Purpose:
**   This file contains the source code for the SAMPLE application.
**
*******************************************************************************/

/*
** Include Files
*/
#include "sample_app.h"


/*
** Global Data
*/
SAMPLE_AppData_t SAMPLE_AppData;

static CFE_EVS_BinFilter_t  SAMPLE_EventFilters[] =
{   /* Event ID    mask */
    {SAMPLE_RESERVED_EID,           0x0000},
    {SAMPLE_STARTUP_INF_EID,        0x0000},
    {SAMPLE_INVALID_MSGID_ERR_EID,  0x0000},
    {SAMPLE_LEN_ERR_EID,            0x0000},
    {SAMPLE_PIPE_ERR_EID,           0x0000},
    {SAMPLE_SUB_CMD_ERR_EID,        0x0000},
    {SAMPLE_SUB_REQ_HK_ERR_EID,     0x0000},
    {SAMPLE_SUB_REQ_DEVICE_ERR_EID, 0x0000},
    {SAMPLE_UART_ERR_EID,           0x0000},
    {SAMPLE_COMMAND_ERR_EID,        0x0000},
    {SAMPLE_COMMANDNOP_INF_EID,     0x0000},
    {SAMPLE_COMMANDRST_INF_EID,     0x0000},
    {SAMPLE_COMMANDRAW_INF_EID,     0x0000},
    {SAMPLE_COMMANDCONFIG_INF_EID,  0x0000},
    {SAMPLE_COMMANDCONFIG_ERR_EID,  0x0000},
    {SAMPLE_MUTEX_ERR_EID,          0x0000},
    {SAMPLE_CREATE_DEVICE_ERR_EID,  0x0000},
    {SAMPLE_DEVICE_REG_ERR_EID,     0x0000},
    {SAMPLE_DEVICE_REG_INF_EID,     0x0000},
    {SAMPLE_DEVICE_STREAM_ERR_EID,  0x0000},
    // TODO: Add additional event IDs (EID) to the table as created
};


/*
** Application entry point and main process loop
*/
void SAMPLE_AppMain(void)
{
    int32 status = OS_SUCCESS;
    uint32 local_run = CFE_ES_APP_RUN;

    /*
    ** Register the Application with Executive Services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(SAMPLE_PERF_ID);

    /* 
    ** Perform Application Initialization
    */
    status = SAMPLE_AppInit();
    if (status != CFE_SUCCESS)
    {
        SAMPLE_SetRunStatus(CFE_ES_APP_ERROR);
        local_run = CFE_ES_APP_ERROR;
    }

    /*
    ** SAMPLE App Main Loop
    */
    while (CFE_ES_RunLoop(&local_run) == TRUE)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(SAMPLE_PERF_ID);

        /* 
        ** Pend on the arrival of the next Software Bus message
        */
        status = CFE_SB_RcvMsg(&SAMPLE_AppData.MsgPtr, SAMPLE_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /* 
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(SAMPLE_PERF_ID);

        /*
        ** If the CFE_SB_RcvMsg was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        */
        if (status == CFE_SUCCESS)
        {
            SAMPLE_ProcessCommandPacket();
        }
        else if (status == CFE_SB_PIPE_RD_ERR)
        {
            /* 
            ** This is an example of exiting on an error.
            ** Note that a SB read error is not always going to result in an app quitting.
            */
            CFE_EVS_SendEvent(SAMPLE_PIPE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: SB Pipe Read Error = %d", (int) status);
            SAMPLE_SetRunStatus(CFE_ES_APP_ERROR);
        }

        /*
        ** Check run status
        */
        local_run = SAMPLE_GetRunStatus();
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(SAMPLE_PERF_ID);

    /*
    ** Exit the Application
    */
    CFE_ES_ExitApp(SAMPLE_GetRunStatus());
} 


/* 
** Initialize Application
*/
int32 SAMPLE_AppInit(void)
{
    int32 status = OS_SUCCESS;
    
    SAMPLE_AppData.RunStatus = CFE_ES_APP_RUN;

    /*
    ** Register the events
    */ 
    status = CFE_EVS_Register(SAMPLE_EventFilters,
                              sizeof(SAMPLE_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                              CFE_EVS_BINARY_FILTER);    /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SAMPLE: error registering for event services: 0x%08X\n", (unsigned int) status);
       return status;
    }

    /*
    ** Create the Software Bus command pipe 
    */
    status = CFE_SB_CreatePipe(&SAMPLE_AppData.CmdPipe, SAMPLE_PIPE_DEPTH, "SAMPLE_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_PIPE_ERR_EID, CFE_EVS_ERROR,
            "Error Creating SB Pipe,RC=0x%08X",(unsigned int) status);
       return status;
    }
    
    /*
    ** Subscribe to "ground commands"
    ** Ground commands are those commands with command codes
    */
    status = CFE_SB_Subscribe(SAMPLE_CMD_MID, SAMPLE_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_SUB_CMD_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X",
            SAMPLE_CMD_MID, (unsigned int) status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) messages
    ** HK messages request an app to send its HK telemetry
    */
    status = CFE_SB_Subscribe(SAMPLE_SEND_HK_MID, SAMPLE_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_SUB_REQ_HK_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X",
            SAMPLE_SEND_HK_MID, (unsigned int) status);
        return status;
    }

    /*
    ** Subscribe to device telemetry request messages
    */
    status = CFE_SB_Subscribe(SAMPLE_SEND_DEVICE_TLM_MID, SAMPLE_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_SUB_REQ_DEVICE_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to Device Telemetry Request, MID=0x%04X, RC=0x%08X",
            SAMPLE_SEND_DEVICE_TLM_MID, (unsigned int) status);
        return status;
    }

    /*
    ** TODO: Subscribe to any other messages here - these are probably going to 
    ** be messages that are published from other apps that this app will need 
    ** to perform a specific task.
    */

    /* 
    ** Initialize the published HK message - this HK message will contain the 
    ** telemetry that has been defined in the SAMPLE_HkTelemetryPkt for this app.
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.HkTelemetryPkt,
                   SAMPLE_HK_TLM_MID,
                   SAMPLE_HK_TLM_LNGTH, TRUE);

    /*
    ** Initialize the raw IO message - this message contains raw write and read
    ** data and lengths to enable device access via a ground command.
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.RawIO,
                   SAMPLE_RAWIO_TLM_MID,
                   SAMPLE_RAWIO_TLM_LNGTH, TRUE);

    /*
    ** Initialize the single device packet message for internal use
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.DevicePkt,
                   SAMPLE_DEVICE_TLM_MID,
                   SAMPLE_DEVICE_TLM_LNGTH, TRUE);

    /*
    ** Initialize the packed device data message
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.DevicePack,
                   SAMPLE_DEVICEPACK_TLM_MID,
                   SAMPLE_DEVICE_PACK_TLM_LNGTH, TRUE);

    /*
    ** TODO: Initialize any other messages that this app will publish.  
    ** The cFS "way", is to mainly use the app's HK message to push telemetry
    ** and data onto the Software Bus (SB).
    */

    /*
    ** Initialize hardware interface
    ** TODO: Make specific to your application
    */ 
    SAMPLE_AppData.SampleUart.deviceString = SAMPLE_CFG_STRING;
    SAMPLE_AppData.SampleUart.handle = SAMPLE_CFG_HANDLE;
    SAMPLE_AppData.SampleUart.isOpen = PORT_CLOSED;
    SAMPLE_AppData.SampleUart.baud = SAMPLE_CFG_BAUDRATE_HZ;
    status = uart_init_port(&SAMPLE_AppData.SampleUart);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_UART_ERR_EID, CFE_EVS_ERROR, "SAMPLE: UART port initialization error %d", status);
        return status;
    }

    /*
    ** Create HK data mutex
    */
    status = OS_MutSemCreate(&SAMPLE_AppData.HkDataMutex, SAMPLE_HKDATA_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_MUTEX_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Create HkDataMutex error %d", status);
        return status;
    }

    /*
    ** Create device mutex
    */
    status = OS_MutSemCreate(&SAMPLE_AppData.DeviceMutex, SAMPLE_DEVICE_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_MUTEX_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Create DeviceMutex error %d", status);
        return status;
    }

    /* 
    ** Create device task
    */
    status = CFE_ES_CreateChildTask(&SAMPLE_AppData.DeviceID,
                                    SAMPLE_DEVICE_NAME,
                                    (void *) SAMPLE_DeviceTask, 0,
                                    SAMPLE_DEVICE_STACK_SIZE,
                                    SAMPLE_DEVICE_PRIORITY, 0);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_CREATE_DEVICE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Create device task error %d", status);
        return status;
    }

    /* 
    ** Always reset all counters during application initialization 
    */
    SAMPLE_ResetCounters();

    /* 
     ** Send an information event that the app has initialized. 
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(SAMPLE_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "SAMPLE App Initialized. Version %d.%d.%d.%d",
                SAMPLE_MAJOR_VERSION,
                SAMPLE_MINOR_VERSION, 
                SAMPLE_REVISION, 
                SAMPLE_MISSION_REV);	
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("SAMPLE: error sending initialization event: 0x%08X\n", (unsigned int) status);
    }
    return status;
} 


/* 
**  Process packets received on the SAMPLE command pipe
*/
void SAMPLE_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);
    switch (MsgId)
    {
        /*
        ** Ground Commands with command codes fall under the SAMPLE_CMD_MID (Message ID)
        */
        case SAMPLE_CMD_MID:
            SAMPLE_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        ** The HK MID comes first, as it is currently the only other messages defined.
        */
        case SAMPLE_SEND_HK_MID:
            SAMPLE_ReportHousekeeping();
            break;

        /*
        ** Report latest device telemetry
        */
        case SAMPLE_SEND_DEVICE_TLM_MID:
            SAMPLE_ReportDeviceTelemetry();
            break;

         /*
         ** All other invalid messages that this app doesn't recognize, 
         ** increment the command error counter and log as an error event.  
         */
        default:
            SAMPLE_IncrementCommandErrorCount();
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID,CFE_EVS_ERROR, "SAMPLE: invalid command packet, MID = 0x%x", MsgId);
            break;
    }
    return;
} 


/*
** Process ground commands
*/
void SAMPLE_ProcessGroundCommand(void)
{
    /*
    ** MsgId is only needed if the command code is not recognized. See default case.
    */
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);   

    /*
    ** Ground Commands, by definition, have a command code associated with them.
    ** Pull this command code from the message and then process.
    */
    uint16 CommandCode = CFE_SB_GetCmdCode(SAMPLE_AppData.MsgPtr);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case SAMPLE_NOOP_CC:
            /* 
            ** Notice the usage of the VerifyCmdLength() function call
            */
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDNOP_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: NOOP command received");
                /* Increment the command count upon receipt of a valid command */
                SAMPLE_IncrementCommandCount();
            }
            break;

        /*
        ** Reset Counters Command
        */
        case SAMPLE_RESET_COUNTERS_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDRST_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: RESET counters command received");
                /* Skipped incrementing command count as the goal is to reset it */
                SAMPLE_ResetCounters();
            }
            break;

        /*
        ** Raw Input / Output Command
        */
        case SAMPLE_RAW_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_RawIO_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDRAW_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Raw IO command received");
                SAMPLE_RawIO();
                SAMPLE_IncrementCommandCount();
            }
            break;

        /*
        ** TODO: Edit and add more command codes as appropriate for the application
        */
        case SAMPLE_CONFIG_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_Config_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDCONFIG_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Configuration command received");
                SAMPLE_Configuration();
                SAMPLE_IncrementCommandCount();
            }
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID, CFE_EVS_ERROR, 
                "SAMPLE: invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", MsgId, CommandCode);
            /* Increment the error counter upon receipt of an invalid command */
            SAMPLE_IncrementCommandErrorCount();
            break;
    }
    return;
} 


/* 
** Report Housekeeping - Triggered in response to a telemetry request
*/
void SAMPLE_ReportHousekeeping(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.HkTelemetryPkt);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.HkTelemetryPkt);

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return;
}


/* 
** Report Latest Telemetry
*/
void SAMPLE_ReportDeviceTelemetry(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.DeviceMutex) == OS_SUCCESS)
    {
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePkt);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePkt);

        OS_MutSemGive(SAMPLE_AppData.DeviceMutex);
    }
    return;
}


/*
** Safe Get Run Status
*/
int32 SAMPLE_GetRunStatus(void)
{
    uint32 status;

    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        status = SAMPLE_AppData.RunStatus;

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return status;
}


/*
** Safe Set Run Status
*/
void SAMPLE_SetRunStatus(uint32 status)
{
    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        SAMPLE_AppData.RunStatus = status;

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return;
}


/*
** Safely increment command counter
*/
void SAMPLE_IncrementCommandCount(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        SAMPLE_AppData.HkTelemetryPkt.CommandCount++;

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return;
}


/*
** Safely increment command error counter
*/
void SAMPLE_IncrementCommandErrorCount(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return;
}


/*
** Reset all global counter variables
*/
void SAMPLE_ResetCounters(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.HkDataMutex) == OS_SUCCESS)
    {
        SAMPLE_AppData.HkTelemetryPkt.CommandCount       = 0;
        SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount  = 0;

        OS_MutSemGive(SAMPLE_AppData.HkDataMutex);
    }
    return;
} 


/*
** Verify command packet length matches expected
*/
int32 SAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 expected_length)
{     
    int32  status = OS_SUCCESS;
    CFE_SB_MsgId_t msg_id = 0xFFFF;
    uint16 cmd_code = 0xFFFF;
    uint16 actual_length = CFE_SB_GetTotalMsgLength(msg);

    if (expected_length != actual_length)
    {
        msg_id = CFE_SB_GetMsgId(msg);
        cmd_code = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(SAMPLE_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              msg_id, cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the error counter upon receipt of an invalid command */
        SAMPLE_IncrementCommandErrorCount();
    }
    return status;
} 
