/************************************************************************
** File:
**   $Id: sample_app_msgids.h  $
**
** Purpose:
**  Define Sample App Message IDs
**
** Notes:
**
*************************************************************************/
#ifndef _SAMPLE_MSGIDS_H_
#define _SAMPLE_MSGIDS_H_

/* 
** Commands - these can be either plain ol' messages or "ground commands" that
**            have associated command codes (cmdCodes)  
*/
#define SAMPLE_CMD_MID         0x1830      /* todo change this for your app */ 

/* 
** This MID is for commands telling the app to publish its telemetry message
*/
#define SAMPLE_SEND_HK_MID     0x1831      /* todo change this for your app */

/* 
** Telemetry message IDs - these messages are meant for publishing messages
** containing telemetry from the application needs sent to other apps or
** to the ground
*/
#define SAMPLE_HK_TLM_MID      0x0830      /* todo change this for your app */
#define SAMPLE_DEVICE_TLM_MID  0x0831      /* todo change this for your app */

#endif
