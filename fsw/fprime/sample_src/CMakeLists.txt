####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
# UT_SOURCE_FILES: list of source files for unit tests
#
####
# include_directories("../../shared")
# include_directories("../../standalone") #device_cfg.h
# include_directories("../../../../../fsw/apps/hwlib/fsw/public_inc")
# include_directories("../platform_inc")


# set(SOURCE_FILES
#   "${CMAKE_CURRENT_LIST_DIR}/SampleSim.fpp"
#   "${CMAKE_CURRENT_LIST_DIR}/SampleSim.cpp"
#   "${CMAKE_CURRENT_LIST_DIR}/../../shared/sample_device.c"
# )


# # Uncomment and add any modules that this component depends on, else
# # they might not be available when cmake tries to build this component.

# # set(MOD_DEPS
# #     Add your dependencies here
# # )

# set(MOD_DEPS
#     ${ITC_Common_LIBRARIES}
#     ${NOSENGINE_LIBRARIES}
# )


# register_fprime_module()

include_directories("../../shared")
include_directories("../../standalone") #device_cfg.h
include_directories("../../../../../fsw/apps/hwlib/fsw/public_inc")
include_directories("../platform_inc")
# add_library(sample_device "${CMAKE_CURRENT_LIST_DIR}/../../shared/sample_device.c")
register_fprime_module(
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/SampleSim.cpp"
        # "${CMAKE_CURRENT_LIST_DIR}/../../shared/sample_device.c"
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/SampleSim.fpp"
    DEPENDS
        Fw_Types
        ${ITC_Common_LIBRARIES}
        ${NOSENGINE_LIBRARIES}
        # sample_device
)
target_sources(${FPRIME_CURRENT_MODULE} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../shared/sample_device.c")

target_include_directories(${FPRIME_CURRENT_MODULE} PRIVATE
  "../../shared"
  "../../standalone/"
  "../../../../../fsw/apps/hwlib/fsw/public_inc"
  "../platform_inc"
)
