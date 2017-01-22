/*
 * wrarpd.c
 * rarp-server daemon for WMAC
 * verified by Qige Zhao <qige@6harmonics.com>, Jun 29, 2016
 * updated on: July 14, 2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>

#include <net/if.h>
#include <net/if_arp.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>


/*
 * args:
 * 		yiaddr 		- what IP to ping
 *  	ip 			- our ip
 *  	mac 		- our arp address
 *  	interface 	- interface to use
 * retn:
 * 		1  			addr free
 *  	0  			addr used
 *  	-1 			error
 */

/* FIXME: match response against chaddr */
struct arp_msg {
	struct ethhdr ethhdr;	/* Ethernet header */
	u_short htype;    		/* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;    		/* protocol type (must be ETH_P_IP) */
	u_char  hlen;    		/* hardware address length (must be 6) */
	u_char  plen;    		/* protocol address length (must be 4) */
	u_short operation;   	/* ARP opcode */
	u_char  sndr_haddr[6];  /* sender's hardware address */
	u_char  sndr_ipaddr[4]; /* sender's IP address */
	u_char  tgt_haddr[6];   /* target's hardware address */
	u_char  tgt_ipaddr[4];  /* target's IP address */
	u_char  pad[18];   		/* pad for min. Ethernet payload (60 bytes) */
};

/* miscellaneous defines */
#define MAC_BCAST_ADDR  	(uint8_t *) "\xff\xff\xff\xff\xff\xff"
#define OPT_CODE 			0
#define OPT_LEN 			1
#define OPT_DATA 			2
#define MIN(x, y)			(x < y ? x : y)

struct interface_info {
	char ifname[64];
	unsigned char ip[4];
	unsigned char mac[6];
};


struct interface_info if_info;

int get_iface_index(int fd, const char *ifname)
{
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        return (-1);
    }
    return ifr.ifr_ifindex;
}

char *text_mac(unsigned char *mac_addr)
{
	static char tmac[19] = {0};
	snprintf(tmac, sizeof(tmac)-1, "%02X:%02X:%02X:%02X:%02X:%02X",
			mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	return tmac;
}
char *text_ip(unsigned char *ip_addr)
{
	static char tip[16] = {0};
	snprintf(tip, sizeof(tip)-1, "%d.%d.%d.%d",
			ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
	return tip;
}

int equal_mac(unsigned char *mac1, unsigned char *mac2)
{
	int i;
	for (i = 0; i < 6; i ++) {
		if (mac1[i] != mac2[i])
			return 0;
	}
	return 1;
}


int set_mac(char *wmac)
{
	int i;
	unsigned int m[6] = {0};

	//+ covert to upper case
	for(i = 0; i < strlen(wmac); i ++) {
		wmac[i] = tolower(wmac[i]);
	}

	//+ scan to if_info.mac
	if (sscanf(wmac, "%02X:%02X:%02X:%02X:%02X:%02X",
			&m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) == -1) {
		return -1;
	}
	if (m[0] + m[1] + m[2] + m[3] + m[4] + m[5] < 1)
		return -2;

	//printf("%02x %02x %02x %02x %02x %02x\n", m[0], m[1], m[2], m[3], m[4], m[5]);
	//memcpy(&if_info.mac, &m, MIN(sizeof(if_info.mac), sizeof(m)));
	//print_mac(if_info.mac);
	for(i = 0; i < 6; i ++) {
		if_info.mac[i] = m[i];
	}

	return 0;
}

int set_ip(char *lip)
{
	unsigned int ip[4] = {0};
	if (sscanf(lip, "%d.%d.%d.%d",
			&ip[0], &ip[1], &ip[2], &ip[3]) == -1) {
		return -1;
	}

	if (ip[0] + ip[1] + ip[2] + ip[4] < 1)
		return -2;

	//printf("%d %d %d %d\n", ip[0], ip[1], ip[2], ip[3]);
	if_info.ip[0] = ip[0];
	if_info.ip[1] = ip[1];
	if_info.ip[2] = ip[2];
	if_info.ip[3] = ip[3];

	return 0;
}


int main(int argc, char **argv)
{
	int optval = 1;
	int sockfd;
	struct sockaddr_ll addr;  /* for interface name */
	struct arp_msg arp;
	struct arp_msg *p_arp;

	int nLen;
	char buffer[4096];


	//* read mac/ip from cli
	if (argc < 3) {
		printf("Usage: %s mac ip\n", argv[0]);
		return -9;
	}

	if (set_ip(argv[2]) || set_mac(argv[1])) {
		printf("Invalid mac or ip\n");
		return -8;
	}

	printf("Local: %s, %s\n", text_mac(if_info.mac), text_ip(if_info.ip));

	if ((sockfd = socket (PF_PACKET, SOCK_RAW, htons(ETH_P_RARP))) == -1) {
		printf("Could not open raw socket\n");
		return -1;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
			&optval, sizeof(optval)) == -1) {
		printf("Could not setsocketopt on raw socket\n");
		close(sockfd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = get_iface_index(sockfd, "br-lan");
	addr.sll_protocol = htons(ETH_P_ARP);

	memset(buffer, 0, sizeof(buffer));
	while ((nLen = recvfrom(sockfd, buffer, sizeof(buffer),
			MSG_TRUNC, NULL, NULL)) > 0) {
		p_arp = (struct arp_msg*) buffer;
		printf("-> RARP from %s, %s\n", text_mac(p_arp->sndr_haddr), text_ip(p_arp->sndr_ipaddr));

		if (p_arp->operation == htons(3) && equal_mac(if_info.mac, p_arp->tgt_haddr)) {
			/* send arp request */
			memset(&arp, 0, sizeof(arp));

			// MAC DA
			// MAC SA
			memcpy(arp.ethhdr.h_dest, p_arp->sndr_haddr, 6);
			//memcpy(arp.ethhdr.h_source, "\xd4\xca\x6d\x35\x0d\x95", 6);
			//memcpy(arp.ethhdr.h_source, MAC_BCAST_ADDR, 6);
			//memcpy(arp.ethhdr.h_source, if_info.mac, 6);
			memcpy(arp.ethhdr.h_source, p_arp->tgt_haddr, 6);

			// protocol type (Ethernet)
			// hardware type
			// protocol type (ARP message)
			arp.ethhdr.h_proto = htons(ETH_P_RARP);
			arp.htype = htons(ARPHRD_ETHER);
			arp.ptype = htons(ETH_P_IP);

			// hardware address length
			// protocol address length
			arp.hlen = 6;
			arp.plen = 4;

			// RARP reply code
			arp.operation = htons(4);

			// source IP address
			// source hardware address
			// target IP address
			memcpy(arp.sndr_ipaddr, if_info.ip, 4);
			memcpy(arp.sndr_haddr, p_arp->tgt_haddr, 6);
			//memcpy(arp.sndr_haddr, if_info.mac, 6);
			//memcpy(arp.sndr_haddr, "\xd4\xca\x6d\x35\x0d\x95", 6);
			memcpy(arp.tgt_ipaddr, p_arp->sndr_ipaddr, 4);
			memcpy(arp.tgt_haddr, p_arp->sndr_haddr, 6);

			if (sendto(sockfd, &arp, sizeof(arp), 0,
					(struct sockaddr *) &addr, sizeof(addr)) < 0) {
				printf("--> Error No: %d\n", errno);
				perror("--> Reply failed!");
			} else {
				printf("--> Reply to: %s\n", text_ip(p_arp->sndr_ipaddr));
			}
		} else {
			printf("-> Not for me.\n");
		}
	}

	close(sockfd);
	return 0;
}

