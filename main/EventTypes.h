#pragma once

#include "esp_event.h"
#include <memory>

static std::shared_ptr<esp_event_loop_handle_t> myEventLoop;

ESP_EVENT_DECLARE_BASE(STATE_MACHINE_EVENT);
ESP_EVENT_DECLARE_BASE(UI_QUEUE_EVENT);
ESP_EVENT_DECLARE_BASE(SETTINGS_EVENT);

enum class Event
{
	LeftPressed = 1,
	LeftReleased,
	RightPressed,
	RightReleased,
	RapidPressed,
	RapidReleased,
	UpdateNormalSpeed,
	UpdateRapidSpeed,
	SetStopped,
	ToggleUnits,
	SetEncoderOffset,
	SetSpeed,
	UpdateSettings
};


class EventData
{
  public:
	EventData(){};
	virtual ~EventData(){};
};

class UIEventData : public EventData
{
  public:
	UIEventData(uint32_t aSpeed = 0) 
	{
		mySpeed = aSpeed;
	}
	uint32_t mySpeed;
};

class UISetEncoderOffsetEventData : public EventData
{
  public:
	UISetEncoderOffsetEventData(int32_t aOffset = 0)
	{
		myEncoderOffset = aOffset;
	}
	int32_t myEncoderOffset;
};

class UpdateSpeedEventData : public EventData
{
  public:
	UpdateSpeedEventData(){};
	UpdateSpeedEventData(uint32_t aSpeed): mySpeed(aSpeed){};
	int16_t mySpeed;

	// Copy constructor
	UpdateSpeedEventData(const UpdateSpeedEventData &other)
		: mySpeed(other.mySpeed){};

	// Copy assignment operator
	UpdateSpeedEventData &operator=(const UpdateSpeedEventData &other)
	{
		if (this != &other)
		{
			mySpeed = other.mySpeed;
		}
		return *this;
	}

	// Destructor
	~UpdateSpeedEventData() override {}
};