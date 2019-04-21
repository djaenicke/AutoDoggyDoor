#ifndef TIME_SCHEDULE_H_
#define TIME_SCHEDULE_H_

#include "fsl_rtc.h"

extern void Start_RTC(rtc_datetime_t * datetime);
extern void Update_Restricted_Intervals(const char *json_data);
extern void Print_Datetime(void);
extern void Get_RTC_Time(rtc_datetime_t * datetime);

#endif /* TIME_SCHEDULE_H_ */
