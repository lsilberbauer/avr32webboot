/**************************************************************************************************
 *
 * @file ethernet.c
 *
 * @brief Initializes the LwIP Stack and starts all Ethernet-related tasks
 *
 * @author Lukas Silberbauer (lukas.silberbauer@taurob.com)
 *
 *  Copyright (c) 2015 taurob GmbH. All rights reserved. See LICENSE.txt.
 *  Perfektastrasse 57/7, 1230 Wien, Austria.
 *
 *************************************************************************************************/

#include "ethernet.h"

#include <string.h>

#include "gpio.h" // Have to include gpio.h before FreeRTOS.h as long as FreeRTOS
                  // redefines the inline keyword to empty.
#include "registry/registry.h"
/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "conf_lwip_threads.h"

/* ethernet includes */
#include "conf_eth.h"
#include "macb.h"

/* lwIP includes */
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
#include "netif/loopif.h"
#else
#include "lwip/inet.h"
#endif

/* webserver includes */
#include "webserver/httpd.h"
#include "ssi/ssi.h"

struct netif MACB_if;

/*************************************************************************************************
 * ethernet_configure_interface
 *
 * Configures the ethernet interface
 */
static void ethernet_configure_interface(void * param)
{
	struct ip_addr    xIpAddr, xNetMask, xGateway;
	extern err_t      ethernetif_init( struct netif *netif );
	unsigned portCHAR MacAddress[6];

	/* Default MAC addr. */
	MacAddress[0] = ETHERNET_CONF_ETHADDR0;
	MacAddress[1] = ETHERNET_CONF_ETHADDR1;
	MacAddress[2] = ETHERNET_CONF_ETHADDR2;
	MacAddress[3] = ETHERNET_CONF_ETHADDR3;
	MacAddress[4] = ETHERNET_CONF_ETHADDR4;
	MacAddress[5] = ETHERNET_CONF_ETHADDR5;

	/* pass the MAC address to MACB module */
	vMACBSetMACAddress( MacAddress );

	/* Default ip addr. */
	IP4_ADDR( &xIpAddr,registry_get_value("IP Address 1"),
	                   registry_get_value("IP Address 2"),
					   registry_get_value("IP Address 3"),
					   registry_get_value("IP Address 4") );

	/* Default Subnet mask. */
	IP4_ADDR( &xNetMask,ETHERNET_CONF_NET_MASK0,ETHERNET_CONF_NET_MASK1,ETHERNET_CONF_NET_MASK2,ETHERNET_CONF_NET_MASK3 );

	/* Default Gw addr. */
	IP4_ADDR( &xGateway,ETHERNET_CONF_GATEWAY_ADDR0,ETHERNET_CONF_GATEWAY_ADDR1,ETHERNET_CONF_GATEWAY_ADDR2,ETHERNET_CONF_GATEWAY_ADDR3 );

	/* add data to netif */
	netif_add( &MACB_if, &xIpAddr, &xNetMask, &xGateway, NULL, ethernetif_init, tcpip_input );

	/* make it the default interface */
	netif_set_default( &MACB_if );

	/* bring it up */
	netif_set_up( &MACB_if );
}

/*************************************************************************************************
 * ethernet_tcpip_init_done
 *
 * Callback executed when the TCP/IP init is done.
 */
static void ethernet_tcpip_init_done(void *arg)
{
	sys_sem_t *sem;
	sem = (sys_sem_t *)arg;

	/* Set hw and IP parameters, initialize MACB too */
	ethernet_configure_interface(NULL);

	#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
	sys_sem_signal(*sem); // Signal the waiting thread that the TCP/IP init is done.
	#else
	sys_sem_signal(sem); // Signal the waiting thread that the TCP/IP init is done.
	#endif
}

/*************************************************************************************************
 * ethernet_init_LwIP
 *
 * starts the LwIP layer
 */
static void ethernet_init_LwIP(void)
{
	sys_sem_t sem;

	#if ( (LWIP_VERSION) == ((1U << 24) | (3U << 16) | (2U << 8) | (LWIP_VERSION_RC)) )
	  sem = sys_sem_new(0); // Create a new semaphore.
	  tcpip_init(ethernet_tcpip_init_done, &sem);
	  sys_sem_wait(sem);    // Block until the lwIP stack is initialized.
	  sys_sem_free(sem);    // Free the semaphore.
	#else
	  err_t  err_sem;
	  err_sem = sys_sem_new(&sem, 0); // Create a new semaphore.
	  tcpip_init(ethernet_tcpip_init_done, &sem);
	  sys_sem_wait(&sem);    // Block until the lwIP stack is initialized.
	  sys_sem_free(&sem);    // Free the semaphore.
	#endif
}

/*************************************************************************************************
 * ethernet_task
 *
 * Ethernet task, manages the interface, runs only once
 */
static portTASK_FUNCTION( ethernet_task, pvParameters )
{
	static const gpio_map_t MACB_GPIO_MAP =
	{
		{EXTPHY_MACB_MDC_PIN,     EXTPHY_MACB_MDC_FUNCTION   },
		{EXTPHY_MACB_MDIO_PIN,    EXTPHY_MACB_MDIO_FUNCTION  },
		{EXTPHY_MACB_RXD_0_PIN,   EXTPHY_MACB_RXD_0_FUNCTION },
		{EXTPHY_MACB_TXD_0_PIN,   EXTPHY_MACB_TXD_0_FUNCTION },
		{EXTPHY_MACB_RXD_1_PIN,   EXTPHY_MACB_RXD_1_FUNCTION },
		{EXTPHY_MACB_TXD_1_PIN,   EXTPHY_MACB_TXD_1_FUNCTION },
		{EXTPHY_MACB_TX_EN_PIN,   EXTPHY_MACB_TX_EN_FUNCTION },
		{EXTPHY_MACB_RX_ER_PIN,   EXTPHY_MACB_RX_ER_FUNCTION },
		{EXTPHY_MACB_RX_DV_PIN,   EXTPHY_MACB_RX_DV_FUNCTION },
		{EXTPHY_MACB_TX_CLK_PIN,  EXTPHY_MACB_TX_CLK_FUNCTION}
	};

	gpio_enable_module(MACB_GPIO_MAP, sizeof(MACB_GPIO_MAP) / sizeof(MACB_GPIO_MAP[0]));

	ethernet_init_LwIP();

	httpd_init();

	ssi_init();

	/* Kill this task. */
	vTaskDelete(NULL);
}

/*************************************************************************************************
 * ethernet_init
 *
 * Initializes the LwIP stack and starts all Ethernet-related tasks
 */
void ethernet_init(void)
{
	registry_initialize_key("IP Address 1", ETHERNET_CONF_IPADDR0, false);
	registry_initialize_key("IP Address 2", ETHERNET_CONF_IPADDR1, false);
	registry_initialize_key("IP Address 3", ETHERNET_CONF_IPADDR2, false);
	registry_initialize_key("IP Address 4", ETHERNET_CONF_IPADDR3, false);

	/* Spawn the Sentinel task. */
	xTaskCreate( ethernet_task, ( const char * const) "ETHLAUNCH",
	             configMINIMAL_STACK_SIZE, NULL,  tskIDLE_PRIORITY + 1 , ( xTaskHandle * )NULL );
}

