/************************************************************************
** File:
**   $Id: sample_app_msgids.h  $
**
** Purpose:
**  Define Sample App Message IDs
**
*************************************************************************/
#ifndef _SAMPLE_MSGIDS_H_
#define _SAMPLE_MSGIDS_H_

/* 
** Commands - these can be either plain ol' messages or "ground commands" that
**            have associated command codes (cmdCodes)  
*/
#define SAMPLE_CMD_MID              0x18FA /* TODO: Change this for your app */ 

/* 
** This MID is for commands telling the app to publish its telemetry message
*/
#define SAMPLE_SEND_HK_MID          0x18FB /* TODO: Change this for your app */
#define SAMPLE_SEND_DEVICE_TLM_MID  0x18FC /* TODO: Change this for your app */

/* 
** Telemetry message IDs - these messages are meant for publishing messages
** containing telemetry from the application needs sent to other apps or
** to the ground
*/
#define SAMPLE_HK_TLM_MID           0x08FA /* TODO: Change this for your app */
#define SAMPLE_RAWIO_TLM_MID        0x08FB /* TODO: Change this for your app */
#define SAMPLE_DEVICE_TLM_MID       0x08FC /* TODO: Change this for your app */
#define SAMPLE_DEVICEPACK_TLM_MID   0x08FD /* TODO: Change this for your app */

#endif /* _SAMPLE_MSGIDS_H_ */
