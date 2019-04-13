#include "lwip/apps/http_client.h"
#include "lwip/altcp.h"
#include "mbedtls/debug.h"

static uint8_t queried = 0;
static uint8_t callback_arg;

httpc_state_t * http_state;
httpc_connection_t settings;

static err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);

void Query_Weather_API(void)
{
    if (0 == queried)
    {
        mbedtls_debug_set_threshold(0);
        httpc_get_file_dns("api.openweathermap.org", 443, "/data/2.5/weather?q=London,uk&appid=b6907d289e10d714a6e88b30761fae22", &settings, &Weather_API_Callback, &callback_arg, &http_state);
        queried = 1;
    }
}

err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf(p->payload);

    return(ERR_OK);
}
