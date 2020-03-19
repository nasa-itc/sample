/************************************************************************
** File:
**   $Id: sample_platform_cfg.h  $
**
** Purpose:
**  Define Sample App Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _SAMPLE_PLATFORM_CFG_H_
#define _SAMPLE_PLATFORM_CFG_H_

/*
** SAMPLE Task Definitions
*/
#define SAMPLE_DEVICE_NAME              "SAMPLE_DEVICE"
#define SAMPLE_DEVICE_STACK_SIZE        2048
#define SAMPLE_DEVICE_PRIORITY          250
#define SAMPLE_DEVICE_MS_LOOP_DELAY     10  

#define SAMPLE_HKDATA_MUTEX_NAME        "SAMPLE_HKDATA_MUTEX"
#define SAMPLE_DEVICE_MUTEX_NAME        "SAMPLE_DEVICE_MUTEX"

/*
** Default Sample Configuration
*/
#ifndef SAMPLE_CFG
    #define SAMPLE_CFG_STRING           "usart_0"
    #define SAMPLE_CFG_BAUDRATE_HZ      115200
    #define SAMPLE_CFG_MS_TIMEOUT       250
    #define SAMPLE_CFG_TLM_PACK         8
#endif

#endif /* _SAMPLE_PLATFORM_CFG_H_ */
