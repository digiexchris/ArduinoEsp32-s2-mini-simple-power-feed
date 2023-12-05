//Currently written for the esp32-s2 mini dev kit available on aliexpress, the one based on the esp32-mini board with two rows of pins on each side
//a 5v level shifter is recommended for the stepper driver inputs, but I have not had any issues without one
//the stepper driver is a TB6600, but any driver that accepts step/dir inputs should work
//the stepper is a 60BYGH Nema23, but any stepper should work.
//prioritize rpm over torque, since the align power feed has a relatively high gear ratio
//if you reuse most of the clutch mechanism
#include <Arduino.h>
#include <Bounce2.h> // Include the Bounce2 library for debounce

#include "FastAccelStepper.h"
#include <memory>
#include "stepper.h"
#include "state.h"
#include <esp_log.h>
#include "config.h"
#include <esp_adc_cal.h>
#include <soc/adc_channel.h>
#include "SpeedUpdateHandler.h"
#include "switches.h"

// #define dirPinStepper 4
// #define enablePinStepper 5
// #define stepPinStepper 6
// const adc1_channel_t speedPin = ADC1_GPIO7_CHANNEL;  //front knob pot
// //#define maxSpeedPin 7 //trimpot to select max speed
// #define leftPin 35
// #define rightPin 38
// #define rapidPin 36
// #define stopLeftPin 8
// #define stopRightPin 17

//TODO add stop positions to oled display

std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
std::shared_ptr<StateMachine> myState;
std::shared_ptr<Switches> mySwitches;

void setup() {
  ESP_LOGI("main.cpp", "Setup start");
  myState = std::make_shared<StateMachine>(dirPinStepper, enablePinStepper, stepPinStepper, maxDriverFreq);
  mySpeedUpdateHandler = std::make_shared<SpeedUpdateHandler>(speedPin, myState, maxDriverFreq);
  mySwitches = std::make_shared<Switches>(myState, leftPin, rightPin, rapidPin);
  ESP_LOGI("main.cpp", "Setup complete");
}

extern "C" void app_main()
{
  esp_log_level_set("main.cpp",ESP_LOG_ERROR);
  esp_log_level_set("state.cpp",ESP_LOG_ERROR);
  esp_log_level_set("stepper.cpp",ESP_LOG_ERROR);
  setup();

//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
  //xTaskCreatePinnedToCore(&UpdateTask,"main loop", 8192*4, nullptr, 2, &loopTaskHandle, 1);
  //xTaskCreatePinnedToCore(&UpdateSpeedAverageTask,"update speed", 4048, nullptr, 1, nullptr, 0);
}