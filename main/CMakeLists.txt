# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.
if(DEFINED USE_DENDO_STEPPER)

idf_component_register(
	SRCS "SpeedUpdateHandler.cpp" "main.cpp" "stepper.cpp" "state.cpp" switches.cpp
	INCLUDE_DIRS "."
	REQUIRES DendoStepper esp_event ) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)
else()
idf_component_register(
	SRCS "SpeedUpdateHandler.cpp" "main.cpp" "stepper.cpp" "state.cpp" switches.cpp
	INCLUDE_DIRS "."
	REQUIRES esp_event FastAccelStepper arduino) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)

endif()
#idf_component_register(SRCS "steppertester.cpp" INCLUDE_DIRS "." REQUIRES FastAccelStepper)

