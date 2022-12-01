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
    {SAMPLE_COMMAND_ERR_EID,        0x0000},
    {SAMPLE_COMMANDNOP_INF_EID,     0x0000},
    {SAMPLE_COMMANDRST_INF_EID,     0x0000},
    {SAMPLE_COMMANDRAW_INF_EID,     0x0000},
    {SAMPLE_COMMANDCONFIG_INF_EID,  0x0000},
    {SAMPLE_COMMANDCONFIG_ERR_EID,  0x0000},
    {SAMPLE_COMMANDREQDATA_INF_EID, 0x0000},
    {SAMPLE_COMMANDREQDATA_ERR_EID, 0x0000},
    // TODO: Add additional event IDs (EID) to the table as created
};


/*
** Application entry point and main process loop
*/
void SAMPLE_AppMain(void)
{
    int32 status = OS_SUCCESS;

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
        SAMPLE_AppData.RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** SAMPLE App Main Loop
    */
    while (CFE_ES_RunLoop(&SAMPLE_AppData.RunStatus) == TRUE)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(SAMPLE_PERF_ID);

        /* 
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
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
        ** Note that a SB read error is not always going to result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            SAMPLE_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(SAMPLE_PIPE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: SB Pipe Read Error = %d", (int) status);
            SAMPLE_AppData.RunStatus = CFE_ES_APP_ERROR;
        }
    }

    /*
    ** Disable component, which cleans up the interface, upon exit
    */
    SAMPLE_Disable();

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(SAMPLE_PERF_ID);

    /*
    ** Exit the Application
    */
    CFE_ES_ExitApp(SAMPLE_AppData.RunStatus);
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
    ** Subscribe to ground commands
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
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(SAMPLE_REQ_HK_MID, SAMPLE_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_SUB_REQ_HK_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X",
            SAMPLE_REQ_HK_MID, (unsigned int) status);
        return status;
    }

    /*
    ** TODO: Subscribe to any other messages here
    */


    /* 
    ** Initialize the published HK message - this HK message will contain the 
    ** telemetry that has been defined in the SAMPLE_HkTelemetryPkt for this app.
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.HkTelemetryPkt,
                   SAMPLE_HK_TLM_MID,
                   SAMPLE_HK_TLM_LNGTH, TRUE);

    /*
    ** Initialize the device packet message
    ** This packet is specific to your application
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.DevicePkt,
                   SAMPLE_DEVICE_TLM_MID,
                   SAMPLE_DEVICE_TLM_LNGTH, TRUE);

    /*
    ** TODO: Initialize any other messages that this app will publish
    */


    /* 
    ** Always reset all counters during application initialization 
    */
    SAMPLE_ResetCounters();

    /*
    ** Initialize application data
    ** Note that counters are excluded as they were reset in the previous code block
    */
    SAMPLE_AppData.HkTelemetryPkt.DeviceEnabled = SAMPLE_DEVICE_DISABLED;
    SAMPLE_AppData.HkTelemetryPkt.DeviceHK.DeviceCounter = 0;
    SAMPLE_AppData.HkTelemetryPkt.DeviceHK.DeviceConfig = 0;
    SAMPLE_AppData.HkTelemetryPkt.DeviceHK.DeviceStatus = 0;

    /*
    ** Initialize hardware interface data
    ** TODO: Make specific to your application depending on protocol in use
    ** Note that other components provide examples for the different protocols available
    */ 
    SAMPLE_AppData.SampleUart.deviceString = SAMPLE_CFG_STRING;
    SAMPLE_AppData.SampleUart.handle = SAMPLE_CFG_HANDLE;
    SAMPLE_AppData.SampleUart.isOpen = PORT_CLOSED;
    SAMPLE_AppData.SampleUart.baud = SAMPLE_CFG_BAUDRATE_HZ;

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
** Process packets received on the SAMPLE command pipe
*/
void SAMPLE_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);
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
        */
        case SAMPLE_REQ_HK_MID:
            SAMPLE_ProcessTelemetryRequest();
            break;

        /*
        ** All other invalid messages that this app doesn't recognize, 
        ** increment the command error counter and log as an error event.  
        */
        default:
            SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID,CFE_EVS_ERROR, "SAMPLE: invalid command packet, MID = 0x%x", MsgId);
            break;
    }
    return;
} 


/*
** Process ground commands
** TODO: Add additional commands required by the specific component
*/
void SAMPLE_ProcessGroundCommand(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);   

    /*
    ** Ground Commands, by definition, have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    uint16 CommandCode = CFE_SB_GetCmdCode(SAMPLE_AppData.MsgPtr);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case SAMPLE_NOOP_CC:
            /*
            ** First, verify the command length immediately after CC identification 
            ** Note that VerifyCmdLength handles the command and command error counters
            */
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                /* Second, send EVS event on successful receipt ground commands*/
                CFE_EVS_SendEvent(SAMPLE_COMMANDNOP_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: NOOP command received");
                /* Third, do the desired command action if applicable, in the case of NOOP it is no operation */
            }
            break;

        /*
        ** Reset Counters Command
        */
        case SAMPLE_RESET_COUNTERS_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDRST_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: RESET counters command received");
                SAMPLE_ResetCounters();
            }
            break;

        /*
        ** Enable Command
        */
        case SAMPLE_ENABLE_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDRST_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Enable command received");
                SAMPLE_Enable();
            }
            break;

        /*
        ** Disable Command
        */
        case SAMPLE_DISABLE_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDRST_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Disable command received");
                SAMPLE_Disable();
            }
            break;

        /*
        ** TODO: Edit and add more command codes as appropriate for the application
        ** Note that this is an example of a command that has additional arguments
        */
        case SAMPLE_CONFIG_CC:
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_Config_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(SAMPLE_COMMANDCONFIG_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Configuration command received");
                status = SAMPLE_Configuration(SAMPLE_AppData.SampleUart.handle, ((SAMPLE_Config_cmd_t*) SAMPLE_AppData.MsgPtr)->DeviceCfg);
                if (status == OS_SUCCESS)
                {
                    SAMPLE_AppData.HkTelemetryPkt.DeviceCount++;
                }
                else
                {
                    SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount++;
                }
            }
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID, CFE_EVS_ERROR, 
                "SAMPLE: invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", MsgId, CommandCode);
            break;
    }
    return;
} 


