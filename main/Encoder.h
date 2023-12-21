#pragma once
#include <driver/gpio.h>
#include <esp_event.h>
#include <memory>

class RotaryEncoder {
public:
  RotaryEncoder(gpio_num_t anAPin,
				gpio_num_t aBPin,
				gpio_num_t aButtonPin,
				std::shared_ptr<esp_event_loop_handle_t> myEventLoop);
	~RotaryEncoder();
	void begin();
	int getCount();
	void resetCount();
  
private:
	static void UpdateTask(void *pvParameters);
	void Update();
	void pause();
	void resume();
	int count;
	gpio_num_t myEncAPin;
	gpio_num_t myEncBPin;
	int16_t myPrevCount;
	int16_t myCount;
	std::shared_ptr<esp_event_loop_handle_t> myEventLoop;
};
