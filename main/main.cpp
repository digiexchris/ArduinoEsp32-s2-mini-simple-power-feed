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
#include "shared.h"
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

std::shared_ptr<Switch> leftSwitch;
std::shared_ptr<Switch> rightSwitch;
std::shared_ptr<Switch> rapidSwitch;

void setup() {
	gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_IRAM);
	
	ESP_LOGI("main.cpp", "Setup start");
	myStepper = std::make_shared<Stepper>();
	myStepper->Init(dirPinStepper, enablePinStepper, stepPinStepper, MAX_DRIVER_STEPS_PER_SECOND);
	myState = std::make_shared<StateMachine>(myStepper);
	mySpeedUpdateHandler = std::make_shared<SpeedUpdateHandler>(speedPin, myState->GetEventLoop(), MAX_DRIVER_STEPS_PER_SECOND);
	Debouncer::Create(myState->GetEventLoop());
	leftSwitch = std::make_shared<Switch>(LEFTPIN, 50, Event::LeftPressed, Event::LeftReleased);
	rightSwitch = std::make_shared<Switch>(RIGHTPIN, 50, Event::RightPressed, Event::RightReleased);
	rapidSwitch = std::make_shared<Switch>(RAPIDPIN, 50, Event::RapidPressed, Event::RapidReleased);

	Debouncer::AddSwitch(SwitchName::LEFT, leftSwitch);
	Debouncer::AddSwitch(SwitchName::RIGHT, rightSwitch);
	Debouncer::AddSwitch(SwitchName::RAPID, rapidSwitch);
	
  
	ESP_LOGI("main.cpp", "Setup complete");
  
	//Start state FIRST or the queues will fill and hang
	myState->Start();
	mySpeedUpdateHandler->Start();
	Debouncer::Start();
  
	ESP_LOGI("main.cpp", "tasks started");
}

const char *stateToString(State aState)
{
  switch (aState)
  {
  case State::MovingLeft:
	  return "MovingLeft";
  case State::MovingRight:
	  return "MovingRight";
  case State::StoppingLeft:
	  return "StoppingLeft";
  case State::StoppingRight:
	  return "StoppingRight";
  case State::Stopped:
	  return "Stopped";
  default:
	  return "Unknown";
  }
}



extern "C" void app_main()
{
  esp_log_level_set("main.cpp",ESP_LOG_ERROR);
  esp_log_level_set("state.cpp",ESP_LOG_INFO);
  esp_log_level_set("stepper.cpp",ESP_LOG_ERROR);
  esp_log_level_set("esp32s3.cpu1", ESP_LOG_INFO);
  setup();

  

  //
  while(1) {
    
//	const char * state = stateToString(myState->GetState());
//    ESP_LOGI("Current State", "%s", state);
//	ESP_LOGI("Stepper State", "%s", myStepper->GetState().c_str());
//	ESP_LOGI("Current Speed", "%d", myStepper->GetCurrentSpeed());
	
	vTaskDelay(portTICK_PERIOD_MS * 1000);

  }

  // while(1) {
  //   vTaskDelay(portMAX_DELAY);
  // }
//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
  //xTaskCreatePinnedToCore(&UpdateTask,"main loop", 8192*4, nullptr, 2, &loopTaskHandle, 1);
  //xTaskCreatePinnedToCore(&UpdateSpeedAverageTask,"update speed", 4048, nullptr, 1, nullptr, 0);
}