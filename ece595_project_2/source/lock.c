/* Application Includes */
#include <stdio.h>
#include <string.h>
#include "MK64F12.h"
#include "lock.h"
#include "assert.h"
#include "io_abstraction.h"
#include "fsl_uart.h"
#include "fsl_clock.h"
#include "time_schedule.h"
#include "http_client_app.h"
#include "proximity_estimation.h"
#include "ultrasonic.h"

/* Macros */
#define LOCKED 0
#define OPEN 1

#define LOCK_THRESH 11

#define OTHER 2
#define INSIDE 1
#define OUTSIDE 0


/* Variables */
static Proximity_Status_T Prox_Status;
static Time_Schedule_Status_T Time_Status;
static Weather_Status_T Weather_Status;
static volatile uint8_t Lock_Status = LOCKED;
static volatile uint8_t Lock_Method = MANUAL;
static Dog_Status_T Dog_Status=0; /*TODO WILL COME FROM ULTRASONIC TO KNOW IF DOG IS OUT*/
static uint8_t Dog_Lock_Flag=0;
static uint8_t Dog_Status_Flag=0;
static uint8_t Lock_Counter=0;

void Run_Lock_Control(void)
{
	Time_Status = Get_Time_Schedule_Status();
	Weather_Status = Get_Weather_Status();
	Prox_Status = Get_Proximity_Status();
	Dog_Status = Get_Dog_Status();
	if(Prox_Status == CLOSE)
	{
		if(Time_Status == NOT_RESTRICTED||Dog_Status == OUTSIDE)
		{
			if((Weather_Status == GOOD_WEATHER||Dog_Status == OUTSIDE) && (Dog_Lock_Flag == LOCKED))
			{
				printf("Door unlocking.\n\r");
				Set_GPIO(ACTUATOR,HIGH);
				Lock_Status = OPEN;
				Dog_Lock_Flag = OPEN;
				Lock_Counter = 0;
			}
			else if(Dog_Lock_Flag == OPEN)
			{
				printf("Weather Conflict.\n\r");
				//Set_GPIO(ACTUATOR,LOW);
				Lock_Counter++;
				Lock_Status = LOCKED;
				Dog_Lock_Flag = LOCKED;
			}
		}
		else if (Dog_Lock_Flag == OPEN)
		{
		printf("Timing Conflict.\n\r");
		//Set_GPIO(ACTUATOR,LOW);
		Lock_Counter++;
		Lock_Status = LOCKED;
		Dog_Lock_Flag = LOCKED;
		}
	}
	else if (Dog_Lock_Flag == OPEN)
	{
		printf("Proximity Conflict.\n\r");
		//Set_GPIO(ACTUATOR,LOW);
		Lock_Counter++;
		Lock_Status = LOCKED;
		Dog_Lock_Flag = LOCKED;
	}

	if(0 < Lock_Counter)
	{
		Lock_Counter++;
		if (LOCK_THRESH == Lock_Counter)
		{
		Set_GPIO(ACTUATOR,LOW);
		Lock_Counter = 0;
		}
	}

	if(Time_Status == NOT_RESTRICTED && Weather_Status == GOOD_WEATHER)
	{
		if (Dog_Status == OUTSIDE && Dog_Status_Flag != LOCKED)
		{
			Set_GPIO(GREEN_LED,HIGH);
			Set_GPIO(RED_LED, HIGH);
			Set_GPIO(BLUE_LED, LOW);
			printf("Dog is currently outside.\n\r");
			Dog_Status_Flag = OUTSIDE;
		}
		else if (Dog_Status_Flag != 1)
		{
			Set_GPIO(GREEN_LED,LOW);
			Set_GPIO(RED_LED, HIGH);
			Set_GPIO(BLUE_LED, HIGH);
			printf("Dog is currently inside.\n\r");
			Dog_Status_Flag = INSIDE;
		}
	}
	else if (Dog_Status_Flag != 2)
	{
		Set_GPIO(GREEN_LED,HIGH);
		Set_GPIO(RED_LED, LOW);
		Set_GPIO(BLUE_LED, HIGH);
		printf("The dog is inside due to weather conditions or time schedule.\n\r");
		Dog_Status_Flag = OTHER;
	}
}


void PORTC_IRQHandler(void)
{
    if (Lock_Method)
    {
        Lock_Method=AUTO;
    }
    else
    {
        Lock_Method=MANUAL;
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

uint8_t Get_Lock_Method(void)
{
    uint8_t ret_val;

    NVIC_DisableIRQ(PORTC_IRQn);
    ret_val = Lock_Method;
    NVIC_EnableIRQ(PORTC_IRQn);

    return(ret_val);
}
