//Currently written for the esp32-s2 mini dev kit available on aliexpress, the one based on the esp32-mini board with two rows of pins on each side
//a 5v level shifter is recommended for the stepper driver inputs, but I have not had any issues without one
//the stepper driver is a TB6600, but any driver that accepts step/dir inputs should work
//the stepper is a 60BYGH Nema23, but any stepper should work.
//prioritize rpm over torque, since the align power feed has a relatively high gear ratio
//if you reuse most of the clutch mechanism

#include <memory>
#include "state.h"
#include <esp_log.h>
#include "config.h"
#include <soc/adc_channel.h>
#include "SpeedUpdateHandler.h"
#include "debounce.h"

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

static std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
static std::shared_ptr<StateMachine> myState;
Debouncer* leftSwitch;
Debouncer* rightSwitch;
Debouncer* rapidSwitch;

void setup() {
  ESP_LOGI("main.cpp", "Setup start");
  myState = std::make_shared<StateMachine>(dirPinStepper, enablePinStepper, stepPinStepper, MAX_DRIVER_STEPS_PER_SECOND);
  mySpeedUpdateHandler = std::make_shared<SpeedUpdateHandler>(speedPin, myState, MAX_DRIVER_STEPS_PER_SECOND);
  leftSwitch = new Debouncer(myState, LEFTPIN, 50);
  leftSwitch->setSwitchPressedEvent(Event::LeftPressed);
  leftSwitch->setSwitchReleasedEvent(Event::LeftReleased);
  // rightSwitch = new Debouncer(myState, RIGHTPIN, 50);
  // rightSwitch->setSwitchPressedEvent(Event::RightPressed);
  // rightSwitch->setSwitchReleasedEvent(Event::RightReleased);
  // rapidSwitch = new Debouncer(myState, RAPIDPIN, 50);
  // rapidSwitch->setSwitchPressedEvent(Event::RapidPressed);
  // rapidSwitch->setSwitchReleasedEvent(Event::RapidReleased);
  // ESP_LOGI("main.cpp", "Setup complete");

  leftSwitch->start();
  // rightSwitch->start();
  // rapidSwitch->start();
}

extern "C" void app_main()
{
  esp_log_level_set("main.cpp",ESP_LOG_ERROR);
  esp_log_level_set("state.cpp",ESP_LOG_INFO);
  esp_log_level_set("stepper.cpp",ESP_LOG_ERROR);
  setup();

  while(1) {
    vTaskDelay(portTICK_PERIOD_MS * 100);
    
  }

  // while(1) {
  //   vTaskDelay(portMAX_DELAY);
  // }
//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
  //xTaskCreatePinnedToCore(&UpdateTask,"main loop", 8192*4, nullptr, 2, &loopTaskHandle, 1);
  //xTaskCreatePinnedToCore(&UpdateSpeedAverageTask,"update speed", 4048, nullptr, 1, nullptr, 0);
}