# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
#set(EXTRA_COMPONENT_DIRS
  #"lib/espp/components/"
#)
#
#set(
  #COMPONENTS
  #"main esptool_py task logger encoder button event_manager"
  #CACHE STRING
  #"List of components to include"
  #)

# Add the u8g2 subdirectory (adjust the path as necessary)
# add_subdirectory(lib/u8g2)


#add_compile_definitions(USE_DENDO_STEPPER, 1)
add_compile_definitions(USE_FASTACCELSTEPPER ARDUINO_SKIP_TICK_CHECK)#CONFIG_IDF_TARGET_ESP32S3 SUPPORT_ESP32_RMT SUPPORT_SELECT_DRIVER_TYPE DRIVER_RMT SUPPORT_ESP32S3_MCPWM_PCNT)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(StepperPowerFeed)
