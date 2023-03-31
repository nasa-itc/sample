cmake_minimum_required(VERSION 2.6.4)

project (sample_checkout)

# Specify the cross compiler executables
# Typically these would be installed in a home directory or somewhere
# in /opt.  However in this example the system compiler is used.
if(DEFINED ENV{TARGET_CROSS})
  SET(CMAKE_C_COMPILER            "$ENV{TARGET_CROSS}gcc")
  SET(CMAKE_CXX_COMPILER          "$ENV{TARGET_CROSS}g++")
else(NOT DEFINED ENV{TARGET_CROSS})
  SET(CMAKE_C_COMPILER            "/usr/bin/gcc")
  SET(CMAKE_CXX_COMPILER          "/usr/bin/g++")
endif()

SET(CMAKE_C_FLAGS_INIT "-m32" CACHE STRING "C Flags required by platform")
SET(CMAKE_C_FLAGS "-m32" CACHE STRING "C Flags required by platform")

find_path(_ITC_CMAKE_MODULES_
  NAMES FindITC_Common.cmake
  PATHS ${ITC_CMAKE_MODULES}
          ${ITC_DEV_ROOT}/cmake/modules
          $ENV{ITC_DEV_ROOT}/cmake/modules
          /usr/local/cmake/modules
          /usr/cmake/modules)
if(NOT _ITC_CMAKE_MODULES_)
  message(WARNING "Unable to find ITC CMake Modules")
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${_ITC_CMAKE_MODULES_})

find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client uart)

#set(TGTNAME cpu2)
set(TGTNAME cpu1)

include(../../ComponentSettings.cmake)

include_directories("./")
include_directories("../fsw/platform_inc")
include_directories("../fsw/src")
include_directories("../../../fsw/apps/hwlib/fsw/public_inc")
include_directories("../../../fsw/apps/hwlib/sim/inc")

set(sample_checkout_src
  sample_checkout.c 
  ../fsw/src/sample_device.c
  # ../../../fsw/apps/hwlib/fsw/linux/libuart.c
  ../../../fsw/apps/hwlib/sim/src/libuart.c
  ../../../fsw/apps/hwlib/sim/src/nos_link_uart_only.c
)

set(sample_checkout_libs
  ${ITC_Common_LIBRARIES}
  ${NOSENGINE_LIBRARIES}
)

add_executable(sample_checkout ${sample_checkout_src})
target_link_libraries(sample_checkout ${sample_checkout_libs} pthread)
#set_target_properties(sample_checkout PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32 -g")
set_target_properties(sample_checkout PROPERTIES COMPILE_FLAGS "-m32 -g" LINK_FLAGS "-m32 -g")
