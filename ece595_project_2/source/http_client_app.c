#include "lwip/apps/http_client.h"

#define TEST_URI   "https://reqres.in/api/users/2"
#define GOOGLE_DNS "8.8.8.8"
#define DNS_PORT   0xFFFF


static err_t Weather_API_Callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

void Query_Weather_API(void)
{
    httpc_state_t * http_state;
    httpc_connection_t settings;

    httpc_get_file_dns(GOOGLE_DNS, DNS_PORT, TEST_URI, &settings, &Weather_API_Callback, NULL, &http_state);
}


err_t Weather_API_Callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{

}
