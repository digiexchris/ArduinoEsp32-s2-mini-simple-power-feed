
#ifndef SPEEDUPDATEHANDLER_H
#define SPEEDUPDATEHANDLER_H

#include <mutex>
#include <memory>
#include <freertos/FreeRTOS.h>
#include <soc/adc_channel.h>
#include <driver/adc.h>
#include <freertos/task.h>
#include <atomic>
#include "state.h"
#include "StateMachine.h"

//#define USE_SPEED_POT 1
#define USE_SPEED_ENCODER 1

class SpeedUpdateHandler {
    public:
	    SpeedUpdateHandler(adc1_channel_t speedPin, esp_event_loop_handle_t anEventLoop, uint32_t maxDriverFreq);
        uint32_t GetNormalSpeed();
        uint32_t GetRapidSpeed();
        static void UpdateTask(void* params);
        void UpdateSpeeds();
		
	    void Start();

    private:
        uint32_t myMaxDriverFreq;
        TaskHandle_t updateTaskHandle;
        esp_event_loop_handle_t myEventLoop;
        adc1_channel_t speedPin;
        std::atomic<uint32_t> setSpeedADC;
        std::atomic<uint32_t> rapidSpeed;
        /*******Averaging filter stuff********/
        #define WINDOW_SIZE 8
        uint32_t INDEX = 0;
        uint32_t VALUE = 0;
        uint32_t SUM = 0;
        uint32_t READINGS[WINDOW_SIZE];
        uint32_t AVERAGED = 0;
        /*************************************/

        uint32_t mapAdcToSpeed (uint16_t value, uint16_t inMin, uint16_t inMax, uint16_t outMin, uint16_t outMax) {
            return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
        };
};

#endif // SPEEDUPDATEHANDLER_H
