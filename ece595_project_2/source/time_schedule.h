#ifndef TIME_SCHEDULE_H_
#define TIME_SCHEDULE_H_

#include "fsl_rtc.h"

extern void Start_RTC(rtc_datetime_t * datetime);
extern void Print_Datetime(void);

#endif /* TIME_SCHEDULE_H_ */
