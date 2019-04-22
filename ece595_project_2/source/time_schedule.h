#ifndef TIME_SCHEDULE_H_
#define TIME_SCHEDULE_H_

#include "fsl_rtc.h"

typedef enum {
    NOT_RESTRICTED = 0,
    RESTRICTED,
    UNKNOWN
} Time_Schedule_Status_T;

extern void Start_RTC(rtc_datetime_t * datetime, int8_t day);
extern void Update_Restricted_Intervals(const char *json_data);
extern void Print_Datetime(void);
extern void Get_RTC_Time(rtc_datetime_t * datetime);
extern Time_Schedule_Status_T Get_Time_Schedule_Status(void);

#endif /* TIME_SCHEDULE_H_ */
