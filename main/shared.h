
#pragma once
#include <memory>
#include <esp_event.h>
class StateMachine;
class SpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<StateMachine> myState;
static DRAM_ATTR std::shared_ptr<Stepper> myStepper;

#ifdef DEBUG_ABORTS
#define ASSERT(x) if(!(x)) { abort(); }
#define ASSERT_MSG(x, tag, msg) if(!(x)) { ESP_LOGE(tag, "ASSERT FAILED: %s", msg); abort(); }
#else
#define ASSERT(x) if(!(x)) { ESP_LOGE("shared.h", "ASSERT FAILED: %s", #x); }
#define ASSERT_MSG(x, tag, msg) if(!(x)) { ESP_LOGE(tag, "ASSERT FAILED: %s", msg); }
#endif

#define UNUSED(x) (void)(x);

template <class T>
const T &clamp(const T &v, const T &lo, const T &hi)
{
	return (v < lo) ? lo : (hi < v) ? hi
									: v;
}