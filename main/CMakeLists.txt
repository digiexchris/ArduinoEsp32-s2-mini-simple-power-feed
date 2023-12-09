# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(
	SRCS "SpeedUpdateHandler.cpp" "main.cpp" "stepper.cpp" "state.cpp" switches.cpp
	INCLUDE_DIRS "."
	REQUIRES DendoStepper) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)
#idf_component_register(SRCS "steppertester.cpp" INCLUDE_DIRS "." REQUIRES FastAccelStepper)

