/*
 * proximity_estimation.c
 *
 *  Created on: Mar 28, 2019
 *      Author: Devin
 */

#include "proximity_estimation.h"
#include "platform_k64f.h" /* Xbee library */

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

void Run_Proximity_Estimation(void)
{
    static uint8_t timer_initialized = FALSE;

    if (!timer_initialized)
    {
        Init_Xbee_Timer();
        timer_initialized = TRUE;
    }
}

