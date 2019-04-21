#include "stdio.h"
#include "time_schedule.h"

static uint8_t RTC_Init_Complete = 0;

void Start_RTC(rtc_datetime_t * datetime)
{
    rtc_config_t cfg;

    RTC_GetDefaultConfig(&cfg);
    RTC_Init(RTC, &cfg);
    RTC_SetClockSource(RTC);
    RTC_StopTimer(RTC);

    RTC_SetDatetime(RTC, datetime);
    RTC_StartTimer(RTC);

    RTC_Init_Complete = 1;
}

void Print_Datetime(void)
{
    rtc_datetime_t datetime;

    RTC_GetDatetime(RTC, &datetime);

    printf("Current time: %02d:%02d:%02d\n\r", datetime.hour, datetime.minute, datetime.second);
}

void Get_RTC_Time(rtc_datetime_t * datetime)
{
    if (RTC_Init_Complete)
    {
        RTC_GetDatetime(RTC, datetime);
    }
    else
    {
        datetime->hour = 0;
        datetime->minute = 0;
        datetime->second = 0;
    }
}


