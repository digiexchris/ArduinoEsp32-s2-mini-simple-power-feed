#pragma once

#include "esp_event.h"
#include <memory>

static std::shared_ptr<esp_event_loop_handle_t> myEventLoop;

ESP_EVENT_DECLARE_BASE(COMMAND_EVENT);
ESP_EVENT_DECLARE_BASE(SETTINGS_EVENT);
ESP_EVENT_DECLARE_BASE(STATE_TRANSITION_EVENT);

enum class Event
{
	Any = -1,
	//Commands
	MoveLeft = 0,
	StopMoveLeft,
	MoveRight,
	StopMoveRight,
	RapidSpeed,
	NormalSpeed,
	UpdateNormalSpeed,
	UpdateRapidSpeed,
	SetStopped,
	ToggleUnits,
	
	//Settings
	SetEncoderOffset,
	UpdateSettings,
	
	//States
	MovingLeft,
	MovingRight,
	Stopping,
	Stopped
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
	int32_t mySpeed;

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