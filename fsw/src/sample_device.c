/*******************************************************************************
** File: sample_device.c
**
** Purpose:
**   This file contains the source code for the Sample Device Task.
**
*******************************************************************************/

/*
**   Include Files
*/
#include "sample_device.h"

/*
** Sample Device Task Loop
*/
int32 Sample_DeviceTask(void)
{
  int32 status = CFE_SUCCESS;

  status = CFE_ES_RegisterChildTask();
  if(status != CFE_SUCCESS)
  {
    CFE_EVS_SendEvent(SAMPLE_DEVICE_REG_ERR_EID, CFE_EVS_ERROR, "SAMPLE: Register device task error %d", status);
    CFE_ES_ExitChildTask();
    return;
  }
  else
  {
    CFE_EVS_SendEvent(SAMPLE_DEVICE_REG_INF_EID, CFE_EVS_INFORMATION, "SAMPLE: Device task registration complete");
  }

  

  return status;
}


