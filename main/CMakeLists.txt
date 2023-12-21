# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.
if(DEFINED USE_DENDO_STEPPER)

idf_component_register(
	SRCS "Screen.cpp" "SpeedUpdateHandler.cpp" "main.cpp" "stepper.cpp" "state.cpp" switches.cpp ui.cpp
	INCLUDE_DIRS "."
	REQUIRED_IDF_TARGETS esp32-s3
	REQUIRES DendoStepper esp_event ssd1306) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)
else()
idf_component_register(
	SRCS
		SpeedUpdateHandler.cpp
		main.cpp
		stepper.cpp
		StateMachine.cpp
		switches.cpp
		ui.cpp
		Event.cpp
		Screen.cpp
		Encoder.cpp
	INCLUDE_DIRS .
	REQUIRED_IDF_TARGETS esp32s3
	REQUIRES esp_event FastAccelStepper arduino driver u8g2 u8g2-hal-esp-idf rmt-ws2812) #FastAccelStepper arduino)
		component_compile_options(-Wno-error=format= -Wno-format)

endif()
#idf_component_register(SRCS "steppertester.cpp" INCLUDE_DIRS "." REQUIRES FastAccelStepper)

