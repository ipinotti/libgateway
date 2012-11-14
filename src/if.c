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
 *  File name: if.c
 *  Created on: Jun 29, 2012
 *  Author: Thomas Del Grande <tgrande@pd3.com.br>
 *
 *  Network related functions
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <ifaddrs.h>

#include <net/if.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <if.h>

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

static int _dev_get_ctrlfd(void)
{
	int s_errno;
	int fd;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;

	s_errno = errno;

	fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;

	fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;

	errno = s_errno;
	perror("Cannot create control socket");

	return -1;
}

static int _in_cksum(unsigned short *buf, int sz)
{

	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return ans;
}

static int _dev_get_flags(char *dev, __u32 *flags)
{
	struct ifreq ifr;
	int fd;
	int err;

	strcpy(ifr.ifr_name, dev);
	fd = _dev_get_ctrlfd();

	if (fd < 0)
		return -1;

	err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (err) {
		//pr_error(1, "dev_get_flags %s SIOCGIFFLAGS", ifr.ifr_name);
		close(fd);
		return -1;
	}

	*flags = ifr.ifr_flags;
	close(fd);

	return err;
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

int libamg_dev_get_link_running(char *dev)
{
	__u32 flags;

	if (_dev_get_flags(dev, &flags) < 0)
		return (-1);

	return (flags & IFF_RUNNING);
}

int libamg_ping(char *ipaddr, char *intf)
{
	enum {
		DEFDATALEN = 56,
		MAXIPLEN = 60,
		MAXICMPLEN = 76,
		MAXPACKET = 65468,
		MAX_DUP_CHK = (8 * 128),
		MAXWAIT = 10,
		PINGINTERVAL = 1,
	/* 1 second */
	};

	struct sockaddr_in pingaddr;
	struct icmp *pkt;
	int pingsock, c, i, ret = 0;
	long arg;
	char packet[DEFDATALEN + MAXIPLEN + MAXICMPLEN];
	struct ifreq ifr;

	if_dbg("Pinging %s ... \n", ipaddr);

	if (libamg_dev_get_link_running(intf) <= 0)
		return -1;

	pingsock = socket(AF_INET, SOCK_RAW, 1); /* 1 == ICMP */
	pingaddr.sin_family = AF_INET;
	pingaddr.sin_addr.s_addr = inet_addr(ipaddr);

	/* Force source address to be of the interface we want */
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, intf, IFNAMSIZ - 1);
	ioctl(pingsock, SIOCGIFADDR, &ifr);

	if_dbg("Ping interface %s. IP is %s\n", intf,
			inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	if (bind(pingsock, (struct sockaddr*) &ifr.ifr_addr, sizeof(struct sockaddr_in)) == -1) {
		perror("bind");
		exit(2);
	}

	pkt = (struct icmp *) packet;
	memset(pkt, 0, sizeof(packet));

	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_cksum = _in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, DEFDATALEN + ICMP_MINLEN, 0, (struct sockaddr *) &pingaddr,
	                sizeof(pingaddr));

	/* Set non-blocking */
	if ((arg = fcntl(pingsock, F_GETFL, NULL)) < 0) {
		fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno));
		return -1;
	}

	arg |= O_NONBLOCK;

	if (fcntl(pingsock, F_SETFL, arg) < 0) {
		fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno));
		return -1;
	}

	sleep(1);

	/* listen for replies */
	i = 30; /* Number of attempts */
	while (i--) {
		struct sockaddr_in from;
		socklen_t fromlen = sizeof(from);

		c = recvfrom(pingsock, packet, sizeof(packet), 0, (struct sockaddr *) &from,
		                &fromlen);

		if_dbg("recvfrom returned %d bytes\n", c);

		if (c < 0) {
			usleep(10000);
			continue;
		}

		if (c >= 76) { /* ip + icmp */
			struct iphdr *iphdr = (struct iphdr *) packet;

			pkt = (struct icmp *) (packet + (iphdr->ihl << 2)); /* skip ip hdr */
			if (pkt->icmp_type == ICMP_ECHOREPLY) {
				ret = 1;
				break;
			}
		}
	}

	close(pingsock);
	return ret;
}

int libamg_arp_resolv(struct in_addr *ipaddr)
{
	char ipaddr_str[16];
	memset(ipaddr_str, 0, sizeof(ipaddr_str));
	inet_ntop(AF_INET, (const void *)ipaddr, ipaddr_str, sizeof(ipaddr_str));

	return libamg_ping(ipaddr_str, MAIN_IF);
}

int libamg_arp_get_hwaddr(struct in_addr *ipaddr, char *hwaddr)
{
	char line [128];
	char ipaddr_str[16];
	char hwaddr_str[18];
	FILE * file;

	memset(line, 0, sizeof(line));
	memset(ipaddr_str, 0, sizeof(ipaddr_str));
	memset(hwaddr_str, 0, sizeof(hwaddr_str));
	inet_ntop(AF_INET, (const void *)ipaddr, ipaddr_str, sizeof(ipaddr_str));

	if ((file = fopen(ARP_ENTRIES_FILE, "r")) == NULL) {
		syslog(LOG_ERR, "Could not open ARP Entries File at [%s]\n", ARP_ENTRIES_FILE);
		return -1;
	}

	while (fgets(line, sizeof(line), file) != NULL) {
		if (strstr(line, ipaddr_str)){
			if (sscanf(line, "%*s %*s %*s %s", hwaddr_str) < 1){
				fclose(file);
				return -1;
			}
		}
	}

	fclose(file);

	sscanf(hwaddr_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&hwaddr[0],
			&hwaddr[1],
			&hwaddr[2],
			&hwaddr[3],
			&hwaddr[4],
			&hwaddr[5]);

	return 0;
}

int libamg_get_default_gw_hwaddr(char *hwaddr)
{
	struct in_addr gw_ip_addr;

	memset(&gw_ip_addr, 0, sizeof(gw_ip_addr));
	memset(hwaddr, 0, 6);

	/* STEP 1: Get default gateway IP */
	if (libamg_get_default_gw_ipaddr(&gw_ip_addr) < 0)
		return -1;

	/* STEP 2: Do ARP on that IP */
	/* Force resolution of hardware address */
	if (libamg_arp_resolv(&gw_ip_addr) < 0)
		return -1;

	/* STEP 3: Check ARP table for HW address */
	if (libamg_arp_get_hwaddr(&gw_ip_addr, hwaddr) < 0) {
		syslog(LOG_ERR, "Could not get default gateway's MAC address\n");
		return -1;
	}

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

int libamg_get_interface_ipv4_addr(char *iface, struct in_addr *addr)
{
	struct ifreq req;

	if (_do_ioctl(iface, AF_INET, SIOCGIFADDR, &req) < 0)
		return -1;

	addr->s_addr = ((struct sockaddr_in *) &req.ifr_addr)->sin_addr.s_addr;

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
