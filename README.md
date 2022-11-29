# Sample - cFS Application

This repository contains the Sample cFS application.

## Overview
The example assumes a UART based device that streams telemetry at a fixed rate.
A single configuration command is recognized that allows modifying the streaming rate.
The device confirms receipt of command by echoing the data back.
The latest streamed data is available upon request via a command.
All streamed data is packed into a larger telemetry packet to be sent to the ground.

## Documentation
If this sample application had an ICD and/or test procedure, they would be linked here.

## Platform Support
Currently support exists for the following cFS versions:
* [cFS 6.7/6.8](https://cfs.gsfc.nasa.gov/)

Currently this application is only tested to support the Linux operating system.

## Configuration
Refer to the file [fsw/platform_inc/sample_platform_cfg.h](fsw/platform_inc/sample_platform_cfg.h) for the default
configuration settings, as well as a summary on overriding parameters in mission-specific repositories.

## Commanding
Refer to the file [fsw/platform_inc/sample_msgids.h](fsw/platform_inc/sample_msgids.h) for the Sample app message IDs

Refer to the file [fsw/src/sample_msg.h](fsw/src/sample_msg.h) for the Sample app command codes

## Versions
We use [SemVer](http://semver.org/) for versioning. For the versions available, see the tags on this repository.

v0.1.0 - Sept. 9 2021 - Initial release with version tagging.
