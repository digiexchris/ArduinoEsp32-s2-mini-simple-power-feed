#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#define DEBUG_ABORTS 1
#define ENABLE_SSD1306 1

#define ENCODER_COUNTS_FULL_SCALE 1000 * 4 // 10 turns on a 20 turn encoder, 4 counts per detent

#define dirPinStepper 4
#define enablePinStepper 5
#define stepPinStepper 6
#define LEFTPIN GPIO_NUM_38
#define RIGHTPIN GPIO_NUM_35
#define RAPIDPIN GPIO_NUM_37
#define ENCODER_A_PIN GPIO_NUM_8
#define ENCODER_B_PIN GPIO_NUM_18
#define ENCODER_BUTTON_PIN GPIO_NUM_17
#define PIEZO_PIN GPIO_NUM_2
#define ACCELERATION 20000 // steps/s/s
#define DECELERATION 200000

#define I2C_MASTER_SCL_IO GPIO_NUM_16 /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_15 /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1	  /*!< I2C port number for master dev */
// redefined in u8g2_hal #define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

// const adc1_channel_t speedPin = ADC1_CHANNEL_6;  //front knob pot, GPIO7 on the S3

const uint16_t maxOutputRPM = 200;				  // 160 rpm max output speed
const float MAX_DRIVER_STEPS_PER_SECOND = 200000; // 13000/80 maximum rpm to give around 160 output rpm  :20kHz max pulse freq in hz at 25/70 duty cycle, 13kHz at 50/50. FastAccelStepper is doing 50/50@13 :(
const float stepsPerRev = 1600 * (73 / 18);		  // 18:73 reduction off of the leadscrew
const float maxStepsPerSecond = maxOutputRPM * stepsPerRev / 60;
const float mmPerRev = 0.25 * 25.4; // 4 tpi lead screw
const float stepsPerMm = stepsPerRev / mmPerRev;

#endif // SHARED_H
