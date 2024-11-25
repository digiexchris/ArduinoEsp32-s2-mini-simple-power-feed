
#include "driver/pcnt.h"
#include "Encoder.h"
#include "config.h"
#include <cstdint>
#include <memory>
#include <esp_event.h>
#include "EventTypes.h"
#include "shared.h"

#include <rotary_encoder.h>

RotaryEncoder::RotaryEncoder(
	gpio_num_t anAPin,
	gpio_num_t aBPin,
	gpio_num_t aButtonPin,
	uint32_t aMaxStepsPerSecond,
	int32_t aSavedEncoderCount	) :

								   myEncAPin(anAPin),
								   myEncBPin(aBPin),
								   myPrevCount(0),
								   myCount(0),
								   myEncoder(nullptr),
								   myMaxStepsPerSecond(aMaxStepsPerSecond)
																											
{
	mySavedCount = aSavedEncoderCount; 
	
	myOffset = 0;
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
	
	xTaskCreate(UpdateTask, "UpdateTask", 2048*4, this, 10, NULL);
}

void RotaryEncoder::pause()
{
	myEncoder->stop(myEncoder);
}

int RotaryEncoder::getCount()
{
	int count = myEncoder->get_counter_value(myEncoder);
	myCount = count + myOffset + mySavedCount;
	if (myCount < 0)
	{
		myOffset += myCount;
//		updateOffset = true;
		myCount = 0;
	}

	if (myCount > ENCODER_COUNTS_FULL_SCALE)
	{
		myOffset += myCount - ENCODER_COUNTS_FULL_SCALE;
//		updateOffset = true;
		myCount = ENCODER_COUNTS_FULL_SCALE;
	}
	
//	if (updateOffset)
//	{
//		UISetEncoderOffsetEventData *evt = new UISetEncoderOffsetEventData(mySavedOffset);
//
//		PublishEvent(SETTINGS_EVENT, Event::SetEncoderOffset, evt);
//	}

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
		
		int32_t delta = (myCount - myPrevCount)*2;
		myPrevCount = myCount;
		
		auto *eventData = new SingleValueEventData<int32_t>(delta);

		PublishEvent(COMMAND_EVENT, Event::UpdateSpeed, eventData);

	}
}