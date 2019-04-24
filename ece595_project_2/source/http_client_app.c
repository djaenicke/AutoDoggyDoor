#include <string.h>
#include <stdio.h>

#include "http_client_app.h"
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

#define UNKNOWN_ID          (0)
#define NO_RAIN_AMOUNT_DATA (-1.0)
#define MAX_RAIN_AMOUNT     (5.0)  /* (mm) */
#define CLEAR_WEATHER_ID    (800)
#define MAX_CLOUDS_ID       (804)

typedef enum {
    WEATHER=0,
    TIME
} API_t;

typedef struct {
    uint16_t id;
    double rain_1h_amount;
} Weather_Info_T;

static Weather_Info_T Weather_Info = {UNKNOWN_ID, NO_RAIN_AMOUNT_DATA};
static uint8_t Callback_Arg;
static uint32_t User_Zip = 0;
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
        	if (User_Zip)
        	{
				sprintf(uri, "%s%d%s%s", URI_BASE, User_Zip, URI_END, API_KEY);
				httpc_get_file_dns(WEATHER_ENDPOINT, PORT, uri, \
								   &Settings, &Weather_API_Callback, &Callback_Arg, &HTTP_State);
				Waiting = TRUE;
        	}
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
    const cJSON * weather_array = NULL;
    const cJSON * weather = NULL;
    const cJSON * weather_id = NULL;
    const cJSON * weather_main = NULL;

    const cJSON * rain = NULL;
    const cJSON * rain_1h = NULL;

    cJSON *json = cJSON_Parse(p->payload);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf(error_ptr);
        }
    }

    weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather");

    cJSON_ArrayForEach(weather, weather_array)
    {
        weather_id = cJSON_GetObjectItemCaseSensitive(weather, "id");
        weather_main = cJSON_GetObjectItemCaseSensitive(weather, "main");
    }

    /* Get the current weather condition */
    printf("weather id: %d\n\r", (int) weather_id->valuedouble);
    Weather_Info.id = (uint16_t) weather_id->valuedouble;
    printf("main weather: %s\n\r", weather_main->valuestring);

    /* Get the amount of rain received in the last hour */
    rain = cJSON_GetObjectItemCaseSensitive(json, "rain");

    if (rain != NULL)
    {
        rain_1h = cJSON_GetObjectItemCaseSensitive(rain, "1h");
        printf("1 hour rain amount: %.2f\n\r", (float) rain_1h->valuedouble);
        Weather_Info.rain_1h_amount = rain_1h->valuedouble;
    }
    else
    {
        Weather_Info.rain_1h_amount = NO_RAIN_AMOUNT_DATA;
    }

    Waiting = FALSE;

    cJSON_Delete(json);

    return(ERR_OK);
}

err_t Time_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    rtc_datetime_t datetime = {0};
    int8_t day = -1;
    char * current_ptr = NULL;
    char * next_ptr = NULL;

    const cJSON *datetime_str = NULL;
    const cJSON *day_of_week = NULL;
    cJSON *json = cJSON_Parse(p->payload);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf(error_ptr);
        }
    }
    else
    {
		datetime_str = cJSON_GetObjectItemCaseSensitive(json, "datetime");

		if (cJSON_IsString(datetime_str) && (datetime_str->valuestring != NULL))
		{
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

		day_of_week = cJSON_GetObjectItemCaseSensitive(json, "day_of_week");

		if (cJSON_IsNumber(day_of_week))
		{
			day = day_of_week->valueint;
		}

		Start_RTC(&datetime, day);
		Print_Datetime();

		Waiting = FALSE;
    }

    cJSON_Delete(json);

    return(ERR_OK);
}

Weather_Status_T Get_Weather_Status(void)
{
    Weather_Status_T status = BAD_WEATHER;

    if ((Weather_Info.id >= CLEAR_WEATHER_ID) && (Weather_Info.id <= MAX_CLOUDS_ID) && \
        (Weather_Info.rain_1h_amount <= MAX_RAIN_AMOUNT))
    {
        status = GOOD_WEATHER;
    }

    return(status);
}

void Set_Zip_Code(const char* str)
{
    const cJSON *zip = NULL;
    cJSON *json = cJSON_Parse(str);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf(error_ptr);
        }
    }
    else
    {
		zip = cJSON_GetObjectItemCaseSensitive(json, "zip");

		if (cJSON_IsNumber(zip))
		{
			User_Zip = zip->valueint;
		}
    }

	cJSON_Delete(json);
}

uint32_t Get_Zip_Code(void)
{
	return(User_Zip);

void Run_IoT_Task(void *pvParameters)
{

}
