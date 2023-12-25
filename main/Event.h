#pragma once
#include <esp_event.h>]
#include "EventTypes.h"

class EventBase
{
  protected:
	EventBase()
	{
		if (!myEventLoopInstalled)
		{
			ESP_ERROR_CHECK(esp_event_loop_create_default());
		}
	};
	static bool myEventLoopInstalled;

  public:
};

class EventPublisher : public EventBase
{
  public:
	EventPublisher() : EventBase()
	{}
	virtual ~EventPublisher(){};
	virtual void PublishEvent(esp_event_base_t aBase, Event event, EventData *eventData) = 0;
};

class EventHandler : public EventBase
{
  protected:
	EventHandler(esp_event_base_t aBase): EventBase()
	{
		myBase = aBase;
	}
	virtual void RegisterEvent(Event event, esp_event_handler_t callback)
	{
		ESP_ERROR_CHECK(esp_event_handler_instance_register(myBase, (int32_t)event, callback, this, nullptr));
	};

	esp_event_base_t myBase;
};