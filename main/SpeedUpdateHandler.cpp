#include "SpeedUpdateHandler.h"
#include <driver/adc.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include "config.h"
#include <algorithm>
#include "state.h"
#include <esp_log.h>
#include "shared.h"
#include "StateMachine.h"
#include "esp_adc_cal.h"

SpeedUpdateHandler::SpeedUpdateHandler(adc1_channel_t aSpeedPin, std::shared_ptr<esp_event_loop_handle_t> anEventLoop, uint32_t maxDriverFreq) {
    speedPin = aSpeedPin;
    myMaxDriverFreq = maxDriverFreq;
	rapidSpeedADC = 0;
	myEventLoop = anEventLoop;
	setSpeedADC = 0;
	myADC1Calibration = new esp_adc_cal_characteristics_t();

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(speedPin, ADC_ATTEN_DB_11));

    esp_adc_cal_characterize(ADC_UNIT_1 , ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, myADC1Calibration);


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
	return rapidSpeedADC;
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
			uint32_t adc_reading = 0;
			for (int i = 0; i < 10; i++)
			{
				adc_reading += adc1_get_raw(speedPin);
				if (adc_reading == 0)
				{
					adc_reading = 1;
				}
			}
			adc_reading /= 10;
			VALUE = esp_adc_cal_raw_to_voltage(adc_reading, myADC1Calibration);
			
            //VALUE = adc1_get_raw(speedPin);       // Read the next sensor value
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
        if(setSpeed > (MAX_DRIVER_STEPS_PER_SECOND * 0.0005)) {
            lowBound = setSpeed - (MAX_DRIVER_STEPS_PER_SECOND * 0.0005);
        }

        uint32_t highBound = MAX_DRIVER_STEPS_PER_SECOND;
        if(setSpeed < (MAX_DRIVER_STEPS_PER_SECOND - (MAX_DRIVER_STEPS_PER_SECOND * 0.0005))) {
            highBound = setSpeed + (MAX_DRIVER_STEPS_PER_SECOND * 0.0005);
        }
            
        if(clamp(AVERAGED, lowBound, highBound) != AVERAGED) {

			rapidSpeedADC.store(AVERAGED, std::memory_order_relaxed);

			UpdateSpeedEventData *eventData = new UpdateSpeedEventData(mapValueToRange(AVERAGED, 0, 3073, 0, myMaxDriverFreq));

			ESP_ERROR_CHECK(esp_event_post_to(*myEventLoop, STATE_MACHINE_EVENT, static_cast<int32_t>(Event::UpdateRapidSpeed), eventData, sizeof(UpdateSpeedEventData), portMAX_DELAY));
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