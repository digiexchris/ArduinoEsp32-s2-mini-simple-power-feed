#pragma once

#include "esp_event.h"
#include <memory>

static std::shared_ptr<esp_event_loop_handle_t> myEventLoop;

ESP_EVENT_DECLARE_BASE(COMMAND_EVENT);
ESP_EVENT_DECLARE_BASE(SETTINGS_EVENT);
ESP_EVENT_DECLARE_BASE(STATE_TRANSITION_EVENT);
ESP_EVENT_DECLARE_BASE(UI_EVENT);

enum class Event
{
	Any = -1,
	// Commands
	MoveLeft = 0,
	StopMoveLeft,
	MoveRight,
	StopMoveRight,
	RapidSpeed,
	NormalSpeed,
	UpdateSpeed, //update a speed by a delta, based on which speed state the machine is in
	SetStopped,
	ToggleUnits,

	// Settings
	//SetEncoderOffset,
	SetSpeedUnit,
	SaveNormalSpeed,
	SaveRapidSpeed,
	
	
	//States
	MovingLeft,
	MovingRight,
	Stopping,
	Stopped
};


class EventData
{
  public:
	EventData(std::string aPublisher = "EventPublisher")
	{
		myPublisher = aPublisher;
	}
	virtual ~EventData(){};
	std::string myPublisher;
  private:
	
};

template <typename T>
class SingleValueEventData : public EventData
{
  public:
	SingleValueEventData(T aValue ) 
	{
		myValue = aValue;
	}
	T myValue;
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
	

	// Copy constructor
	UpdateSpeedEventData(const UpdateSpeedEventData &other)
		: EventData(), mySpeed(other.mySpeed){};

	// Copy assignment operator
	UpdateSpeedEventData &operator=(const UpdateSpeedEventData &other)
	{
		if (this != &other)
		{
			mySpeed = other.mySpeed;
		}
		return *this;
	}

	int32_t mySpeed;
	// Destructor
	~UpdateSpeedEventData() override {}
};