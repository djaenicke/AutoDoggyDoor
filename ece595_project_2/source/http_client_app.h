#ifndef HTTP_CLIENT_APP_H_
#define HTTP_CLIENT_APP_H_

#include <stdint.h>

typedef enum {
    BAD_WEATHER = 0,
    GOOD_WEATHER
} Weather_Status_T;

extern void Run_HTTP_Client(void);
extern Weather_Status_T Get_Weather_Status(void);
extern void Set_Zip_Code(const char* str);
extern uint32_t Get_Zip_Code(void);
extern void Run_IoT_Logging(void);

#endif /* HTTP_CLIENT_APP_H_ */
