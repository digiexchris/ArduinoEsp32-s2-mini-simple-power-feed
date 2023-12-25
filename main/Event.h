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
	virtual esp_err_t PublishEvent(esp_event_base_t aBase, Event event, EventData *eventData)
	{
		return esp_event_post(aBase, static_cast<int32_t>(event), (void *)eventData, sizeof(EventData), portTICK_PERIOD_MS * 200);
	}
};

class EventHandler : public EventBase
{
  protected:
	EventHandler(): EventBase()
	{
	}
	virtual void RegisterEventHandler(esp_event_base_t aBase, Event event, esp_event_handler_t callback)
	{
		ESP_ERROR_CHECK(esp_event_handler_instance_register(aBase, (int32_t)event, callback, this, nullptr));
	};

	esp_event_base_t myBase;
};