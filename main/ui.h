#pragma once

#include <ssd1306.h>

#include "state.h"
#include "EventTypes.h"
#include <esp_event.h>
#include <memory>
#include "Screen.h"

ESP_EVENT_DEFINE_BASE(UI_EVENT);

//class SpeedEncoder
//{
//  public:
//	SpeedEncoder(gpio_num_t anAPin, gpio_num_t aBPin, gpio_num_t aButtonPin);
//	
//  private:
//};

class UI
{
  public:
	UIState myUIState;
	
	UI(gpio_num_t sdaPin, 
	   gpio_num_t sclPin, 
	   i2c_port_t i2cPort, 
	   uint i2cClkFreq, 
	   gpio_num_t anEncAPin, 
	   gpio_num_t aEncBPin, 
	   gpio_num_t aButtonPin);
	void Update();
	static void UpdateTask(void *pvParameters);
	void Start();
	static void ProcessUIEventLoopTask(void *pvParameters);
	void ProcessUIEventLoopTask();
	std::shared_ptr<esp_event_loop_handle_t> GetUiEventLoop();
	static void ProcessUIEventLoopIteration(void *aUi, esp_event_base_t base, int32_t id, void *payload);

	void ProcessUIEvent(UIEvent event, UIEventData* eventData);

  private:
	UI *myRef;
	void HandleButton();
	//SpeedEncoder* mySpeedEncoder;
	Screen* myScreen;
	std::shared_ptr<esp_event_loop_handle_t> myUIEventLoop;
};