#ifndef HTTP_CLIENT_APP_H_
#define HTTP_CLIENT_APP_H_

typedef enum {
    BAD_WEATHER = 0,
    GOOD_WEATHER
} Weather_Status_T;

extern void Run_HTTP_Client(void);
extern Weather_Status_T Get_Weather_Status(void);
extern void Run_IoT_Task(void *pvParameters);

#endif /* HTTP_CLIENT_APP_H_ */
