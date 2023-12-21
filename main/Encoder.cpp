
#include "driver/pcnt.h"
#include "Encoder.h"
#include "config.h"
#include <memory>
#include <esp_event.h>
#include "EventTypes.h"
#include "shared.h"

#define PCNT_UNIT_A PCNT_UNIT_0
#define PCNT_UNIT_B PCNT_UNIT_1

RotaryEncoder::RotaryEncoder(
	gpio_num_t anAPin,
	gpio_num_t aBPin,
	gpio_num_t aButtonPin,
	std::shared_ptr<esp_event_loop_handle_t> myEventLoop) :

	myEncAPin(anAPin),
	myEncBPin(aBPin),
	myPrevCount(0),
	myCount(0),
	myEventLoop(myEventLoop)
																											
{
}

RotaryEncoder::~RotaryEncoder()
{
	pause();
}

void RotaryEncoder::begin()
{
	pcnt_config_t pcnt_config_a = {
		// Set up PCNT configuration for Channel A
		.pulse_gpio_num = myEncAPin,
		.ctrl_gpio_num = PCNT_PIN_NOT_USED,
		.pos_mode = PCNT_COUNT_INC,
		.neg_mode = PCNT_COUNT_DEC,
		.counter_h_lim = ENCODER_COUNTS_FULL_SCALE,
		.counter_l_lim = 0,
		.unit = PCNT_UNIT_A,
		.channel = PCNT_CHANNEL_0,
	};
	pcnt_unit_config(&pcnt_config_a);

	// Set up PCNT configuration for Channel B
	pcnt_config_t pcnt_config_b = {
		.pulse_gpio_num = myEncBPin,
		.ctrl_gpio_num = PCNT_PIN_NOT_USED,
		.pos_mode = PCNT_COUNT_DEC,
		.neg_mode = PCNT_COUNT_INC,
		.counter_h_lim = ENCODER_COUNTS_FULL_SCALE,
		.counter_l_lim = 0,
		.unit = PCNT_UNIT_A,
		.channel = PCNT_CHANNEL_1,
	};
	pcnt_unit_config(&pcnt_config_b);

	resetCount();
}

void RotaryEncoder::pause()
{
	pcnt_counter_pause(PCNT_UNIT_A);
	pcnt_counter_pause(PCNT_UNIT_B);
}

int RotaryEncoder::getCount()
{
	int16_t count_a, count_b;
	pcnt_get_counter_value(PCNT_UNIT_A, &count_a);
	pcnt_get_counter_value(PCNT_UNIT_B, &count_b);

	// Reset the PCNT counts after reading
	resetCount();

	// Determine the direction and update the synthesized count
	if (count_a > 0 && count_b == 0)
	{
		// Forward rotation detected
		myCount += count_a;
	}
	else if (count_a < 0 && count_b == 0)
	{
		// Reverse rotation detected
		myCount += count_a; // count_a is negative
	}

	if (myCount < 0)
	{
		myCount = 0;
	}
	else if (myCount > ENCODER_COUNTS_FULL_SCALE)
	{
		myCount = ENCODER_COUNTS_FULL_SCALE;
	}

	return myCount;
}

void RotaryEncoder::resume()
{
	pcnt_counter_resume(PCNT_UNIT_A);
	pcnt_counter_resume(PCNT_UNIT_B);
}

void RotaryEncoder::resetCount()
{
	pause();
	pcnt_counter_clear(PCNT_UNIT_A);
	pcnt_counter_clear(PCNT_UNIT_B);
	resume();
	myCount = 0;
}

void RotaryEncoder::UpdateTask(void *pvParameters)
{
	RotaryEncoder *encoder = (RotaryEncoder *)pvParameters;
	
	while (true)
	{
		encoder->Update();
		vTaskDelay(40 / portTICK_PERIOD_MS);
	}
}

void RotaryEncoder::Update()
{
	if (myPrevCount != myCount)
	{	
		myPrevCount = myCount;

		UpdateSpeedEventData *eventData = new UpdateSpeedEventData(mapValueToRange(myPrevCount, 0, ENCODER_COUNTS_FULL_SCALE, 0, MAX_DRIVER_STEPS_PER_SECOND));

		ESP_ERROR_CHECK(esp_event_post_to(*myEventLoop, STATE_MACHINE_EVENT, static_cast<int32_t>(Event::UpdateNormalSpeed), eventData, sizeof(UpdateSpeedEventData), portMAX_DELAY));
	}
}
