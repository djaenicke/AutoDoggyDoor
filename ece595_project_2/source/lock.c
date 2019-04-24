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
#define INSIDE 0
#define OUTSIDE 1


/* Variables */
static Proximity_Status_T Prox_Status;
static Time_Schedule_Status_T Time_Status;
static Weather_Status_T Weather_Status;
static volatile uint8_t Lock_Status = LOCKED;
static volatile uint8_t Lock_Method = MANUAL;
static Dog_Status_T Dog_Status=INSIDE;
static uint8_t Dog_Lock_Flag=LOCKED;

void Run_Lock_Control(void)
{
	Time_Status = Get_Time_Schedule_Status();
	Weather_Status = Get_Weather_Status();
	Prox_Status = Get_Proximity_Status();
	Dog_Status = Get_Dog_Status();

	/* Is dog close to the door ? */
	if (Prox_Status == CLOSE)
	{
	    if ((Dog_Status == OUTSIDE) && (Weather_Status==BAD_WEATHER))
	    {
	       Set_GPIO(ACTUATOR, HIGH);
	    }
	    else if ((Dog_Status == OUTSIDE) && (Time_Status == RESTRICTED))
	    {
	        Set_GPIO(ACTUATOR, HIGH);
	    }
	    /* Is time restricted and dog triggered USS ? */
	    else if (Time_Status == NOT_RESTRICTED && Dog_Status == OUTSIDE)
		{
			if (Weather_Status == GOOD_WEATHER && Dog_Status == OUTSIDE)
			{
				//printf("Door unlocking.\n\r");
				Set_GPIO(ACTUATOR,HIGH);
				Lock_Status = OPEN;
				Dog_Lock_Flag = OPEN;
			}
			else if (Weather_Status == BAD_WEATHER)
			{
				//printf("Weather Conflict.\n\r");
			    Set_GPIO(ACTUATOR,LOW);
				Lock_Status = LOCKED;
				Dog_Lock_Flag = LOCKED;
			}
		}
		else if (Time_Status == RESTRICTED)
		{
            //printf("Timing Conflict.\n\r");
		    Set_GPIO(ACTUATOR,LOW);
            Lock_Status = LOCKED;
            Dog_Lock_Flag = LOCKED;
		}
	}
	else
	{
		//printf("Proximity Conflict.\n\r");
	    Set_GPIO(ACTUATOR,LOW);
		Lock_Status = LOCKED;
		Dog_Lock_Flag = LOCKED;
	}

	if (Time_Status == NOT_RESTRICTED && Weather_Status == GOOD_WEATHER)
	{
		if (Dog_Status == OUTSIDE)
		{
			Set_GPIO(GREEN_LED,HIGH);
			Set_GPIO(RED_LED, HIGH);
			Set_GPIO(BLUE_LED, LOW);
		}
		else if (Dog_Status == INSIDE)
		{
			Set_GPIO(GREEN_LED,LOW);
			Set_GPIO(RED_LED, HIGH);
			Set_GPIO(BLUE_LED, HIGH);
		}
	}
	else
	{
		Set_GPIO(GREEN_LED,HIGH);
		Set_GPIO(RED_LED, LOW);
		Set_GPIO(BLUE_LED, HIGH);
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
