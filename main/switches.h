
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "rom/gpio.h"
#include "freertos/queue.h"

#include <memory>
#include "state.h"
#include <esp_log.h>
#include <vector>
#include <freertos/ringbuf.h>

enum class SwitchName
{
	LEFT = 0,
	RIGHT,
	RAPID
};

struct Switch
{
  public:
	Switch(gpio_num_t aSwitchPin, uint16_t aDelay, Event aPressedEvent, Event aReleasedEvent);
	gpio_num_t mySwitchPin;
	Event mySwitchPressedEvent;
	Event mySwitchReleasedEvent;
	TickType_t myLastStateChangeTime = 0;
	uint16_t myDelay;
	gpio_pull_mode_t myPullMode;
	gpio_int_type_t myIntrType;
	gpio_mode_t myMode;
	bool callbackCalled;
	bool myLastSwitchState;
};

class Debouncer
{
  public:
	static void Create(RingbufHandle_t stateRingBuf);

	static void AddSwitch(SwitchName aName, std::shared_ptr<Switch> aSwitch);

	static void Start();

  private:
    static RingbufHandle_t myStateRingBuf;

	static std::vector<std::shared_ptr<Switch>> mySwitches;

	static void IRAM_ATTR DebounceHandler(void *arg);
};

