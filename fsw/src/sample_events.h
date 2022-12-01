/************************************************************************
** File:
**    sample_events.h
**
** Purpose:
**  Define SAMPLE application event IDs
**
*************************************************************************/

#ifndef _SAMPLE_EVENTS_H_
#define _SAMPLE_EVENTS_H_

/* Standard app event IDs */
#define SAMPLE_RESERVED_EID              0
#define SAMPLE_STARTUP_INF_EID           1 
#define SAMPLE_INVALID_MSGID_ERR_EID     2 
#define SAMPLE_LEN_ERR_EID               3 
#define SAMPLE_PIPE_ERR_EID              4
#define SAMPLE_SUB_CMD_ERR_EID           5
#define SAMPLE_SUB_REQ_HK_ERR_EID        6
#define SAMPLE_SUB_REQ_DEVICE_ERR_EID    7

/* Hardware protocol event IDs */
#define SAMPLE_UART_INIT_ERR_EID         10
#define SAMPLE_UART_CLOSE_ERR_EID        11
#define SAMPLE_UART_READ_ERR_EID         12
#define SAMPLE_UART_WRITE_ERR_EID        13
#define SAMPLE_UART_TIMEOUT_ERR_EID      14

/* Implementation specific command event IDs */
#define SAMPLE_COMMAND_ERR_EID           20
#define SAMPLE_COMMANDNOP_INF_EID        21
#define SAMPLE_COMMANDRST_INF_EID        22
#define SAMPLE_COMMANDRAW_INF_EID        23
#define SAMPLE_COMMANDCONFIG_INF_EID     24
#define SAMPLE_COMMANDCONFIG_ERR_EID     25
#define SAMPLE_COMMANDREQDATA_INF_EID    26
#define SAMPLE_COMMANDREQDATA_ERR_EID    27

#define SAMPLE_ENABLE_INF_EID            30
#define SAMPLE_ENABLE_ERR_EID            31
#define SAMPLE_DISABLE_INF_EID           32
#define SAMPLE_DISABLE_ERR_EID           33

/* TODO: Fix these */
#define SAMPLE_HK_ERR_EID                40
#define SAMPLE_DEVICE_TLM_ERR_EID        41

#endif /* _SAMPLE_EVENTS_H_ */
