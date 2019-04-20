#include "stdio.h"
#include "time_schedule.h"

void Start_RTC(rtc_datetime_t * datetime)
{
    rtc_config_t cfg;

    RTC_GetDefaultConfig(&cfg);
    RTC_Init(RTC, &cfg);
    RTC_SetClockSource(RTC);
    RTC_StopTimer(RTC);

    RTC_SetDatetime(RTC, datetime);
    RTC_StartTimer(RTC);
}

void Print_Datetime(void)
{
    rtc_datetime_t datetime;

    RTC_GetDatetime(RTC, &datetime);

    printf("Current time: %02d:%02d:%02d\n\r", datetime.hour, datetime.minute, datetime.second);
}


