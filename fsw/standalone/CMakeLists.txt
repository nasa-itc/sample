cmake_minimum_required(VERSION 2.6.4)

project (sample_checkout)

if (NOT DEFINED TGTNAME)
  message(FATAL_ERROR "TGTNAME must be defined on the cmake command line (e.g. \"-DTGTNAME=cpu1\")")
endif()

include(../../../ComponentSettings.cmake)

if(${TGTNAME} STREQUAL cpu1)
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

  find_package(NOSENGINE REQUIRED QUIET COMPONENTS common transport client uart can i2c spi)
  
  # Find Simulith headers
  set(SIMULITH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../sims/simulith" CACHE PATH "Path to Simulith directory")
  if(NOT EXISTS "${SIMULITH_DIR}/include/simulith_uart.h")
      message(FATAL_ERROR "Could not find simulith_uart.h in ${SIMULITH_DIR}/include. Please set SIMULITH_DIR to the correct path.")
  endif()
endif()

include_directories("./")
include_directories("../cfs/platform_inc")
include_directories("../cfs/src")
include_directories("../shared")
include_directories("../../../../fsw/apps/hwlib/fsw/public_inc")

set(sample_checkout_src
  sample_checkout.c 
  ../shared/sample_device.c
)

if(${TGTNAME} STREQUAL cpu1)
  include_directories("${SIMULITH_DIR}/include")
  include_directories("../../../../fsw/apps/hwlib/sim/inc")
  set(sample_checkout_src 
    ${sample_checkout_src}
    ${SIMULITH_DIR}/src/simulith_uart.c
    ${SIMULITH_DIR}/src/simulith_common.c
    ../../../../fsw/apps/hwlib/sim/src/libuart.c
    ../../../../fsw/apps/hwlib/sim/src/libcan.c
    ../../../../fsw/apps/hwlib/sim/src/libi2c.c
    ../../../../fsw/apps/hwlib/sim/src/libspi.c
    ../../../../fsw/apps/hwlib/sim/src/nos_link.c
  )
  set(sample_checkout_libs
    ${ITC_Common_LIBRARIES}
    ${NOSENGINE_LIBRARIES}
    ${ZeroMQ_LIBRARIES}
    simulith
  )
endif()
if(${TGTNAME} STREQUAL cpu2)
  set(sample_checkout_src 
    ${sample_checkout_src}
    ../../../../fsw/apps/hwlib/fsw/linux/libuart.c
  )
endif()

add_executable(sample_checkout ${sample_checkout_src})
target_link_libraries(sample_checkout ${sample_checkout_libs})

if(${TGTNAME} STREQUAL cpu1)
  set_target_properties(sample_checkout PROPERTIES COMPILE_FLAGS "-g" LINK_FLAGS "-g")
endif()
