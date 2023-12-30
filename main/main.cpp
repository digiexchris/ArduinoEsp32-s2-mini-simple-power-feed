//Currently written for the esp32-s2 mini dev kit available on aliexpress, the one based on the esp32-mini board with two rows of pins on each side
//a 5v level shifter is recommended for the stepper driver inputs, but I have not had any issues without one
//the stepper driver is a TB6600, but any driver that accepts step/dir inputs should work
//the stepper is a 60BYGH Nema23, but any stepper should work.
//prioritize rpm over torque, since the align power feed has a relatively high gear ratio
//if you reuse most of the clutch mechanism

#include <memory>
#include "state.h"
#include "StateMachine.h"
#include <esp_log.h>
#include "config.h"
#include "shared.h"
#include <soc/adc_channel.h>
#include "RapidPot.h"
#include "MovementSwitches.h"
#include "ui.h"
#include "driver/gpio.h"
#include "Encoder.h"

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

static DRAM_ATTR std::shared_ptr<Settings> mySettings;
static DRAM_ATTR std::shared_ptr<RapidPot> mySpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<StateMachine> myState;
static DRAM_ATTR std::shared_ptr<Stepper> myStepper;
static DRAM_ATTR std::shared_ptr<UI> myUI;
static DRAM_ATTR std::shared_ptr<RotaryEncoder> myEncoder;

std::shared_ptr<Switch> leftSwitch;
std::shared_ptr<Switch> rightSwitch;
std::shared_ptr<Switch> rapidSwitch;

void setup() {
	//gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_IRAM);
	
	ESP_LOGI("main.cpp", "Setup start");
	mySettings = std::make_shared<Settings>();

	std::shared_ptr<SettingsData> savedSettings = mySettings->Get();

	myStepper = std::make_shared<Stepper>();
	myUI = std::make_shared<UI>(
		I2C_MASTER_SDA_IO, 
		I2C_MASTER_SCL_IO, 
		I2C_MASTER_NUM, 
		I2C_MASTER_FREQ_HZ, 
		ENCODER_A_PIN, 
		ENCODER_B_PIN, 
		ENCODER_BUTTON_PIN,
		savedSettings->mySpeedUnits
	);

	myStepper->Init(
		dirPinStepper, 
		enablePinStepper, 
		stepPinStepper, 
		maxStepsPerSecond
	);

	myState = std::make_shared<StateMachine>(myStepper);
	mySpeedUpdateHandler = std::make_shared<RapidPot>(speedPin, maxStepsPerSecond);
	
	myEncoder = std::make_shared<RotaryEncoder>(ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BUTTON_PIN, maxStepsPerSecond, savedSettings->myEncoderCount);
	
	MovementSwitches::Create();
	leftSwitch = std::make_shared<Switch>(LEFTPIN, 50, Event::MoveLeft, Event::StopMoveLeft);
	rightSwitch = std::make_shared<Switch>(RIGHTPIN, 50, Event::MoveRight, Event::StopMoveRight);
	rapidSwitch = std::make_shared<Switch>(RAPIDPIN, 50, Event::RapidSpeed, Event::NormalSpeed);

	MovementSwitches::AddSwitch(SwitchName::LEFT, leftSwitch);
	MovementSwitches::AddSwitch(SwitchName::RIGHT, rightSwitch);
	MovementSwitches::AddSwitch(SwitchName::RAPID, rapidSwitch);
	
  
	ESP_LOGI("main.cpp", "Setup complete");
	
	
	myUI->Start();
	//Start state FIRST or the queues will fill and hang
	myState->Start();
	mySpeedUpdateHandler->Start();
	MovementSwitches::Start();
	
	myEncoder->begin();
  
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
//   esp_log_level_set("main.cpp",ESP_LOG_ERROR);
//   esp_log_level_set("state.cpp",ESP_LOG_INFO);
//   esp_log_level_set("stepper.cpp",ESP_LOG_ERROR);
//   esp_log_level_set("esp32s3.cpu1", ESP_LOG_INFO);
  setup();

  std::string prevState = "";
  std::string prevSpeed = "";
  //
  while(1) {
    
	std::string state = stateToString(myState->GetState());
	std::string speed = std::to_string(myStepper->GetCurrentSpeed());

	if (state != prevState)
	{
		ESP_LOGI("State", "%s", state.c_str());
		prevState = state;	
	}

	if (speed != prevSpeed)
	{
		ESP_LOGI("Speed", "%s", speed.c_str());
		prevSpeed = speed;
	}
	
	vTaskDelay(portTICK_PERIOD_MS * 1000);

  }

  // while(1) {
  //   vTaskDelay(portMAX_DELAY);
  // }
//xTaskCreateUniversal(loopTask, "loopTask", getArduinoLoopTaskStackSize(), NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
  //xTaskCreatePinnedToCore(&UpdateTask,"main loop", 8192*4, nullptr, 2, &loopTaskHandle, 1);
  //xTaskCreatePinnedToCore(&UpdateSpeedAverageTask,"update speed", 4048, nullptr, 1, nullptr, 0);
}