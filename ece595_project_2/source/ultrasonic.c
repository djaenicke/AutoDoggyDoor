/*
 * ultrasonic.c
 *
 *  Created on: Apr 6, 2019
 *      Author: Lacy
 */

#include <stdio.h>

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
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/4)
#define DETECTION_COUNT 10

static Proximity_Status_T Prox_Status;
static Time_Schedule_Status_T Time_Status;
static Weather_Status_T Weather_Status;
static volatile Dog_Status_T dog_status;
static uint8_t dog_counter;

void Detect_Pet(void)
{
    ftm_config_t ftm_info;
    float dist = 0;

    FTM_GetDefaultConfig(&ftm_info);

    /* Divide FTM clock by 4 */
    ftm_info.prescale = kFTM_Prescale_Divide_4;

    FTM_Init(FTM1, &ftm_info);
    FTM_SetTimerPeriod(FTM1, UINT16_MAX);

	Time_Status = Get_Time_Schedule_Status();
	Weather_Status = Get_Weather_Status();
	Prox_Status = Get_Proximity_Status();

    if (Time_Status == NOT_RESTRICTED && Weather_Status == GOOD_WEATHER && Prox_Status == CLOSE)
	{
		uint32_t start_cnt, stop_cnt;
		FTM_StartTimer(FTM1, kFTM_SystemClock);

		//Triggers the ultrasonic sensor and starts the timer
		start_cnt = FTM_GetCurrentTimerCount(FTM1);
		Set_GPIO(USS_TRIGGER, 1);
		while(COUNT_TO_USEC(FTM_GetCurrentTimerCount(FTM1)-start_cnt, FTM_SOURCE_CLOCK)<=15);
		Set_GPIO(USS_TRIGGER, 0);

		/* Wait for the echo line to go HIGH */
		while(LOW == Read_GPIO(USS_ECHO)); /* Might need a timeout here */
		start_cnt = FTM_GetCurrentTimerCount(FTM1);
		while(HIGH == Read_GPIO(USS_ECHO)); /* Might need a timeout here */
		stop_cnt = FTM_GetCurrentTimerCount(FTM1);

		FTM_StopTimer(FTM1);

		if (stop_cnt > start_cnt)
		{
		    dist = (stop_cnt - start_cnt)*(1.0f/15000000)*343/2*100;
		}
		else
		{
		    dist = ((UINT16_MAX - start_cnt) + stop_cnt)*(1.0f/15000000)*343/2*100;
		}

		if (dist < 30)
		{
			dog_counter++;
		}
		else
		{
			dog_counter=0;
		}

		if (dog_counter > DETECTION_COUNT)
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

    FTM_Deinit(FTM1);
}

Dog_Status_T Get_Dog_Status(void)
{
	return(dog_status);
}
