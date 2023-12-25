
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
#include "esp_adc_cal.h"
#include "EventTypes.h"
#include "Event.h"

class SpeedUpdateHandler : public EventPublisher {
    public:
	  SpeedUpdateHandler(adc1_channel_t speedPin, std::shared_ptr<esp_event_loop_handle_t> anEventLoop, uint32_t maxDriverFreq);
        uint32_t GetNormalSpeed();
        uint32_t GetRapidSpeed();
        static void UpdateTask(void* params);
        void UpdateSpeeds();
		
	    void Start();

    private:
        uint32_t myMaxDriverFreq;
        TaskHandle_t updateTaskHandle;
		std::shared_ptr<esp_event_loop_handle_t> myEventLoop;
        adc1_channel_t speedPin;
        std::atomic<uint32_t> setSpeedADC;
        std::atomic<uint32_t> rapidSpeedADC;
        /*******Averaging filter stuff********/
        #define WINDOW_SIZE 8
        uint32_t INDEX = 0;
        uint32_t VALUE = 0;
        uint32_t SUM = 0;
        uint32_t READINGS[WINDOW_SIZE];
        uint32_t AVERAGED = 0;
		esp_adc_cal_characteristics_t* myADC1Calibration;
        /*************************************/
};

#endif // SPEEDUPDATEHANDLER_H
