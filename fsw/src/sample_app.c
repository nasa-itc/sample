/*******************************************************************************
** File: sample_app.c
**
** Purpose:
**   This file contains the source code for the Sample App.
**
*******************************************************************************/

/*
**   Include Files
*/
#include "sample_app.h"

/*
**   Global Data
*/
SAMPLE_AppData_t SAMPLE_AppData;

static CFE_EVS_BinFilter_t  SAMPLE_EventFilters[] =
       {  /* Event ID    mask */
          {SAMPLE_STARTUP_INF_EID,       0x0000},
          {SAMPLE_COMMAND_ERR_EID,       0x0000},
          {SAMPLE_COMMANDNOP_INF_EID,    0x0000},
          {SAMPLE_COMMANDRST_INF_EID,    0x0000},
       };

/*
** SAMPLE_AppMain() -- Application entry point and main process loop
*/
void sample_AppMain( void )
{
    int32 status = CFE_SUCCESS;

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
        */
        status = CFE_SB_RcvMsg(&SAMPLE_AppData.MsgPtr, SAMPLE_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /* 
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution
        */
        CFE_ES_PerfLogEntry(SAMPLE_PERF_ID);

        /*
        ** If the RcvMsg() was successful, then continue to process the CommandPacket()
        ** if not successful, then 
        */
        if (status == CFE_SUCCESS)
        {
            SAMPLE_ProcessCommandPacket();
        }
        else if (status == CFE_SB_PIPE_RD_ERR)
        {
            /* This is an example of exiting on an error.
            ** Note that a SB read error is not always going to
            ** result in an app quitting.
            */
            CFE_EVS_SendEvent(SAMPLE_PIPE_ERR_EID, CFE_EVS_ERROR, "SAMPLE STF1 APP: SB Pipe Read Error = %d", (int) status);
            SAMPLE_AppData.RunStatus = CFE_ES_APP_ERROR;
        }
    }

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
** SAMPLE_AppInit() --  initialization
*/
int32 SAMPLE_AppInit(void)
{
    int32 status = CFE_SUCCESS;
    
    SAMPLE_AppData.RunStatus = CFE_ES_APP_RUN;

    /*
    ** Register the events
    */ 
    status = CFE_EVS_Register(SAMPLE_EventFilters,
                              sizeof(SAMPLE_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                              CFE_EVS_NO_FILTER);    /* as default, no filters are used */
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
        CFE_EVS_SendEvent(SAMPLE_SUB_REQ_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X",
            SAMPLE_SEND_HK_MID, (unsigned int) status);
        return status;
    }

    /*
    ** todo - subscribe to any other messages here - these are probably going to 
    **        be messages that are published from other apps that this app will
    **        need to perform a specific task
    */

    /*
    ** Initialize hardware interface
    */ 
    sample_uart = 


    /*
    ** Create data mutex
    */
    status = OS_MutSemCreate(&SAMPLE_AppData.data_mutex, SAMPLE_MUTEX_NAME, 0);
    if (status != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_MUTEX_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Create mutex error %d", status);
        return status;
    }

    /* 
    ** Create device task
    */
    status = CFE_ES_CreateChildTask(&SAMPLE_AppData.ChildTaskID,
                                    SAMPLE_DEVICE_NAME,
                                    (void *) SAMPLE_DeviceTask, 0,
                                    SAMPLE_DEVICE_STACK_SIZE,
                                    SAMPLE_DEVICE_PRIORITY, 0);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(SAMPLE_CREATE_DEVICE_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Create device child task error %d", status);
        return status;
    }

    /* on app init, always reset all counters */
    SAMPLE_ResetCounters();

    /* Initialize the published HK message - this HK message will contain the telemetry
    ** that has been defined in the SAMPLE_HkTelemetryPkt for this app
    */
    CFE_SB_InitMsg(&SAMPLE_AppData.HkTelemetryPkt,
                   SAMPLE_HK_TLM_MID,
                   SAMPLE_HK_TLM_LNGTH, TRUE);

    /*
    ** todo - initialize any other messages that this app will publish.  The cFS "way", is to 
    **        mainly use the app's HK message to push telemetry and data onto the Software Bus (SB)
    */


    /* 
     ** Important to send an information event that the app has initialized. this is
     ** useful for debugging the loading of individual apps
     */
    status = CFE_EVS_SendEvent (SAMPLE_STARTUP_INF_EID, CFE_EVS_INFORMATION,
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
**  Name:  SAMPLE_ProcessCommandPacket
**
**  Purpose:
**  This routine will process any packet that is received on the SAMPLE command pipe.       
*/
void SAMPLE_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);
    switch (MsgId)
    {
        /*
        ** Ground Commands with command codes fall under the SAMPLE_STF1_APP_CMD_MID
        ** message ID
        */
        case SAMPLE_CMD_MID:
            SAMPLE_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        ** The HK MID comes first, as it is currently the only other messages defined
        ** besides the SAMPLE_STF1_APP_CMD_MID message above
        */
        case SAMPLE_SEND_HK_MID:
            SAMPLE_ReportHousekeeping();
            break;

         /*
         ** All other invalid messages that this app doesn't recognize, increment
         ** the command error counter and log as an error event.  
         */
        default:
            SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID,CFE_EVS_ERROR, "SAMPLE: invalid command packet, MID = 0x%x", MsgId);
            break;
    }

    return;
} 


