#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>

#include "../include/global.h"
#include "../include/routing_handler.h"

#define RTR_UPDATE_HDR_SIZE 8
#define RTR_UPDATE_PAY_SIZE 12

struct __attribute__((__packed__)) RTR_UPDT_HDR
{
	uint16_t numrtr;
    uint16_t sport;
	uint32_t sip;
};

struct __attribute__((__packed__)) RTR_UPDT_PAY
{
	uint32_t rip;
    uint16_t rport;
	uint16_t padding;
	uint16_t rid;
	uint16_t rcost;
};
void read_conn(int sock_index)
{
	struct RTR_UPDT_HDR hdr;
	struct RTR_UPDT_PAY pay;
    int totalsize = RTR_UPDATE_HDR_SIZE + (nrtr * RTR_UPDATE_PAY_SIZE);
	char *total = (char *) malloc(totalsize);
 	struct sockaddr_in other;int otherlen = sizeof(other);
	if(recvfrom(sock_index,total,totalsize,0,(struct sockaddr *)&other,&otherlen)==-1){
		ERROR("recvfrom got !@#!!@#");
	}
	memcpy(&hdr, total, RTR_UPDATE_HDR_SIZE);
	for(int i =0;i<nrtr;i++){
		memcpy(&pay, total+ RTR_UPDATE_HDR_SIZE + (i*RTR_UPDATE_PAY_SIZE), RTR_UPDATE_PAY_SIZE);
	}	
	printf("numrtr%d\tsport%d\tsip%d\trip%d\trport%d\trid%d\trcost%d\n",ntohs(hdr.numrtr),ntohs(hdr.sport),ntohl(hdr.sip),ntohl(pay.rip),ntohs(pay.rport),ntohs(pay.rid),ntohs(pay.rcost));	
}

void send_conn(int sock_index)
{
    struct RTR_UPDT_HDR hdr;
	struct RTR_UPDT_PAY pay;
    int totalsize = RTR_UPDATE_HDR_SIZE + (nrtr * RTR_UPDATE_PAY_SIZE);
	char *total = (char *) malloc(totalsize);
	hdr.numrtr = htons(nrtr);
	hdr.sport = htons(rtrport[self]);
	hdr.sip = htonl(rtrip[self]);
	memcpy(total, &hdr, RTR_UPDATE_HDR_SIZE);
	for(int i =0;i<nrtr;i++){
		pay.rip = htonl(rtrip[pos[i]]);
		pay.rport = htons(rtrport[pos[i]]);
		pay.padding = 0;
		pay.rid = htons(rtrid[pos[i]]);
		pay.rcost = htons(dv[pos[i]]);
		memcpy(total+ RTR_UPDATE_HDR_SIZE + (i*RTR_UPDATE_PAY_SIZE), &pay, RTR_UPDATE_PAY_SIZE);
	} 	
	struct sockaddr_in other;
	for(int i=0;i<nrtr;i++)
		{
			if(neighbor[pos[i]] != 1){
				continue;
			}
			other.sin_family = AF_INET;
			other.sin_port = htons(rtrport[pos[i]]);
			other.sin_addr.s_addr = htonl(rtrip[pos[i]]);
			int otherlen = sizeof(other);
			if(sendto(sock_index,total,totalsize,0,(struct sockaddr *)&other,otherlen)==-1){
				ERROR("sendto got !@#!!@#");
			}
		}		
}

