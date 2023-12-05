
#ifndef CONFIG_H
#define CONFIG_H

#include <driver/adc.h>
#include <soc/adc_channel.h>

#define dirPinStepper 4
#define enablePinStepper 5
#define stepPinStepper 6
#define leftPin 35
#define rightPin 38
#define rapidPin 36

const adc1_channel_t speedPin = ADC1_CHANNEL_6;  //front knob pot, GPIO7 on the S3
const int maxDriverFreq = 13000; // 20kHz max pulse freq in hz at 25/70 duty cycle, 13kHz at 50/50. FastAccelStepper is doing 50/50@13 :(
const int stepsPerRev = 200;


#endif // SHARED_H
