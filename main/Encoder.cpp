
#include "driver/pcnt.h"
#include "Encoder.h"
#include "config.h"
#include <memory>
#include <esp_event.h>
#include "EventTypes.h"
#include "shared.h"

#include <rotary_encoder.h>

RotaryEncoder::RotaryEncoder(
	gpio_num_t anAPin,
	gpio_num_t aBPin,
	gpio_num_t aButtonPin,
	std::shared_ptr<esp_event_loop_handle_t> myEventLoop,
	uint32_t aMaxStepsPerSecond,
	int32_t aSavedEncoderOffset	) :

								   myEncAPin(anAPin),
								   myEncBPin(aBPin),
								   myPrevCount(0),
								   myCount(0),
								   myEventLoop(myEventLoop),
								   myEncoder(nullptr),
								   myMaxStepsPerSecond(aMaxStepsPerSecond)
																											
{
	mySavedOffset = aSavedEncoderOffset; //TODO get this from NVS
	rotary_encoder_config_t config = ROTARY_ENCODER_DEFAULT_CONFIG((rotary_encoder_dev_t)PCNT_UNIT_2, anAPin, aBPin);
	
	//stepper already installed the service, so calling this with false.
	ESP_ERROR_CHECK(rotary_encoder_new_ec11(&config, &myEncoder, false));

	// Filter out glitch (1us)
	ESP_ERROR_CHECK(myEncoder->set_glitch_filter(myEncoder, 10));
}

RotaryEncoder::~RotaryEncoder()
{
	pause();
}

void RotaryEncoder::begin()
{
	// Start encoder
	ESP_ERROR_CHECK(myEncoder->start(myEncoder));
	
	//myEncoder->set_counter_value(myEncoder, mySavedOffset);
	
	xTaskCreate(UpdateTask, "UpdateTask", 2048*4, this, 10, NULL);
}

void RotaryEncoder::pause()
{
	myEncoder->stop(myEncoder);
}

int RotaryEncoder::getCount()
{
	int count = myEncoder->get_counter_value(myEncoder);
	bool updateOffset = false;
	myCount = count - mySavedOffset;
	if (myCount < 0)
	{
		mySavedOffset += myCount;
		updateOffset = true;
		myCount = 0;
	}

	if (myCount > ENCODER_COUNTS_FULL_SCALE)
	{
		mySavedOffset += myCount - ENCODER_COUNTS_FULL_SCALE;
		updateOffset = true;
		myCount = ENCODER_COUNTS_FULL_SCALE;
	}
	
	if (updateOffset)
	{
		UISetEncoderOffsetEventData *evt = new UISetEncoderOffsetEventData(mySavedOffset);

		ESP_ERROR_CHECK(esp_event_post_to(myEventLoop, UI_QUEUE_EVENT, static_cast<int32_t>(UIEvent::SetEncoderOffset), evt, sizeof(UISetEncoderOffsetEventData), portMAX_DELAY));
	}

	return myCount;
}

void RotaryEncoder::resume()
{
	myEncoder->start(myEncoder);
}

void RotaryEncoder::UpdateTask(void *pvParameters)
{
	RotaryEncoder *encoder = (RotaryEncoder *)pvParameters;
	
	while (true)
	{
		encoder->Update();
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void RotaryEncoder::Update()
{
	myCount = getCount();
	if (myPrevCount != myCount)
	{	
		myPrevCount = myCount;
		auto stepsPerSecond = mapValueToRange(myPrevCount, 0, ENCODER_COUNTS_FULL_SCALE, 0, myMaxStepsPerSecond);
		UpdateSpeedEventData *eventData = new UpdateSpeedEventData(stepsPerSecond);

		ESP_ERROR_CHECK(esp_event_post_to(*myEventLoop, STATE_MACHINE_EVENT, static_cast<int32_t>(Event::UpdateNormalSpeed), eventData, sizeof(UpdateSpeedEventData), portMAX_DELAY));
	}
}
