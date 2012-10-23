/*
 * Copyright (c) 2012 PD3 Tecnologia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * File name: if.h
 * Created on: Jun 29, 2012
 * Author: Thomas Del Grande <tgrande@pd3.com.br>
 *
 * Definitions for if.c
 */

#ifndef IF_H_
#define IF_H_

#include <errno.h>
#include <syslog.h>

/*
 * Debug Function
 */
#if defined (DEBUG_IF_SYSLOG)
#define if_dbg(x,...) \
		syslog(LOG_INFO,  "%s : %d => "x, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#elif defined (DEBUG_IF_PRINTF)
#define if_dbg(x,...) \
		printf("%s : %d => "x, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define if_dbg(x,...)
#endif

/*
 * General Defines
 */
#define MAIN_IF "eth0"
#define ARP_ENTRIES_FILE "/proc/net/arp"

/*
 * General Structures
 */



/*
 * Functions Declaration
 */

/**
 * libamg__get_default_gw_ipaddr
 *
 * Get system's default gateway IPv4 address
 *
 * @param ip Pointer to in_addr structure where data will be saved
 * @return 0 if success, -1 if error
 */
int libamg_get_default_gw_ipaddr(struct in_addr *ip);

/**
 * Get default gateway's hardware address
 *
 * @param addr Buffer where data will be saved
 * @return 0 if success, -1 if error
 */
int libamg_get_default_gw_hwaddr(char *addr);

/**
 * libamg_get_interface_hwaddr
 *
 * Get interface hardware address
 *
 * @param iface		Interface name
 * @param addr		Pointer to memory where hwaddr must be stored
 * @return 0 if success, -1 if error
 */
int libamg_get_interface_hwaddr(char *iface, char *addr);

/**
 * libamg_get_interface_ipv4_addr
 *
 * @param iface
 * @param addr
 * @return 0 if success, -1 if error
 */
int libamg_get_interface_ipv4_addr(char *iface,  struct in_addr *addr);

/**
 * libamg_get_interface_ipv6_addr
 *
 * @param iface
 * @param addr
 * @return 0 if success, -1 if error
 */
int libamg_get_interface_ipv6_addr(char *iface, struct in6_addr *addr);


/**
 * libamg_arp_get_hwaddr
 *
 * Get HW address related to IPv4 address in ARPs table
 *
 * @param ip_addr
 * @param hwaddr
 * @return 0 if success, -1 if error
 */
int libamg_arp_get_hwaddr(struct in_addr *ipaddr, char *hwaddr);

/**
 * libamg_arp_resolv
 *
 * Executes a ping targeting the provided ipaddr on the default interface
 * to fulfill the ARP table
 *
 * @param ipaddr
 * @return 0 if success, -1 if error
 */
int libamg_arp_resolv(struct in_addr *ipaddr);

/**
 * libamg_ping
 *
 * Executes a ping targeting the provided ipaddr on the intf (interface)
 *
 * @param ipaddr
 * @param intf
 * @return 0 if success, -1 if error
 */
int libamg_ping(char *ipaddr, char *intf);

/**
 * libamg_dev_get_link_running
 *
 * Return the status about link running of a given dev (interface)
 *
 * @param dev
 * @return 0 if not running, > 0 if running
 */
int libamg_dev_get_link_running(char *dev);


#endif /* IF_H_ */
