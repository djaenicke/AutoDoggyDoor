#include <string.h>

#include "lwip/apps/http_client.h"
#include "lwip/altcp.h"
#include "mbedtls/debug.h"
#include "weather_api_key.h"
#include "cJSON.h"
#include "time_schedule.h"

#define WEATHER_ENDPOINT "api.openweathermap.org"
#define URI_BASE         "/data/2.5/weather?zip="
#define URI_END          ",us&appid="
#define URI_BASE_SIZE    sizeof(URI_BASE)-1
#define URI_END_SIZE     sizeof(URI_END)-1
#if LWIP_ALTCP && LWIP_ALTCP_TLS
#define PORT       443
#else
#define PORT       80
#endif
#define ZIP_LENGTH       5

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum {
    WEATHER=0,
    TIME
} API_t;

static uint8_t Callback_Arg;
static uint32_t User_Zip = 46062;
static uint8_t Waiting = 0;

httpc_state_t * HTTP_State;
httpc_connection_t Settings;
API_t Current_API = TIME;

static err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t Time_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);

void Run_HTTP_Client(void)
{
    char uri[URI_BASE_SIZE+ZIP_LENGTH+URI_END_SIZE+API_KEY_SIZE+1];

    if (!Waiting)
    {
        if (WEATHER == Current_API)
        {
            sprintf(uri, "%s%d%s%s", URI_BASE, User_Zip, URI_END, API_KEY);
            httpc_get_file_dns(WEATHER_ENDPOINT, PORT, uri, \
                               &Settings, &Weather_API_Callback, &Callback_Arg, &HTTP_State);
            Waiting = TRUE;
        }
        else if (TIME == Current_API)
        {
            httpc_get_file_dns("worldtimeapi.org", PORT, "/api/ip", \
                               &Settings, &Time_API_Callback, &Callback_Arg, &HTTP_State);
            Current_API = WEATHER;
            Waiting = TRUE;
        }
    }
    else
    {
        printf("Waiting for response to previous request...\r\n");
    }
}

err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    Print_Datetime();

    printf(p->payload);
    printf("\r\n");
    Waiting = FALSE;

    return(ERR_OK);
}

err_t Time_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    rtc_datetime_t datetime;
    char * current_ptr = NULL;
    char * next_ptr = NULL;

    const cJSON *datetime_str = NULL;
    cJSON *json = cJSON_Parse(p->payload);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf(error_ptr);
        }
    }

    datetime_str = cJSON_GetObjectItemCaseSensitive(json, "datetime");

    if (cJSON_IsString(datetime_str) && (datetime_str->valuestring != NULL))
    {
        printf("datetime \"%s\"\n\r", datetime_str->valuestring);

        datetime.year = (uint16_t) strtoul(datetime_str->valuestring, &next_ptr, 10);
        current_ptr = next_ptr + 1;

        datetime.month = (uint8_t) strtoul(current_ptr, &next_ptr, 10);
        current_ptr = next_ptr + 1;

        datetime.day = (uint8_t) strtoul(current_ptr, &next_ptr, 10);
        current_ptr = next_ptr + 1;

        datetime.hour = (uint8_t) strtoul(current_ptr, &next_ptr, 10);
        current_ptr = next_ptr + 1;

        datetime.minute = (uint8_t) strtoul(current_ptr, &next_ptr, 10);
        current_ptr = next_ptr + 1;

        datetime.second = (uint8_t) strtoul(current_ptr, &next_ptr, 10);
    }

    Start_RTC(&datetime);

    Waiting = FALSE;

    cJSON_Delete(json);

    return(ERR_OK);
}




