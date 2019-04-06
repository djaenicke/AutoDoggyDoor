/*
 * ultrasonic.c
 *
 *  Created on: Apr 6, 2019
 *      Author: Lacy
 */
#include <stdio.h>
//#include <unistd.h>
//#include <sys/time.h>
//#include <wiringPi.h>

#include "fsl_ftm.h"
#include "clock_config.h"
#include "xbee_interface.h"
#include "xbee_cbuf.h"

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/4)

#define PIN_TRIGGER 2// PTB2
#define PIN_ECHO 3 // PTB3

long getMicrotime(){
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);

  return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}



//int main(void)
{
  wiringPiSetup();

  pinMode(PIN_TRIGGER, OUTPUT);
  digitalWrite(PIN_TRIGGER, LOW);

  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIGGER, HIGH);
  usleep(10);
  digitalWrite(PIN_TRIGGER, LOW);

  int echo, previousEcho, lowHigh, highLow;
  long startTime, stopTime, difference;
  float rangeCm;
  lowHigh = highLow = echo = previousEcho = 0;
  while(lowHigh== 0 || highLow == 0) {
    previousEcho = echo;
    echo = digitalRead(PIN_ECHO);
    if(lowHigh== 0 && previousEcho == 0 && echo == 1) {
      lowHigh = 1;
      startTime = getMicrotime();
    }
    if(lowHigh== 1 && previousEcho == 1 && 0 == echo) {
      highLow = 1;
      stopTime = getMicrotime();
    }
  }
  difference = stopTime - startTime;
  rangeCm = difference / 58;
  printf("Start: %ld, stop: %ld, difference: %ld, range: %.2f cm\n", startTime, stopTime, difference, rangeCm);

  return 0;
}

