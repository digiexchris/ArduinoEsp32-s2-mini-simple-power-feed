# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(SRCS "SpeedUpdateHandler.cpp" "main.cpp" "stepper.cpp" "state.cpp" INCLUDE_DIRS "." REQUIRES Bounce2 FastAccelStepper arduino)

#idf_component_register(SRCS "steppertester.cpp" INCLUDE_DIRS "." REQUIRES FastAccelStepper)

