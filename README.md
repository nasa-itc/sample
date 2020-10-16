
# Sample - NOS3 cFS Application

This repository contains the NOS3 Sample cFS application.

## Overview
The example assumes a UART based device that streams telemetry at a fixed rate.
A single configuration command is recognized that allows modifying the streaming rate.
The device confirms receipt of command by echoing the data back.
The latest streamed data is available upon request via a command.
All streamed data is packed into a larger telemetry packet to be sent to the ground.

## Documentation
Currently support exists for the following versions:
* [cFS 6.7/6.8](https://cfs.gsfc.nasa.gov/)

# Commanding
Refer to the file in `fsw/platform_inc/sample_msgids.h` for the Sample app message IDs
Refer to the file in `fsw/src/sampel_msg.h` for the Sample app command codes

## Versioning
We use [SemVer](http://semver.org/) for versioning. For the versions available, see the tags on this repository.
