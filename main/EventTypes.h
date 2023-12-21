#pragma once

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(STATE_MACHINE_EVENT);
ESP_EVENT_DECLARE_BASE(UI_QUEUE_EVENT);

enum class Event
{
	LeftPressed,
	LeftReleased,
	RightPressed,
	RightReleased,
	RapidPressed,
	RapidReleased,
	UpdateNormalSpeed,
	UpdateRapidSpeed,
	SetStopped
};

enum class UIEvent
{
	MoveLeft,
	MoveRight,
	RapidSpeed,
	NormalSpeed,
	SetSpeed,
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