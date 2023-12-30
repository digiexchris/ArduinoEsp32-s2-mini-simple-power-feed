#pragma once
#include <driver/gpio.h>
#include <esp_event.h>
#include <memory>

#include <rotary_encoder.h>
#include "Event.h"

class RotaryEncoder : public EventPublisher {
public:
  RotaryEncoder(gpio_num_t anAPin,
				gpio_num_t aBPin,
				gpio_num_t aButtonPin,
				uint32_t maxStepsPerSecond,
				int32_t aSavedEncoderCount = 0);
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
	int myPrevCount;
	int myCount;
	int myOffset;
	int mySavedCount;
	std::shared_ptr<esp_event_loop_handle_t> myEventLoop;
	rotary_encoder_t *myEncoder;
	uint32_t myMaxStepsPerSecond;
};
