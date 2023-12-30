#include "EventTypes.h"
#include "Event.h"
#include "esp_event.h"

ESP_EVENT_DEFINE_BASE(COMMAND_EVENT);
ESP_EVENT_DEFINE_BASE(SETTINGS_EVENT);
ESP_EVENT_DEFINE_BASE(STATE_TRANSITION_EVENT);

bool EventBase::myEventLoopInstalled = false;
std::string EventPublisher::myPublisherName = "EventPublisher";