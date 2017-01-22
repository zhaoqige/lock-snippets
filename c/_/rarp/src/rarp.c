/*
 * rarp-client
 * verified by Qige Zhao <qige@6harmonics.com>
 * Updated on: Jun 29, 2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <time.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/if_arp.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#include <arpa/inet.h>


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


void print_mac(unsigned char * mac_addr)
{
	int i;
	for (i =0; i < 6; ++i)
	{
		printf("%02X", mac_addr[i]);
		if (i != 5)
			printf(":");
	}
	printf("\n");
}
void print_ip(unsigned char * ip_addr)
{
	int i;
	for (i =0; i < 4; ++i)
	{
		printf("%d", ip_addr[i]);
		if (i != 3)
			printf(".");
	}
	printf("\n");
}

void get_local_addr(unsigned char* mac, u_int32_t *ip)
{
	struct ifreq ifreq1;
	int sockfd;
	struct sockaddr_in* psockaddr_in = NULL;


	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Unable to create socket for geting the mac address");
		exit(1);
	}
	strcpy(ifreq1.ifr_name, "br-lan");

	if (ioctl(sockfd, SIOCGIFHWADDR, &ifreq1) < 0) {
		perror("Unable to get the mac address");
		exit(1);
	}
	memcpy(mac, ifreq1.ifr_hwaddr.sa_data, 6);
	if (ioctl(sockfd, SIOCGIFADDR, &ifreq1) < 0) {
		perror("Unable to get the ip address");
		exit(1);
	}

	psockaddr_in = (struct sockaddr_in*)&ifreq1.ifr_addr;
	*ip = psockaddr_in->sin_addr.s_addr;
	//print_ip((unsigned char*)ip);
	close(sockfd);
}


int main(int argc, char* argv[])
{
	int i;
	int timeout = 2;
	int optval = 1;
	int sockfd;   /* socket */
	struct sockaddr addr;  /* for interface name */
	struct arp_msg arp;
	fd_set  fdset;
	struct timeval tm;
	time_t  prevTime;
	u_int32_t  ip;
	char  buff[2000];

	unsigned char mac[6];
	unsigned char dmac[6];

	char interface[] = "br-lan";

	if (argc != 2) {
		printf("Usage: %s 00:5E:AC:00:5E:AC\n", argv[0]);
		return 0;
	}

	get_local_addr(mac, &ip);

	for (i = 0; i < 6; ++i) {
		strncpy(buff, argv[1]+3*i, 2);
		buff[3] = '\0';
		dmac[i] = strtol(buff, (char**)NULL, 16);
	}
	if ((sockfd = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_RARP))) == -1) {
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
	strcpy(addr.sa_data, interface);

	/* send rarp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6); /* MAC DA */
	memcpy(arp.ethhdr.h_source, mac, 6);  	/* MAC SA */
	arp.ethhdr.h_proto = htons(ETH_P_RARP); /* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);  		/* hardware type */
	arp.ptype = htons(ETH_P_IP);   			/* protocol type (ARP message) */
	arp.hlen = 6;     						/* hardware address length */
	arp.plen = 4;     						/* protocol address length */
	arp.operation = htons(3);  				/* RARP request code */
	*((u_int *) arp.sndr_ipaddr) = ip;   	/* source IP address */
	memcpy(arp.sndr_haddr, mac, 6);   		/* source hardware address */
	memcpy(arp.tgt_haddr, dmac, 6);

	if (sendto(sockfd, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0) {
		perror("Unabele to send arp request");
		return 0;
	}

	/* wait arp reply, and check it */
	tm.tv_usec = 0;
	time(&prevTime);
	while (timeout > 0) {
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		tm.tv_sec = timeout;
		if (select(sockfd + 1, &fdset, (fd_set *) NULL,
				(fd_set *) NULL, &tm) < 0) {
			//printf("Error on ARPING request:");
			if (errno != EINTR) {
				//rv = 0;
			}
		} else if (FD_ISSET(sockfd, &fdset)) {
			if (recv(sockfd, &arp, sizeof(arp), 0) < 0) {
				//perror("Unable get valid rarp response");
				//rv = 0;
			}
			if (arp.operation == htons(4) &&
					bcmp(arp.tgt_haddr, mac, 6) == 0) {
				//printf("Valid rarp reply receved for this address\n");
				//print_mac(arp.sHaddr);
				print_ip(arp.sndr_ipaddr);
				//rv = 0;
				break;
			}
		}
		timeout -= time(NULL) - prevTime;
		time(&prevTime);
	}
	close(sockfd);
	return 0;
}
