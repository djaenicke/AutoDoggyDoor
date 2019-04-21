#include <stdio.h>

#include "time_schedule.h"
#include "cJSON.h"

#define NUM_INTERVALS 5

typedef struct {
    int id;
    int days;
    rtc_datetime_t start;
    rtc_datetime_t end;
} Restricted_Interval_T;

static uint8_t RTC_Init_Complete = 0;
static Restricted_Interval_T Restricted_Intervals[NUM_INTERVALS];

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

void Update_Restricted_Intervals(const char *json_data)
{
    uint8_t i=0;
    char * current_ptr = NULL;
    char * next_ptr = NULL;

    cJSON *interval   = NULL;
    cJSON *id         = NULL;
    cJSON *days       = NULL;
    cJSON *start_time = NULL;
    cJSON *stop_time  = NULL;
    cJSON *json = cJSON_Parse(json_data);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf(error_ptr);
        }
    }

    cJSON_ArrayForEach(interval, json)
    {
        if (i < NUM_INTERVALS)
        {
            id = cJSON_GetObjectItemCaseSensitive(interval, "id");
            Restricted_Intervals[i].id = id->valueint;

            if (-1 != Restricted_Intervals[i].id)
            {
                days = cJSON_GetObjectItemCaseSensitive(interval, "days");
                Restricted_Intervals[i].days = days->valueint;

                start_time = cJSON_GetObjectItemCaseSensitive(interval, "start_time");
                Restricted_Intervals[i].start.hour = (uint16_t) strtoul(start_time->valuestring, &next_ptr, 10);
                current_ptr = next_ptr + 1;
                Restricted_Intervals[i].start.minute = (uint16_t) strtoul(current_ptr, &next_ptr, 10);

                stop_time = cJSON_GetObjectItemCaseSensitive(interval, "stop_time");
                Restricted_Intervals[i].end.hour = (uint16_t) strtoul(stop_time->valuestring, &next_ptr, 10);
                current_ptr = next_ptr + 1;
                Restricted_Intervals[i].end.minute = (uint16_t) strtoul(current_ptr, &next_ptr, 10);
            }
        }

        i++;
    }

    cJSON_Delete(json);
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


