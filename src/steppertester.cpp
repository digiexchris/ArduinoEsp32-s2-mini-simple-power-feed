#include "FastAccelStepper.h"

#define dirPinStepper 4
#define enablePinStepper 5
#define stepPinStepper 6
#define speedPin 7 //front knob pot
//#define maxSpeedPin 7 //trimpot to select max speed
#define leftPin 35
#define rightPin 38
#define rapidPin 36
#define stopLeftPin 8
#define stopRightPin 17

struct stepper_config_s {
  uint8_t step;
  uint8_t enable_low_active;
  uint8_t enable_high_active;
  uint8_t direction;
  uint16_t dir_change_delay;
  bool direction_high_count_up;
  bool auto_enable;
  uint32_t on_delay_us;
  uint16_t off_delay_ms;
};

extern "C" void app_main() {
    FastAccelStepperEngine engine = FastAccelStepperEngine();
    FastAccelStepper *stepper = NULL;

    engine.init();

    const struct stepper_config_s config = {
        .step = stepPinStepper,
        .enable_low_active = enablePinStepper,
        .enable_high_active = PIN_UNDEFINED,
        .direction = dirPinStepper,
        .dir_change_delay = 50,
        .direction_high_count_up = true,
        .auto_enable = true,
        .on_delay_us = 50,
        .off_delay_ms = 1000
    };

    stepper = engine.stepperConnectToPin(config.step);
    stepper->setDirectionPin(config.direction, config.direction_high_count_up, config.dir_change_delay);
    stepper->setEnablePin(config.enable_low_active, true);
    stepper->setEnablePin(config.enable_high_active, false);
    stepper->setAutoEnable(config.auto_enable);
    stepper->setDelayToEnable(config.on_delay_us);
    stepper->setDelayToDisable(config.off_delay_ms);
    // stepper->setDirectionPin(dirPinStepper);
    // stepper->setEnablePin(enablePinStepper);
    // stepper->setAutoEnable(true);
    stepper->setSpeedInHz(13000);  // the parameter is us/step !!!
    stepper->setAcceleration(20000);

    ESP_LOGI("main.cpp", "Pos Move");
        stepper->runForward();
    
    while(true) {
        ESP_LOGI("main.cpp", "Main start");
        // stepper->runBackward();
        // vTaskDelay(pdMS_TO_TICKS(1000));
        
        
        ESP_LOGI("main.cpp", "Main end");
        ESP_LOGI("main.cpp", "POS %d", stepper->getCurrentPosition());
        ESP_LOGI("main.cpp", "SPEED %d", stepper->getSpeedInMilliHz());
        ESP_LOGI("main.cpp", "ACCEL %d", stepper->getAcceleration());
        ESP_LOGI("main.cpp", "RUNNING %d", stepper->isRunning());
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}