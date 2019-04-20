/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Application Includes */
#include <stdio.h>
#include <string.h>
#include "lock.h"
#include "assert.h"
#include "io_abstraction.h"
#include "fsl_uart.h"
#include "fsl_clock.h"

/* Macros */
#define LOCKED (0)
#define OPEN (1)
#define AUTO (0)
#define MANUAL (1)
#define FAR (0)
#define NEAR (1)
#define INSIDE (0)
#define OUTSIDE (1)
#define INCLIMATE (0)
#define GOOD (1)

/* Variables */
static uint8_t Prox_Status; /*TODO WILL COME FROM XBEE*/
static uint8_t Time_Status; /*TODO WILL COME FROM RTC*/
static uint8_t Weather_Status; /*TODO WILL COME FROM WEATHER API*/
static uint8_t Lock_Status = LOCKED;
static volatile uint8_t Lock_Method=MANUAL;
static uint8_t Dog_Status=0; /*TODO WILL COME FROM ULTRASONIC TO KNOW IF DOG IS OUT*/
static uint8_t Dog_Lock_Flag=0;
static uint8_t Dog_Status_Flag=0;

void Run_Lock_Control(void)
{
	if(Prox_Status == NEAR)
	{
		if(Time_Status == OPEN||Dog_Status == OUTSIDE)
		{
			if((Weather_Status == GOOD||Dog_Status == OUTSIDE) && (Dog_Lock_Flag == 0))
			{
				printf("Door unlocking.\n\r");
				Set_GPIO(ACTUATOR,HIGH);
				Lock_Status = OPEN;
				Dog_Lock_Flag = 1;
			}
			else if(Dog_Lock_Flag == 1)
			{
				printf("Weather Conflict.\n\r");
				Set_GPIO(ACTUATOR,LOW);
				Lock_Status = LOCKED;
				Dog_Lock_Flag = 0;
			}
		}
		else if (Dog_Lock_Flag == 1)
		{
		printf("Timing Conflict.\n\r");
		Set_GPIO(ACTUATOR,LOW);
		Lock_Status = LOCKED;
		Dog_Lock_Flag = 0;
		}
	}
	else if (Dog_Lock_Flag == 1)
	{
		printf("Proximity Conflict.\n\r");
		Set_GPIO(ACTUATOR,LOW);
		Lock_Status = LOCKED;
		Dog_Lock_Flag = 0;
	}

	if(Time_Status == OPEN && Weather_Status == GOOD)
	{
		if (Dog_Status == OUTSIDE && Dog_Status_Flag != 0)
		{
			Set_GPIO(GREEN_LED,HIGH);
			Set_GPIO(RED_LED, HIGH);
			Set_GPIO(BLUE_LED, LOW);
			printf("Dog is currently outside.\n\r");
			Dog_Status_Flag = 0;
		}
		else if (Dog_Status_Flag != 1)
		{
			Set_GPIO(GREEN_LED,LOW);
			Set_GPIO(RED_LED, HIGH);
			Set_GPIO(BLUE_LED, HIGH);
			printf("Dog is currently inside.\n\r");
			Dog_Status_Flag = 1;
		}
	}
	else if (Dog_Status_Flag != 2)
	{
		Set_GPIO(GREEN_LED,HIGH);
		Set_GPIO(RED_LED, LOW);
		Set_GPIO(BLUE_LED, HIGH);
		printf("The dog is inside due to weather conditions or time schedule.\n\r");
		Dog_Status_Flag = 2;
	}
}

void PORTC_IRQHandler(void)
{
    if (Lock_Method)
    {
        Lock_Method=AUTO;
        printf("The lock is now being controlled automatically.\n\r");
    }
    else
    {
        Lock_Method=MANUAL;
        printf("The lock is now being controlled manually.\n\r");
    }
    PORT_ClearPinsInterruptFlags(PORTC, 0xFFFFFFFF);
}

void PORTA_IRQHandler(void)
{
    if (Lock_Method == MANUAL)
    {
    	if (Lock_Status == LOCKED)
    	{
    		Set_GPIO(ACTUATOR,HIGH);
    		Lock_Status = OPEN;
    		printf("The lock method is now unlocked.\n\r");
    	}
    	else if (Lock_Status == OPEN)
    	{
    		Set_GPIO(ACTUATOR,LOW);
    		Lock_Status = LOCKED;
    		printf("The lock method is now locked.\n\r");
    	}
    }
    PORT_ClearPinsInterruptFlags(PORTA, 0xFFFFFFFF);
}
