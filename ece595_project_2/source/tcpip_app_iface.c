/*
 * tcpip_app_iface.c
 *
 *  Created on: Mar 24, 2019
 *      Author: Devin
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <http_server_app.h>
#include "board.h"
#include "tcpip_app_iface.h"
#include "ksdk_mbedtls.h"
#include "mdns.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MAC address configuration. */
#define configMAC_ADDR                 \
{                                      \
    0x02, 0x12, 0x13, 0x10, 0x15, 0x12 \
}

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
#define MDNS_HOSTNAME "lwip-http"

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
void Init_Network_If(struct netif * net_if)
{
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0 = {
        .phyAddress = EXAMPLE_PHY_ADDRESS, .clockName = kCLOCK_CoreSysClk, .macAddress = configMAC_ADDR,
    };

    CRYPTO_InitHardware();

    IP4_ADDR(&fsl_netif0_ipaddr, 0U, 0U, 0U, 0U);
    IP4_ADDR(&fsl_netif0_netmask, 0U, 0U, 0U, 0U);
    IP4_ADDR(&fsl_netif0_gw, 0U, 0U, 0U, 0U);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(net_if, &fsl_netif0_ipaddr, &fsl_netif0_netmask, &fsl_netif0_gw, &fsl_enet_config0,
                       ethernetif0_init, tcpip_input);
    netifapi_netif_set_default(net_if);
    netifapi_netif_set_up(net_if);

    mdns_resp_init();
    mdns_resp_add_netif(net_if, MDNS_HOSTNAME, 60);
    mdns_resp_add_service(net_if, MDNS_HOSTNAME, "_http", DNSSD_PROTO_TCP, 80, 300, http_srv_txt, NULL);

    netifapi_dhcp_start(net_if);
}

void Print_DHCP_State(struct netif *netif)
{
    struct dhcp *dhcp;
    static uint8_t dhcp_last_state = DHCP_STATE_OFF;

    dhcp = netif_dhcp_data(netif);

    if (dhcp == NULL)
    {
        dhcp_last_state = DHCP_STATE_OFF;
    }
    else if (dhcp_last_state != dhcp->state)
    {
        dhcp_last_state = dhcp->state;

        PRINTF(" DHCP state       : ");
        switch (dhcp_last_state)
        {
            case DHCP_STATE_OFF:
                PRINTF("OFF\r\n");
                break;
            case DHCP_STATE_REQUESTING:
                PRINTF("REQUESTING\r\n");
                break;
            case DHCP_STATE_INIT:
                PRINTF("INIT\r\n");
                break;
            case DHCP_STATE_REBOOTING:
                PRINTF("REBOOTING\r\n");
                break;
            case DHCP_STATE_REBINDING:
                PRINTF("REBINDING\r\n");
                break;
            case DHCP_STATE_RENEWING:
                PRINTF("RENEWING\r\n");
                break;
            case DHCP_STATE_SELECTING:
                PRINTF("SELECTING\r\n");
                break;
            case DHCP_STATE_INFORMING:
                PRINTF("INFORMING\r\n");
                break;
            case DHCP_STATE_CHECKING:
                PRINTF("CHECKING\r\n");
                break;
            case DHCP_STATE_BOUND:
                PRINTF("BOUND");
                break;
            case DHCP_STATE_BACKING_OFF:
                PRINTF("BACKING_OFF\r\n");
                break;
            default:
                PRINTF("%u", dhcp_last_state);
                assert(0);
                break;
        }

        if (dhcp_last_state == DHCP_STATE_BOUND)
        {
            PRINTF("\r\n");
            PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
            PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
            PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
        }
    }
}

