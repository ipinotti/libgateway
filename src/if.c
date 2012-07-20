/*
 * libif.c
 *
 *  Created on: Jun 29, 2012
 *      Author: tgrande
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <ifaddrs.h>

#include <asterisk.h>
#include <asterisk/module.h>
#include <asterisk/logger.h>

#include "libnetlink/libnetlink.h"

static int _do_getifaddrs(char *iface, int family, struct sockaddr *addr)
{
	struct ifaddrs *ifaddr, *ifa;
	int ret = 0;

	getifaddrs(&ifaddr);

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, iface))
			continue;

		if (ifa->ifa_addr->sa_family != family)
			continue;

		/* If we've reached here, then it's a match */
		memcpy(addr, ifa->ifa_addr, sizeof(struct sockaddr));
		break;
	}

	if (ifa == NULL)
		ret = -1; /* Not found */

	freeifaddrs(ifaddr);

	return ret;
}

static int _do_ioctl(char *iface, int family, unsigned long int cmd, struct ifreq *ifr)
{
	int fd;
	int ret = 0;


	fd = socket(family, SOCK_DGRAM, 0);

	if (fd < 0) {
		syslog(LOG_ERR, "Could not open socket : %s", strerror(errno));
		return -1;
	}

	strcpy(ifr->ifr_name, iface);
	if (ioctl(fd, cmd, ifr) < 0) {
		syslog(LOG_ERR, "Could not do IOCTL : %s\n", strerror(errno));
		ret = -1;
	}

	close(fd);

	return ret;
}

static int _filter_default_gw(const struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{

	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr *tb[RTA_MAX + 1];
	struct in_addr *addr = (struct in_addr *) arg;


	if (n->nlmsg_type != RTM_NEWROUTE && n->nlmsg_type != RTM_DELROUTE) {
		fprintf(stderr, "Not a route: %08x %08x %08x\n", n->nlmsg_len, n->nlmsg_type,
		                n->nlmsg_flags);
		return -1;
	}

	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

	/* Not IPv4 family */
	if (r->rtm_family != AF_INET)
		return 0;

	/* Not MAIN table */
	if (tb[RTA_TABLE]) {
		int table = *(int *) RTA_DATA(tb[RTA_TABLE]);
		if (table != RT_TABLE_MAIN)
			return 0;
	}

	/* Not default route */
	if (tb[RTA_DST])
	       return 0;

	/* If we reached here, then we probably found it! */
	if (tb[RTA_GATEWAY])
		addr->s_addr = *(in_addr_t *) RTA_DATA(tb[RTA_GATEWAY]);

	return 0;
}

int libamg_get_default_gw_ipaddr(struct in_addr *ip)
{
	struct rtnl_handle rth;

	if (rtnl_open(&rth, 0) < 0) {
		perror("rtnl_open");
		return -1;
	}

	if (rtnl_wilddump_request(&rth, AF_INET, RTM_GETROUTE) < 0) {
		perror("rtnl_wilddump_request");
		goto rtnl_err;
	}

	if (rtnl_dump_filter(&rth, _filter_default_gw, (void *) ip, NULL, NULL) < 0) {
		perror("rtnl_dump_filter");
		goto rtnl_err;
	}

	rtnl_close(&rth);
	return 0;
rtnl_err:
	rtnl_close(&rth);
	return errno;
}


int libamg_get_default_gw_hwaddr(char *addr)
{
	struct in_addr gw_ip_addr;

	memset(addr, 0, 6);

	/* TODO STEP 1: Get default gateway IP */
	if (libamg_get_default_gw_ipaddr(&gw_ip_addr) < 0)
		return -1;

	/* TODO STEP 2: Do ARP on that IP */
	/* XXX Force resolution of hardware address
	if (libamg_arp_resolv(ipaddr) < 0)
		return -1;
	 */

	/* TODO STEP 3: Check ARP table for HW address */
	/* XXX
	if (libamg_get_peer_hwaddr(ipaddr, hwaddr) < 0) {
		syslog(LOG_ERROR, "Could not get default gateway's MAC address\n");
		return -1;
	}
	*/


	return 0;
}

int libamg_get_interface_hwaddr(char *iface, char *addr)
{
	struct ifreq req;

	if (_do_ioctl(iface, AF_INET, SIOCGIFHWADDR, &req) < 0)
		return -1;

	memcpy(addr, &req.ifr_hwaddr.sa_data, 6);

	return 0;
}

int libamg_get_interface_ipv4_addr(char *iface, unsigned int *addr)
{
	struct ifreq req;

	if (_do_ioctl(iface, AF_INET, SIOCGIFADDR, &req) < 0)
		return -1;

	*addr = ((struct sockaddr_in *) &req.ifr_addr)->sin_addr.s_addr;

	return 0;
}

int libamg_get_interface_ipv6_addr(char *iface, struct in6_addr *addr)
{
	struct sockaddr_in6 saddr;

	if (_do_getifaddrs(iface, AF_INET6, (struct sockaddr *) &saddr) < 0)
		return -1;

	memcpy(addr, &saddr.sin6_addr, sizeof(struct in6_addr));

	return 0;
}
