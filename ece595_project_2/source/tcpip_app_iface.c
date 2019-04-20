/*
 * tcpip_app_iface.c
 *
 *  Created on: Mar 24, 2019
 *      Author: Devin
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "board.h"
#include "tcpip_app_iface.h"
#include "server.h"
#include "mdns.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x12 \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
#define MDNS_HOSTNAME "lwip-http"

/* System clock name. */
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/
void Init_Network_If(struct netif * net_if)
{
    ip4_addr_t fsl_netif0_ipaddr, fsl_netif0_netmask, fsl_netif0_gw;
    ethernetif_config_t fsl_enet_config0 = {
        .phyAddress = EXAMPLE_PHY_ADDRESS, .clockName = EXAMPLE_CLOCK_NAME, .macAddress = configMAC_ADDR,
    };

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

    PRINTF("\r\n\r\n************************************************\r\n");
    PRINTF(" DHCP example\r\n");
    PRINTF("************************************************\r\n");
}

void Print_DHCP_State(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    struct dhcp *dhcp;
    u8_t dhcp_last_state = DHCP_STATE_OFF;

    while (netif_is_up(netif))
    {
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
                    PRINTF("OFF");
                    break;
                case DHCP_STATE_REQUESTING:
                    PRINTF("REQUESTING");
                    break;
                case DHCP_STATE_INIT:
                    PRINTF("INIT");
                    break;
                case DHCP_STATE_REBOOTING:
                    PRINTF("REBOOTING");
                    break;
                case DHCP_STATE_REBINDING:
                    PRINTF("REBINDING");
                    break;
                case DHCP_STATE_RENEWING:
                    PRINTF("RENEWING");
                    break;
                case DHCP_STATE_SELECTING:
                    PRINTF("SELECTING");
                    break;
                case DHCP_STATE_INFORMING:
                    PRINTF("INFORMING");
                    break;
                case DHCP_STATE_CHECKING:
                    PRINTF("CHECKING");
                    break;
                case DHCP_STATE_BOUND:
                    PRINTF("BOUND");
                    break;
                case DHCP_STATE_BACKING_OFF:
                    PRINTF("BACKING_OFF");
                    break;
                default:
                    PRINTF("%u", dhcp_last_state);
                    assert(0);
                    break;
            }
            PRINTF("\r\n");

            if (dhcp_last_state == DHCP_STATE_BOUND)
            {
                PRINTF("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&netif->ip_addr));
                PRINTF(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&netif->netmask));
                PRINTF(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&netif->gw));
            }
        }

        sys_msleep(20U);
    }

    vTaskDelete(NULL);
}

