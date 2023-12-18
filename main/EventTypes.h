#pragma once

enum class Event
{
	LeftPressed,
	LeftReleased,
	RightPressed,
	RightReleased,
	RapidPressed,
	RapidReleased,
	UpdateSpeed,
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
	UIEventData(uint8_t aSpeed = 0) 
	{
		mySpeed = aSpeed;
	}
	uint8_t mySpeed;
};

class UpdateSpeedEventData : public EventData
{
  public:
	UpdateSpeedEventData(){};
	UpdateSpeedEventData(int16_t aNormalSpeed, int16_t aRapidSpeed)
		: myNormalSpeed(aNormalSpeed),
		  myRapidSpeed(aRapidSpeed){};
	int16_t myNormalSpeed;
	int16_t myRapidSpeed;

	// Copy constructor
	UpdateSpeedEventData(const UpdateSpeedEventData &other)
		: myNormalSpeed(other.myNormalSpeed),
		  myRapidSpeed(other.myRapidSpeed){};

	// Copy assignment operator
	UpdateSpeedEventData &operator=(const UpdateSpeedEventData &other)
	{
		if (this != &other)
		{
			myNormalSpeed = other.myNormalSpeed;
			myRapidSpeed = other.myRapidSpeed;
		}
		return *this;
	}

	// Destructor
	~UpdateSpeedEventData() override {}
};