/*
** SAMPLE_ProcessGroundCommand() -- SAMPLE ground commands
*/
void SAMPLE_ProcessGroundCommand(void)
{
    /*
    ** MsgId is only needed if the command code is not recognized. See default case below 
    */
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(SAMPLE_AppData.MsgPtr);   

    /*
    ** Ground Commands, by definition, has a command code associated with them.  Pull
    ** this command code from the message and then process the action associated with
    ** the command code.
    */
    uint16 CommandCode = CFE_SB_GetCmdCode(SAMPLE_AppData.MsgPtr);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case SAMPLE_NOOP_CC:
            /* 
            ** notice the usage of the VerifyCmdLength() function call to verify that
            ** the command length is as expected.  
            */
            if (SAMPLE_VerifyCmdLength(SAMPLE_AppData.MsgPtr, sizeof(SAMPLE_NoArgsCmd_t)))
            {
                SAMPLE_AppData.HkTelemetryPkt.CommandCount++;
                CFE_EVS_SendEvent(SAMPLE_COMMANDNOP_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: NOOP command");
            }
            break;

        /*
        ** Reset Counters Command
        */
        case SAMPLE_RESET_COUNTERS_CC:
            SAMPLE_ResetCounters();
            break;

        /*
        ** todo - add more command codes as appropriate for the app
        **        
        */


        /*
        ** Invalid Command Codes
        */
        default:
            SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(SAMPLE_COMMAND_ERR_EID, CFE_EVS_ERROR, 
                "SAMPLE: invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", MsgId, CommandCode);
            break;
    }
    return;
} 

/* 
**  Name:  SAMPLE_ReportHousekeeping                                             
**                                                                            
**  Purpose:                                                                  
**         This function is triggered in response to a task telemetry request 
**         from the housekeeping task. This function will gather the Apps     
**         telemetry, packetize it and send it to the housekeeping task via   
**         the software bus                                                   
*/
void SAMPLE_ReportHousekeeping(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.data_mutex) == OS_SUCCESS)
    {
        CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.HkTelemetryPkt);
        CFE_SB_SendMsg((CFE_SB_Msg_t *) &SAMPLE_AppData.HkTelemetryPkt);

        OS_MutSemGive(SAMPLE_AppData.data_mutex);
    }
    return;
} 

/*
**  Name:  SAMPLE_ResetCounters                                               
**                                                                            
**  Purpose:                                                                  
**         This function resets all the global counter variables that are    
**         part of the task telemetry.                                        
*/
void SAMPLE_ResetCounters(void)
{
    if (OS_MutSemTake(SAMPLE_AppData.data_mutex) == OS_SUCCESS)
    {
        /* Status of commands processed by the SAMPLE App */
        SAMPLE_AppData.HkTelemetryPkt.CommandCount       = 0;
        SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount  = 0;
        CFE_EVS_SendEvent(SAMPLE_COMMANDRST_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: RESET Counters Command");

        OS_MutSemGive(SAMPLE_AppData.data_mutex);
    }
    return;
} 

/*
** SAMPLE_VerifyCmdLength() -- Verify command packet length                                                                                              
*/
int32 SAMPLE_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 expected_length)
{     
    int32  status = CFE_SUCCESS;
    CFE_SB_MsgId_t msg_id = 0xFFFF;
    uint16 cmd_code = 0xFFFF;
    uint16 actual_length = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (expected_length != actual_length)
    {
        msg_id = CFE_SB_GetMsgId(msg);
        cmd_code = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(SAMPLE_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              msg_id, cmd_code, actual_length, expected_length);

        status = SAMPLE_BAD_MSG_LENGTH_RC;
        SAMPLE_AppData.HkTelemetryPkt.CommandErrorCount++;
    }

    return status;
} 

