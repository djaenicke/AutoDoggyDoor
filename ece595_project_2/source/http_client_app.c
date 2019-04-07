#include "lwip/apps/http_client.h"

static uint8_t queried = 0;
static uint8_t callback_arg;

httpc_state_t * http_state;
httpc_connection_t settings;

static err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);

void Query_Weather_API(void)
{
    if (0 == queried)
    {
        httpc_get_file_dns("api.weather.gov", 443, "/", &settings, &Weather_API_Callback, &callback_arg, &http_state);
        queried = 1;
    }
}


err_t Weather_API_Callback(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    queried=0;

    printf(p->payload);

    return(ERR_OK);
}
