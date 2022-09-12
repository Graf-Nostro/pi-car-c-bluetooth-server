/**
* Header file for controlling 4 DC motors with raspberry pi 2
* and Adafruit DC and Stepper Motor Head with PCA9685 PWM
* 16-channel, 12-bit PWM Fm+ I2C-bus DC/Servo Driver
*
* @Author: Stefan Raunig
* @Date:   21.12.2016
*
* Based on Adafruit python libary https://github.com/adafruit/Adafruit-Motor-HAT-Python-Library
*/

#include <stdint.h>

#define I2C_DEVICE       0x60 /*i2c device address*/

#define FORWARD             1 /*drive directions*/
#define BACKWARD            2 /*drive directions*/
#define RELEASE             3 /*motor realease state*/

#define PCA9685_MODE1    0x00
#define PCA9685_MODE2    0x01

#define PCA9685_SUBADR1  0x02
#define PCA9685_SUBADR2  0x03
#define PCA9685_SUBADR3  0x04

#define LED0_ON_L        0x06 /*registers*/
#define LED0_ON_H        0x07
#define LED0_OFF_L       0x08
#define LED0_OFF_H       0x09

#define ALLLED_ON_L      0xFA
#define ALLLED_ON_H      0xFB
#define ALLLED_OFF_L     0xFC
#define ALLLED_OFF_H     0xFD

#define PCA9685_PRESCALE 0xFE

#ifndef    VERBOSE
   //#define VERBOSE
#endif

/*define motor struct environment*/
struct motor_type {
   uint8_t pin1;
   uint8_t pin2;
   uint8_t pin_pwm;
};

extern struct motor_type m1;
extern struct motor_type m2;
extern struct motor_type m3;
extern struct motor_type m4;

/*read unsigned 8 bit from i2c device with register access*/
uint8_t read8( uint8_t addr );

/*write usigned 8 bit into i2c device register*/
void write8( uint8_t addr, uint8_t d );

/*set PWM pin with speed, speed should be between 0 and 255*/
void setPWMSpeed(       struct motor_type motor, uint8_t speed );

/*set directional pins forward or backward*/
void setDirectionalPin( struct motor_type motor, uint8_t direction );

/*setup for the motors to be used*/
int MotorDriverInit( void );
