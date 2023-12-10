#include "SpeedUpdateHandler.h"
#include <driver/adc.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include "config.h"
#include <algorithm>
#include "state.h"
#include <esp_log.h>
#include <freertos/ringbuf.h>

SpeedUpdateHandler::SpeedUpdateHandler(adc1_channel_t aSpeedPin, esp_event_loop_handle_t anEventLoop, uint32_t maxDriverFreq) {
    speedPin = aSpeedPin;
    myMaxDriverFreq = maxDriverFreq;
    rapidSpeed = maxDriverFreq;
	myEventLoop = anEventLoop;

    //pinMode(speedPin, INPUT);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(speedPin, ADC_ATTEN_DB_11));

    // esp_adc_cal_characteristics_t adc1_chars; // Define adc1_chars variable
    // esp_adc_cal_characterize(ADC_UNIT_1 , ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);


    //initialize the averaging filter
    for (int i = 0; i < WINDOW_SIZE; i++) {
        READINGS[i] = 0;
    }
    SUM = 0;

    //start the update task
}

void SpeedUpdateHandler::Start() {
	//start the update task
	xTaskCreatePinnedToCore(&UpdateTask,"update speed", 4048, this, 4, &updateTaskHandle, 0);
}

uint32_t SpeedUpdateHandler::GetNormalSpeed() {
    return setSpeedADC;
}

uint32_t SpeedUpdateHandler::GetRapidSpeed() {
    return rapidSpeed;
}

//#include "esp_heap_caps.h"
//#include "esp_heap_trace.h"
//#define NUM_RECORDS 100
//static heap_trace_record_t trace_record[NUM_RECORDS]; // This buffer must be in internal RAM

void SpeedUpdateHandler::UpdateSpeeds() {
	//heap_trace_init_standalone(trace_record, NUM_RECORDS);
	//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	while (true)
	{
		//heap_trace_start(HEAP_TRACE_LEAKS);
        SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum

        try {
            VALUE = adc1_get_raw(speedPin);       // Read the next sensor value
        }
        catch (const std::exception& e) {
            ESP_LOGE("SpeedUpdateHandler.cpp", "Error reading ADC: %s", e.what());
        }
        catch (...) {
            ESP_LOGE("SpeedUpdateHandler.cpp", "Error reading ADC");
        }
        
        READINGS[INDEX] = VALUE;           // Add the newest reading to the window
        SUM = SUM + VALUE;                 // Add the newest reading to the sum
        INDEX = (INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

        AVERAGED = SUM / WINDOW_SIZE;
        //if the new speed is within a few of the current speed, don't bother updating it

        uint32_t setSpeed = setSpeedADC.load(std::memory_order_relaxed);

        uint32_t lowBound = 0;
        if(setSpeed > (MAX_DRIVER_STEPS_PER_SECOND * 0.05)) {
            lowBound = setSpeed - (MAX_DRIVER_STEPS_PER_SECOND * 0.01);
        }

        uint32_t highBound = MAX_DRIVER_STEPS_PER_SECOND;
        if(setSpeed < (MAX_DRIVER_STEPS_PER_SECOND - (MAX_DRIVER_STEPS_PER_SECOND * 0.001))) {
            highBound = setSpeed + (MAX_DRIVER_STEPS_PER_SECOND * 0.001);
        }
            
        if(std::clamp(AVERAGED, lowBound, highBound) != AVERAGED) {
        
            setSpeedADC.store(AVERAGED, std::memory_order_relaxed);

			UpdateSpeedEventData* eventData = new UpdateSpeedEventData(mapAdcToSpeed(AVERAGED, 0, 4095, 0, myMaxDriverFreq), rapidSpeed);

			ESP_ERROR_CHECK(esp_event_post_to(myEventLoop, STATE_MACHINE_EVENT, static_cast<int32_t>(Event::UpdateSpeed), &eventData, sizeof(UpdateSpeedEventData), portMAX_DELAY));

//			//auto eventData = new UpdateSpeedEventData(mapAdcToSpeed(AVERAGED, 0, 4095, 0, myMaxDriverFreq), rapidSpeed);
//			BaseType_t res = xRingbufferSendAcquire(mySpeedEventRingBuf, (void**)&eventData, sizeof(UpdateSpeedEventData), pdMS_TO_TICKS(100));
//			ASSERT_MSG(res == pdTRUE, "SpeedUpdateHandler.cpp", "Failed to aquire mem in send update speed event to queue");
//            eventData.myNormalSpeed = mapAdcToSpeed(AVERAGED, 0, 4095, 0, myMaxDriverFreq);
//			eventData.myRapidSpeed = rapidSpeed;
//			res = xRingbufferSendComplete(mySpeedEventRingBuf, &eventData);
//			ASSERT_MSG(res == pdTRUE, "SpeedUpdateHandler.cpp", "Failed to send update speed event to queue");
        }

		//heap_trace_stop();
		//heap_trace_dump();
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void SpeedUpdateHandler::UpdateTask(void* aHandler) {
    SpeedUpdateHandler* handler = static_cast<SpeedUpdateHandler*>(aHandler);
   handler->UpdateSpeeds();
}