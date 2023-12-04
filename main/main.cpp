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

// Create Bounce objects for debouncing interrupts
Bounce2::Button leftDebouncer = Bounce2::Button();
Bounce2::Button rightDebouncer = Bounce2::Button();
Bounce2::Button rapidButtonDebouncer = Bounce2::Button();
Bounce2::Button leftStopButtonDebouncer = Bounce2::Button();
Bounce2::Button rightStopButtonDebouncer = Bounce2::Button();

std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
std::shared_ptr<StateMachine> myState;

void setup() {
  ESP_LOGI("main.cpp", "Setup start");

  pinMode(rightPin, INPUT_PULLDOWN);
  pinMode(rapidPin, INPUT_PULLDOWN);
  pinMode(stopLeftPin, INPUT_PULLDOWN);
  pinMode(stopRightPin, INPUT_PULLDOWN);
  pinMode(speedPin, INPUT);

  ESP_LOGI("main.cpp", "Setup pins complete");

  leftDebouncer.attach( leftPin, INPUT_PULLDOWN );
  leftDebouncer.interval(5);
  // leftDebouncer.setPressedState(HIGH);
  rightDebouncer.attach( rightPin, INPUT_PULLDOWN );
  rightDebouncer.interval(5);
  // rightDebouncer.setPressedState(HIGH);
  rapidButtonDebouncer.attach( rapidPin, INPUT_PULLDOWN );
  rapidButtonDebouncer.interval(5);
  // rapidButtonDebouncer.setPressedState(HIGH);
  leftStopButtonDebouncer.attach( stopLeftPin, INPUT_PULLDOWN );
  leftStopButtonDebouncer.interval(5);
  // leftStopButtonDebouncer.setPressedState(HIGH);
  rightStopButtonDebouncer.attach( stopRightPin, INPUT_PULLDOWN );
  rightStopButtonDebouncer.interval(5);
  // rightStopButtonDebouncer.setPressedState(HIGH);

  ESP_LOGI("main.cpp", "Setup debouncers complete");

  myState = std::make_shared<StateMachine>(dirPinStepper, enablePinStepper, stepPinStepper, rapidPin);
  mySpeedUpdateHandler = std::make_shared<SpeedUpdateHandler>(speedPin, myState, maxDriverFreq);
  ESP_LOGI("main.cpp", "Setup state machine complete");

  ESP_LOGI("main.cpp", "Setup complete");
}



void UpdateTask(void * pvParameters) {

  ESP_LOGI("main.cpp", "Update task start");

  while(true) {

    //ESP_LOGI("main.cpp", "Update task loop start");
    //Update all switches
    leftDebouncer.update();
    rightDebouncer.update();
    rapidButtonDebouncer.update();
    leftStopButtonDebouncer.update();
    rightStopButtonDebouncer.update();

    //todo these probably should go into a StateMachine queue instead.
    if(leftDebouncer.changed() && leftDebouncer.isPressed()) {
      myState->processEvent(Event::LeftPressed);
      //ESP_LOGI("main.cpp", "Left pressed");
    }

    if(leftDebouncer.changed() && !leftDebouncer.isPressed()) {
      myState->processEvent(Event::LeftReleased);
      //ESP_LOGI("main.cpp", "Left released");
    }

    if(rightDebouncer.changed() && rightDebouncer.isPressed()) {
      myState->processEvent(Event::RightPressed);
      //ESP_LOGI("main.cpp", "Right pressed");
    }

    if(rightDebouncer.changed() && !rightDebouncer.isPressed()) {
      myState->processEvent(Event::RightReleased);
      //ESP_LOGI("main.cpp", "Right released");
    }

    if(rapidButtonDebouncer.changed() && rapidButtonDebouncer.isPressed()) {
      myState->processEvent(Event::RapidPressed);
      //ESP_LOGI("main.cpp", "Rapid pressed");
    }

    if(rapidButtonDebouncer.changed() &&  !rapidButtonDebouncer.isPressed()) {
      myState->processEvent(Event::RapidReleased);
      //ESP_LOGI("main.cpp", "Rapid released");
    }
    
    //ESP_LOGI("main.cpp", "Update loop complete, sleeping");
    //30fps :D
    vTaskDelay(pdMS_TO_TICKS(33));
  }
}

TaskHandle_t loopTaskHandle = nullptr;

extern "C" void app_main()
{
  esp_log_level_set("main.cpp",ESP_LOG_ERROR);
  esp_log_level_set("state.cpp",ESP_LOG_ERROR);
  esp_log_level_set("stepper.cpp",ESP_LOG_ERROR);
  setup();

//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(&UpdateTask,"main loop", 8192*4, nullptr, 2, &loopTaskHandle, 1);
  //xTaskCreatePinnedToCore(&UpdateSpeedAverageTask,"update speed", 4048, nullptr, 1, nullptr, 0);
}