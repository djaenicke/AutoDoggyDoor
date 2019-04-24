#include <stdio.h>

#include "time_schedule.h"
#include "cJSON.h"

#define NOT_SET -1

typedef enum {
    eSUNDAY = 0,
    eMONDAY,
    eTUESDAY,
    eWEDNESDAY,
    eTHURSDAY,
    eFRIDAY,
    eSATURDAY,
    eUNKNOWN_DAY
} Day_T;

static uint8_t RTC_Init_Complete = 0;
static Day_T Day = eUNKNOWN_DAY;
static Restricted_Interval_T Restricted_Intervals[NUM_INTERVALS] =
{
    {NOT_SET, 0, {0}, {0}},
    {NOT_SET, 0, {0}, {0}},
    {NOT_SET, 0, {0}, {0}},
    {NOT_SET, 0, {0}, {0}},
    {NOT_SET, 0, {0}, {0}},
};

static uint8_t Is_Day_Restricted(uint8_t encoded_days);

void Start_RTC(rtc_datetime_t * datetime, int8_t day)
{
    rtc_config_t cfg;

    Day = day;

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

        if (0 == memcmp(&(Restricted_Intervals[i].start), &(Restricted_Intervals[i].end), sizeof(rtc_datetime_t)))
        {
            Restricted_Intervals[i].id = -1;
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

Time_Schedule_Status_T Get_Time_Schedule_Status(void)
{
    Time_Schedule_Status_T status = UNKNOWN;
    rtc_datetime_t current_time;
    uint8_t i;

    /* Make sure RTC data is available*/
    if (RTC_Init_Complete)
    {
        RTC_GetDatetime(RTC, &current_time);

        for (i=0; i<NUM_INTERVALS; i++)
        {
            /* Make sure the current interval has been set */
            if (NOT_SET != Restricted_Intervals[i].id)
            {
                if (1 == Is_Day_Restricted(Restricted_Intervals[i].days))
                {
                    /* Are we inside an interval? */
                    if (current_time.hour >= Restricted_Intervals[i].start.hour && \
                        current_time.hour <= Restricted_Intervals[i].end.hour)
                    {
                        /* Is the interval less than an hour? */
                        if (Restricted_Intervals[i].end.hour == Restricted_Intervals[i].start.hour)
                        {
                            if (current_time.minute >= Restricted_Intervals[i].start.minute && \
                                current_time.minute <= Restricted_Intervals[i].end.minute)
                            {
                                status = RESTRICTED;
                                break;
                            }
                        }
                        /* Do we need to check the start minutes ? */
                        else if (current_time.hour == Restricted_Intervals[i].start.hour)
                        {
                            if (current_time.minute >= Restricted_Intervals[i].start.minute)
                            {
                                status = RESTRICTED;
                                break;
                            }
                        }
                        /* Do we need to check the end minutes ? */
                        else if (current_time.hour == Restricted_Intervals[i].end.hour)
                        {
                            if (current_time.minute < Restricted_Intervals[i].end.minute)
                            {
                                status = RESTRICTED;
                                break;
                            }
                        }
                        else
                        {
                            status = NOT_RESTRICTED;
                        }
                    }
                }
                else
                {
                    status = NOT_RESTRICTED;
                }
            }
        }
    }

    return (status);
}

uint8_t Is_Day_Restricted(uint8_t encoded_days)
{
    uint8_t is_restricted = 0;

    if (0xFF == (encoded_days & 0xFF))
    {
        is_restricted = 1;
    }

    if (!is_restricted)
    {
        switch (Day)
        {
            case eSUNDAY:
                if (0x40 == (encoded_days & 0x40))
                    is_restricted = 1;
                break;
            case eMONDAY:
                if (0x01 == (encoded_days & 0x01))
                    is_restricted = 1;
                break;
            case eTUESDAY:
                if (0x02 == (encoded_days & 0x02))
                    is_restricted = 1;
                break;
            case eWEDNESDAY:
                if (0x04 == (encoded_days & 0x04))
                    is_restricted = 1;
                break;
            case eTHURSDAY:
                if (0x08 == (encoded_days & 0x08))
                    is_restricted = 1;
                break;
            case eFRIDAY:
                if (0x10 == (encoded_days & 0x10))
                    is_restricted = 1;
                break;
            case eSATURDAY:
                if (0x20 == (encoded_days & 0x20))
                    is_restricted = 1;
                break;
            default:
                break;
        }
    }

    return is_restricted;
}

void Get_Restricted_Intervals(void * intervals)
{
    if (NULL != intervals)
    {
        memcpy(intervals, &Restricted_Intervals, sizeof(Restricted_Intervals));
    }
}

