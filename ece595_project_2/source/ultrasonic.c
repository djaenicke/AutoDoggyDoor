/*
 * ultrasonic.c
 *
 *  Created on: Apr 6, 2019
 *      Author: Lacy
 */
#include "fsl_ftm.h"
#include "clock_config.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "io_abstraction.h"
#include "ultrasonic.h"
#include "time_schedule.h"
#include "http_client_app.h"
#include "proximity_estimation.h"
#include "ultrasonic.h"

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/1)
#define DOGCOUNT 4

static Proximity_Status_T Prox_Status;
static Time_Schedule_Status_T Time_Status;
static Weather_Status_T Weather_Status;
static volatile Dog_Status_T dog_status;
static uint8_t dog_counter;

void Init_USS_Timer(void)
{
    ftm_config_t ftm_info;
    FTM_GetDefaultConfig(&ftm_info);
    /* Divide FTM clock by 4 */
    ftm_info.prescale = kFTM_Prescale_Divide_4;
    FTM_Init(FTM1, &ftm_info);
    FTM_SetTimerPeriod(FTM1, USEC_TO_COUNT(1U, FTM_SOURCE_CLOCK));
}

void Detect_Pet(void)
{
	Time_Status = Get_Time_Schedule_Status();
	Weather_Status = Get_Weather_Status();
	Prox_Status = Get_Proximity_Status();
	if(Time_Status == NOT_RESTRICTED && Weather_Status == GOOD_WEATHER && Prox_Status == CLOSE)
	{
		uint32_t start_time, stop_time;
		FTM_StartTimer(FTM1, kFTM_SystemClock);
		//Triggers the ultrasonic sensor and starts the timer
		start_time = FTM_GetCurrentTimerCount(FTM1);
		GPIO_PinWrite(GPIOB,2,1);

		//While the time is less than 1500 usec continue to see if PTB3 has received the echo signal, if it has record the stop time. comes out to about 2.5 ft
		do
		{
			if(GPIO_PinRead(GPIOB,3)==1)
			{
				stop_time = FTM_GetCurrentTimerCount(FTM1);
				break;
			}
		} while ((FTM_GetCurrentTimerCount(FTM1) - start_time) < 1500);

		FTM_StopTimer(FTM1);

		if ((stop_time - start_time) < 1500)
		{
			dog_counter++;
		}
		else
		{
			dog_counter=0;
		}

		if(dog_counter > DOGCOUNT)
		{
			if (dog_status == INSIDE)
			{
				dog_status = OUTSIDE;
			}
			else
			{
				dog_status = INSIDE;
			}
		}
	}
}

Dog_Status_T Get_Dog_Status(void)
{
	return(dog_status);
}
