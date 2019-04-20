/*
 * ultrasonic.c
 *
 *  Created on: Apr 6, 2019
 *      Author: Lacy
 */
#include "fsl_ftm.h"
#include "clock_config.h"
//#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "io_abstraction.h"
#include "ultrasonic.h"

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/1)

void Init_USS_Timer(void)
{
    ftm_config_t ftm_info;

    FTM_GetDefaultConfig(&ftm_info);

    /* Divide FTM clock by 4 */
    ftm_info.prescale = kFTM_Prescale_Divide_4;

    FTM_Init(FTM1, &ftm_info);

    /*
     * Set timer period.
    */
    //FTM_SetTimerPeriod(FTM1, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));

    FTM_StartTimer(FTM1, kFTM_SystemClock);
}

void Detect_Pet(void)
{
	uint32_t start_time, stop_time;
	//GPIO_PinInit();

	//Trigger
	start_time = FTM_GetCurrentTimerCount(FTM1);
	GPIO_PinWrite(TRIGGER);

	do
	{
	//while(echo not received);
	stop_time = FTM_GetCurrentTimerCount(FTM1);
	}
	while (GPIO_PinRead(ECHO)==1)
	{
		distance = stop_time - start_time;
		return distance;

	}
}


