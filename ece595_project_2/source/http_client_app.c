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
#define HTTPS_PORT       443
#define ZIP_LENGTH       5

static uint8_t Callback_Arg;
static uint32_t User_Zip = 46062;

httpc_state_t * HTTP_State;
httpc_connection_t Settings;

static err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);

void Query_Weather_API(void)
{
    char uri[URI_BASE_SIZE+ZIP_LENGTH+URI_END_SIZE+API_KEY_SIZE+1];

    mbedtls_debug_set_threshold(0);
    sprintf(uri, "%s%d%s%s", URI_BASE, User_Zip, URI_END, API_KEY);
    httpc_get_file_dns(WEATHER_ENDPOINT, HTTPS_PORT, uri, \
                       &Settings, &Weather_API_Callback, &Callback_Arg, &HTTP_State);
}

err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf(p->payload);

    return(ERR_OK);
}
