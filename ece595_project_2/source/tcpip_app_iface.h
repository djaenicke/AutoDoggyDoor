/*
 * tcpip_app_iface.h
 *
 *  Created on: Mar 24, 2019
 *      Author: Devin
 */

#ifndef TCPIP_APP_IFACE_H_
#define TCPIP_APP_IFACE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"

#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/prot/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/sys.h"
#include "ethernetif.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Stack size of the thread which prints DHCP info. */
#define PRINT_THREAD_STACKSIZE 512

/*! @brief Priority of the thread which prints DHCP info. */
#define PRINT_THREAD_PRIO DEFAULT_THREAD_PRIO

/*******************************************************************************
* Prototypes
******************************************************************************/
extern void Init_Network_If(struct netif * net_if);
extern void Print_DHCP_State(void *arg);

#endif /* TCPIP_APP_IFACE_H_ */
