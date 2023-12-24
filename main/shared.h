
#pragma once
#include <memory>
#include <esp_event.h>
#include "stepper.h"
#include "StateMachine.h"
#include "SpeedUpdateHandler.h"
#include "ui.h"
#include "Encoder.h"

class StateMachine;
class SpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<SpeedUpdateHandler> mySpeedUpdateHandler;
static DRAM_ATTR std::shared_ptr<StateMachine> myState;
static DRAM_ATTR std::shared_ptr<Stepper> myStepper;
static DRAM_ATTR std::shared_ptr<UI> myUI;
static DRAM_ATTR std::shared_ptr<RotaryEncoder> myEncoder;

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

inline uint32_t mapValueToRange(uint16_t value, uint16_t inMin, uint16_t inMax, uint16_t outMin, uint16_t outMax)
{
	return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
};

enum class SpeedUnit
{
	MMPM = 0,
	IPM
};