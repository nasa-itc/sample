
# Sample - NOS3 cFS Application

This repository contains the NOS3 Sample cFS application.

## Overview
The example assumes a UART based device that.  This example can be used
with the sample simulator to show a device that is either synchronous or
asynchronous (i.e. can stream data).  The sample shows how to maintain
and report application status, how to maintain and report device status,
and how to automatically or on command send device data.  When showing
device streaming, the streaming rate can be commanded.

## Documentation
Currently support exists for the following versions:
* [cFS 6.7/6.8](https://cfs.gsfc.nasa.gov/)

# Commanding
Refer to the file `fsw/platform_inc/sample_app_msgids.h` for the Sample app message IDs
Refer to the file `fsw/src/sample_app_msg.h` for the Sample app command codes

## Versioning
We use [SemVer](http://semver.org/) for versioning. For the versions available, see the tags on this repository.
