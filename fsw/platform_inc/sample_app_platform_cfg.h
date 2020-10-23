/************************************************************************
** File:
**  sample_app_platform_cfg.h
**
** Purpose: 
**  Define Sample App platform configuration parameters
**
** Notes:
**
**
*************************************************************************/
#ifndef _SAMPLE_APP_PLATFORM_CFG_H_
#define _SAMPLE_APP_PLATFORM_CFG_H_

#define SAMPLE_UART_BUFFER_SIZE 1024

#define SAMPLE_DEVICE_NAME       "SAMPLE_DEVICE"
/* Note: Sample child task stack and priority get defined here. Main app is specified in cfe_es_startup.scr */
#define SAMPLE_DEVICE_CHILD_STACK_SIZE  2048
#define SAMPLE_DEVICE_CHILD_PRIORITY    100
#define SAMPLE_DEVICE_MUTEX_NAME "SAMPLE_DEVICE_MUTEX"

/*
** Default Sample Configuration
** Note: In practice this configuration should be specified in the mission_defs/cpuX_device_cfg.h
**       These parameters are for the I/O to the device and can change per platform (NOS3 vs. flight vs. flatsat)
**       The definitions here will only be used if SAMPLE_CFG is not in the cpuX_device_cfg.h 
*/
#ifndef SAMPLE_CFG
    /* Note: NOS3 uart requires matching handle and bus number */
    #define SAMPLE_CFG_STRING           "usart_29"
    #define SAMPLE_CFG_HANDLE           29 
    #define SAMPLE_CFG_BAUDRATE_HZ      115200
    #define SAMPLE_CFG_MS_TIMEOUT       250
#endif

#endif /* _SAMPLE_PLATFORM_CFG_H_ */

/************************/
/*  End of File Comment */
/************************/
