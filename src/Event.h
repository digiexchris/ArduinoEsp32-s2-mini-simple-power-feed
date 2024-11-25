#pragma once
#include "EventTypes.h"
#include "shared.h"
#include <typeinfo>

using EventCallbackFn = void (*)(EventType type, int32_t id, void *event_data);

class EventBase
{
protected:
	EventBase()
	{
		if (!myEventLoopInstalled)
		{
			// TODO create event loop
			myEventLoopInstalled = true;
		}
	};
	static bool myEventLoopInstalled;

public:
};

class EventPublisher : public EventBase
{
public:
	EventPublisher() : EventBase()
	{
		// myPublisherName = DemangleTypeName(typeid(*this).name());
	}
	virtual ~EventPublisher(){};

	template <typename T>
	static bool PublishEvent(EventType type, Event event, T *eventData)
	{
		static_assert(std::is_base_of<EventData, T>::value, "T must be derived from EventData");

		return false;
		// return esp_event_post(aBase, static_cast<int32_t>(event), (void *)eventData, sizeof(T), portTICK_PERIOD_MS * 200);
	}

	static bool PublishEvent(EventType type, Event event)
	{
		return false;
		// return esp_event_post(aBase, static_cast<int32_t>(event), nullptr, sizeof(nullptr), portTICK_PERIOD_MS * 200);
	}

protected:
	static std::string myPublisherName;
};

class EventHandler : public EventBase
{
protected:
	EventHandler() : EventBase()
	{
	}
	virtual void RegisterEventHandler(EventType type, Event event, EventCallbackFn callback){
		// ESP_ERROR_CHECK(esp_event_handler_instance_register(aBase, (int32_t)event, callback, this, nullptr));
	};
};