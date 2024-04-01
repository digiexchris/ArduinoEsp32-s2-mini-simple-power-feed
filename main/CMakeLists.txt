# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

cmake_minimum_required(VERSION 3.16)

if(DEFINED USE_DENDO_STEPPER)

idf_component_register(
	SRCS "ui.cpp" "Settings.cpp" "Encoder.cpp" "Screen.cpp" "SpeedUpdateHandler.cpp" "main.cpp" "stepper.cpp" "state.cpp" switches.cpp ui.cpp
	INCLUDE_DIRS "."
	REQUIRED_IDF_TARGETS esp32-s3 esp32-s2
	REQUIRES DendoStepper esp_event ssd1306) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)
else()
idf_component_register(
	SRCS
		main.cpp
		stepper.cpp
		StateMachine.cpp
		MovementSwitches.cpp
		ui.cpp
		Event.cpp
		Screen.cpp
		Encoder.cpp
		Settings.cpp
	INCLUDE_DIRS .
	REQUIRED_IDF_TARGETS esp32s3 esp32s2
	REQUIRES
		esp_event
		FastAccelStepper
		arduino
		driver
		u8g2
		u8g2-hal-esp-idf
		rmt-ws2812
		rotary_encoder) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)

endif()
#idf_component_register(SRCS "steppertester.cpp" INCLUDE_DIRS "." REQUIRES FastAccelStepper)

