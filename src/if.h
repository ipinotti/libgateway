/*
 * if.h
 *
 *  Created on: Jul 12, 2012
 *      Author: tgrande
 */

#ifndef IF_H_
#define IF_H_

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
 * @return
 */
int libamg_get_interface_ipv4_addr(char *iface, unsigned int *addr);

/**
 * libamg_get_interface_ipv6_addr
 *
 * @param iface
 * @param addr
 * @return
 */
int libamg_get_interface_ipv6_addr(char *iface, struct in6_addr *addr);


#endif /* IF_H_ */
