/**
* C source file for controlling 4 DC motors with raspberry pi 2
* and Adafruit DC and Stepper Motor Head with PCA9685 PWM
* 16-channel, 12-bit PWM Fm+ I2C-bus DC/Servo Driver
*
* @Author: Stefan Raunig
* @Date:   21.12.2016
*
* Based on Adafruit python libary https://github.com/adafruit/Adafruit-Motor-HAT-Python-Library
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <wiringPiI2C.h>

#include "MotorHatDriver.h"

#ifndef    VERBOSE
   #define VERBOSE
#endif


/*file descriptor for i2c device*/
int fd = 0;

/*motors*/
struct motor_type m1;
struct motor_type m2;
struct motor_type m3;
struct motor_type m4;


/*read unsigned 8 bit from i2c device*/
uint8_t read8( uint8_t addr ) {

   uint8_t val = 0;

   val = wiringPiI2CReadReg8( fd, addr );

#ifdef VERBOSE
   printf( "READ8 FD: %d Value: %d\n", fd, val );
#endif

   return val;
}

/*write usigned 8 bit into i2c device reg*/
void write8( uint8_t addr, uint8_t d ) {

   uint8_t val = 0;

   val = wiringPiI2CWriteReg8( fd, addr, d );

#ifdef VERBOSE
   printf( "WRITE8 FD: %d, regiser: %d, value: %d, return: %d\n", fd, addr, d, val );
#endif
}

/*set directional pins forward or backward*/
void setDirectionalPin( struct motor_type motor, uint8_t direction ){

   uint16_t ond1  = 0;
   uint16_t ond2  = 0;
   uint16_t offd1 = 0;
   uint16_t offd2 = 0;

   switch( direction ){
     case FORWARD:
       ond1  = 4096;
       ond2  =    0;
       offd1 =    0;
       offd2 = 4096;
       break;
     case BACKWARD:
       ond1  =    0;
       ond2  = 4096;
       offd1 = 4096;
       offd2 =    0;
       break;
     case RELEASE:
       ond1  =    0;
       ond2  =    0;
       offd1 =    0;
       offd2 =    0;
       break;
     default: ond1 = 0; ond2 = 0; offd1 = 0; offd2 = 0; break;
   }

   write8( LED0_ON_L  + 4*motor.pin1,  ond1 & 0xFF );
   write8( LED0_ON_H  + 4*motor.pin1,  ond1 >> 8   );
   write8( LED0_OFF_L + 4*motor.pin1, offd1 & 0xFF );
   write8( LED0_OFF_H + 4*motor.pin1, offd1 >> 8   );

   write8( LED0_ON_L  + 4*motor.pin2,  ond2 & 0xFF );
   write8( LED0_ON_H  + 4*motor.pin2,  ond2 >> 8   );
   write8( LED0_OFF_L + 4*motor.pin2, offd2 & 0xFF );
   write8( LED0_OFF_H + 4*motor.pin2, offd2 >> 8   );
}

/*set PWM pin with speed between 0 and 255 */
void setPWMSpeed( struct motor_type motor, uint8_t speed ){

   if(      speed > 255 ) speed = 255;
   else if( speed <   0 ) speed =   0;

   /*speed modulation 0-255*/
   write8( LED0_ON_L  + 4*motor.pin_pwm,          0 & 0xFF );
   write8( LED0_ON_H  + 4*motor.pin_pwm,          0 >> 8   );
   write8( LED0_OFF_L + 4*motor.pin_pwm, (speed*16) & 0xFF );
   write8( LED0_OFF_H + 4*motor.pin_pwm, (speed*16) >> 8   );
}


/**
* Setup for the motors to be used
*
* @return 0 OK, -1 WiringPI error
*/
int MotorDriverInit( void ){

int i  = 0;

float freq        =     1600;
float prescaleval = 25000000; /*25MHz*/

/*set motor*/
m1.pin1    = 10;
m1.pin2    =  9;
m1.pin_pwm =  8;

m2.pin1    = 11;
m2.pin2    = 12;
m2.pin_pwm = 13;

m3.pin1    =  4;
m3.pin2    =  3;
m3.pin_pwm =  2;

m4.pin1    =  5;
m4.pin2    =  6;
m4.pin_pwm =  7;

/*setup wiring Pi for i2c communication*/
if( (fd = wiringPiI2CSetup( I2C_DEVICE )) == -1 ){

   fprintf( stderr, "WiringPI I2C setup failed.\n" );
   return -1; /* No connection to i2c device */
}
  /*reset*/
  write8( PCA9685_MODE1, 0x0 );

  /*set frequency*/
  freq        *= 0.9;   /* Correct for overshoot in the frequency setting (see issue #11).*/
  prescaleval /= 4096;  /* 12-bit */
  prescaleval /= freq;
  prescaleval -=    1;

  uint8_t prescale = floor( prescaleval + 0.5 );

#ifdef VERBOSE
  printf( "Prescale: %d\n", prescale );
#endif

  uint8_t oldmode = read8( PCA9685_MODE1 );

#ifdef VERBOSE
  printf( "Oldmode %d\n", oldmode );
#endif

  uint8_t newmode = (oldmode & 0x7F) | 0x10;   // sleep

  write8( PCA9685_MODE1,    newmode         ); // go to sleep
  write8( PCA9685_PRESCALE, floor(prescale) ); // set the prescaler
  write8( PCA9685_MODE1,    oldmode         );

  sleep( 1 );

  write8( PCA9685_MODE1, oldmode | 0xa1 );  //  This sets the MODE1 register to turn on auto increment.

  /*set release state to all motors*/
  setDirectionalPin(  m1, RELEASE  );
  setDirectionalPin(  m2, RELEASE  );
  setDirectionalPin(  m3, RELEASE  );
  setDirectionalPin(  m4, RELEASE  );

  return 0;
}
