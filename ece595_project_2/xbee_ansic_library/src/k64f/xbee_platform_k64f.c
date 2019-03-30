#include "xbee/platform.h"
#include "fsl_ftm.h"

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/4)

static uint32_t Xbee_Milliseconds_Timer = 0;
static uint32_t Xbee_Seconds_Timer = 0;

// These function declarations have extra parens around the function name
// because we define them as macros referencing the global directly.
uint32_t (xbee_seconds_timer)(void)
{
    uint32_t ret_val = 0;

    /* Need to disable interrupts to ensure data consistency */
    FTM_DisableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
    ret_val = Xbee_Seconds_Timer;
    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    return(ret_val);
}

uint32_t (xbee_millisecond_timer)(void)
{
    uint32_t ret_val = 0;

    /* Need to disable interrupts to ensure data consistency */
    FTM_DisableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);
    ret_val = Xbee_Milliseconds_Timer;
    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    return(ret_val);
}

void Init_Xbee_Timer(void)
{
    ftm_config_t ftm_info;

    FTM_GetDefaultConfig(&ftm_info);

    /* Divide FTM clock by 4 */
    ftm_info.prescale = kFTM_Prescale_Divide_4;

    FTM_Init(FTM0, &ftm_info);

    /*
     * Set timer period.
    */
    FTM_SetTimerPeriod(FTM0, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));

    FTM_EnableInterrupts(FTM0, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(FTM0_IRQn);

    FTM_StartTimer(FTM0, kFTM_SystemClock);
}

void FTM0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);
    Xbee_Milliseconds_Timer++;

    if (Xbee_Milliseconds_Timer == 1000)
    {
        Xbee_Seconds_Timer++;
        Xbee_Milliseconds_Timer=0;
    }

    __DSB();
}

