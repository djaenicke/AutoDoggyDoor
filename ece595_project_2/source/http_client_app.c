#include <string.h>

#include "lwip/apps/http_client.h"
#include "lwip/altcp.h"
#include "mbedtls/debug.h"
#include "weather_api_key.h"

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

static uint8_t Callback_Arg;
static uint32_t User_Zip = 46062;
static uint8_t Debug_Threshold = 1;

httpc_state_t * HTTP_State;
httpc_connection_t Settings;

static err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);

void Query_Weather_API(void)
{
    char uri[URI_BASE_SIZE+ZIP_LENGTH+URI_END_SIZE+API_KEY_SIZE+1];

    mbedtls_debug_set_threshold(Debug_Threshold);
    sprintf(uri, "%s%d%s%s", URI_BASE, User_Zip, URI_END, API_KEY);
    httpc_get_file_dns(WEATHER_ENDPOINT, PORT, uri, \
                       &Settings, &Weather_API_Callback, &Callback_Arg, &HTTP_State);
}

err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf(p->payload);
    printf("\r\n");

    return(ERR_OK);
}