/*
** Process Telemetry Request - Triggered in response to a telemetery request
** TODO: Add additional telemetry required by the specific component
*/
void SAMPLE_ProcessTelemetryRequest(void)
{
    int32 status = OS_SUCCESS;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);   

    /* Pull this command code from the message and then process */
    uint16 CommandCode = CFE_SB_GetCmdCode(SAMPLE_AppData.MsgPtr);
    switch (CommandCode)
    {
        case SAMPLE_REQ_HK_TLM:
            SAMPLE_ReportHousekeeping();
            break;

        case SAMPLE_REQ_DATA_TLM:
            SAMPLE_ReportDeviceTelemetry();
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_HK_ERR_EID, CFE_EVS_ERROR, 
                "SAMPLE: invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", MsgId, CommandCode);
            break;
    }
    return;
}


/* 
** Report Application Housekeeping
*/
void SAMPLE_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.HkTelemetryPkt);
    return;
}


/*
** Collect and Report Device Telemetry
*/
void SAMPLE_ReportDeviceTelemetry(void)
{
    int32 status = OS_SUCCESS;
    status = SAMPLE_RequestData(SAMPLE_AppData.SampleUart.handle, (SAMPLE_Device_Data_tlm_t*) &SAMPLE_AppData.DevicePkt);
    if (status == OS_SUCCESS)
    {
        SAMPLE_AppData.HkTelemetryPkt.DeviceCount++;
        /* Publish telemetry packet */
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePkt);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.DevicePkt);
    }
    else
    {
        SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(SAMPLE_DEVICE_TLM_ERR_EID, CFE_EVS_ERROR, 
                "SAMPLE: Request device data reported error %d", status);
    }
    return;
}


/*
** Reset all global counter variables
*/
void SAMPLE_ResetCounters(void)
{
    SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    SAMPLE_AppData.HkTelemetryPkt.CommandCount = 0;
    SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount = 0;
    SAMPLE_AppData.HkTelemetryPkt.DeviceCount = 0;
    return;
} 


/*
** Enable Component
** TODO: Edit for your specific component implementation
*/
void SAMPLE_Enable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is disabled */
    if (SAMPLE_AppData.HkTelemetryPkt.DeviceEnabled == SAMPLE_DEVICE_DISABLED)
    {
        /* Open device specific protocols */
        status = uart_init_port(&SAMPLE_AppData.SampleUart);
        if (status == OS_SUCCESS)
        {
            SAMPLE_AppData.HkTelemetryPkt.DeviceCount++;
            SAMPLE_AppData.HkTelemetryPkt.DeviceEnabled = SAMPLE_DEVICE_ENABLED;
            CFE_EVS_SendEvent(SAMPLE_ENABLE_INF_EID, CFE_EVS_ERROR, "SAMPLE: Device enabled");
        }
        else
        {
            SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_UART_INIT_ERR_EID, CFE_EVS_ERROR, "SAMPLE: UART port initialization error %d", status);
        }
    }
    else
    {
        SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(SAMPLE_ENABLE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Device enable failed, already enabled");
    }
    return;
}


/*
** Disable Component
** TODO: Edit for your specific component implementation
*/
void SAMPLE_Disable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (SAMPLE_AppData.HkTelemetryPkt.DeviceEnabled == SAMPLE_DEVICE_ENABLED)
    {
        /* Open device specific protocols */
        status = uart_close_port(SAMPLE_AppData.SampleUart.handle);
        if (status == OS_SUCCESS)
        {
            SAMPLE_AppData.HkTelemetryPkt.DeviceCount++;
            SAMPLE_AppData.HkTelemetryPkt.DeviceEnabled = SAMPLE_DEVICE_ENABLED;
            CFE_EVS_SendEvent(SAMPLE_DISABLE_INF_EID, CFE_EVS_ERROR, "SAMPLE: Device disabled");
        }
        else
        {
            SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_UART_CLOSE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: UART port close error %d", status);
        }
    }
    else
    {
        SAMPLE_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(SAMPLE_DISABLE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Device disable failed, already disabled");
    }
    return;
}


/*
** Verify command packet length matches expected
*/
int32 SAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 expected_length)
{     
    int32 status = OS_SUCCESS;
    CFE_SB_MsgId_t msg_id = 0xFFFF;
    uint16 cmd_code = 0xFFFF;
    uint16 actual_length = CFE_SB_GetTotalMsgLength(msg);

    if (expected_length == actual_length)
    {
        /* Increment the command counter upon receipt of an invalid command */
        SAMPLE_AppData.HkTelemetryPkt.CommandCount++;
    }
    else
    {
        msg_id = CFE_SB_GetMsgId(msg);
        cmd_code = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(SAMPLE_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              msg_id, cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command */
        SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
} 
