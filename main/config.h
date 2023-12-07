#ifndef CONFIG_H
#define CONFIG_H

#include <driver/adc.h>
#include <driver/adc_types_legacy.h>

#define dirPinStepper 4
#define enablePinStepper 5
#define stepPinStepper 6
#define LEFTPIN GPIO_NUM_35
#define RIGHTPIN GPIO_NUM_38
#define RAPIDPIN GPIO_NUM_36
#define ACCELERATION 20000 //steps/s/s
#define DECELERATION 200000

const adc1_channel_t speedPin = ADC1_CHANNEL_6;  //front knob pot, GPIO7 on the S3
const double MAX_DRIVER_STEPS_PER_SECOND = 13000; // 20kHz max pulse freq in hz at 25/70 duty cycle, 13kHz at 50/50. FastAccelStepper is doing 50/50@13 :(
const int stepsPerRev = 200;


#define USE_DENDO_STEPPER 1
    //(13,000 - 0) / 20,000
    const double FULL_SPEED_ACCELERATION_LINEAR_TIME = 1000*(MAX_DRIVER_STEPS_PER_SECOND / ACCELERATION);
    const double FULL_SPEED_DECELERATION_LINEAR_TIME = 1000*(MAX_DRIVER_STEPS_PER_SECOND / DECELERATION);

    // #define ESP_LOGI(tag, format, ...) printf(format, ##__VA_ARGS__)
    // #define ESP_LOGE(tag, format, ...) printf(format, ##__VA_ARGS__)

#ifdef USE_DENDO_STEPPER

#elif USE_FASTACCELSTEPPER
const int acceleration = 20000;
const int deceleration = 20000;
#endif



#endif // SHARED_H
