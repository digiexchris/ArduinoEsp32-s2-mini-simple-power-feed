# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
#add_compile_definitions(USE_DENDO_STEPPER, 1)
add_compile_definitions(USE_FASTACCELSTEPPER )#CONFIG_IDF_TARGET_ESP32S3 SUPPORT_ESP32_RMT SUPPORT_SELECT_DRIVER_TYPE DRIVER_RMT SUPPORT_ESP32S3_MCPWM_PCNT)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(StepperPowerFeed)
