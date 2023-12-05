#include "SpeedUpdateHandler.h"
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include "config.h"

SpeedUpdateHandler::SpeedUpdateHandler(adc1_channel_t aSpeedPin, std::shared_ptr<StateMachine> aStateMachine, uint32_t maxDriverFreq) {
    speedPin = aSpeedPin;
    myMaxDriverFreq = maxDriverFreq;
    rapidSpeed = maxDriverFreq;
    myStateMachine = aStateMachine;

    //pinMode(speedPin, INPUT);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(speedPin, ADC_ATTEN_DB_11));

    esp_adc_cal_characteristics_t adc1_chars; // Define adc1_chars variable
    esp_adc_cal_characterize(ADC_UNIT_1 , ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 0, &adc1_chars);


    //initialize the averaging filter
    for (int i = 0; i < WINDOW_SIZE; i++) {
        READINGS[i] = 0;
    }
    SUM = 0;

    //start the update task
    xTaskCreatePinnedToCore(&UpdateTask,"update speed", 4048, this, 1, &updateTaskHandle, 0);
}

uint32_t SpeedUpdateHandler::GetNormalSpeed() {
    std::lock_guard<std::mutex> lock(setSpeedMutex);
    return setSpeedADC;
}

uint32_t SpeedUpdateHandler::GetRapidSpeed() {
    std::lock_guard<std::mutex> lock(setSpeedMutex);
    return rapidSpeed;
}
void SpeedUpdateHandler::UpdateSpeeds() {
     while(true) {
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

        newSpeedMutex.lock();
        newSpeedADC = SUM / WINDOW_SIZE;
        setSpeedMutex.lock();
        //if the new speed is within a few of the current speed, don't bother updating it
        if(setSpeedADC != newSpeedADC && newSpeedADC != 0) {
            if (newSpeedADC >= setSpeedADC + 8 || newSpeedADC <= setSpeedADC - 8) {
            
                setSpeedADC = newSpeedADC;
            
                //todo myState should be a shared pointer instead of global.
                //or maybe myState should be a singleton?
                //or just a static global shared pointer?
                myStateMachine->AddUpdateSpeedEvent(
                new UpdateSpeedEventData(
                    mapAdcToSpeed(setSpeedADC, 0, 4095, 0, myMaxDriverFreq), 
                        rapidSpeed)
                );
            }
        }
        
        newSpeedMutex.unlock();
        setSpeedMutex.unlock();
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void SpeedUpdateHandler::UpdateTask(void* aHandler) {
    SpeedUpdateHandler* handler = static_cast<SpeedUpdateHandler*>(aHandler);
   handler->UpdateSpeeds();
}