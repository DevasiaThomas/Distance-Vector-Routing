/**
 * @routing_handler
 * @author  Devasia Antony Muthalakuzhy Thomas <devasiaa@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Handler for the routing plane.
 */
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
	int srtr;
    int totalsize = RTR_UPDATE_HDR_SIZE + (nrtr * RTR_UPDATE_PAY_SIZE);
	char *total = (char *) malloc(totalsize);
 	struct sockaddr_in other;int otherlen = sizeof(other);
	if(recvfrom(sock_index,total,totalsize,0,(struct sockaddr *)&other,&otherlen)==-1){
		ERROR("recvfrom got !@#!!@#");
	}
	memcpy(&hdr, total, RTR_UPDATE_HDR_SIZE);
	for(int i =0;i<nrtr;i++){
		if(ntohl(hdr.sip) == rtrip[i]){
			srtr = i;
			break;
		}
	}
	for(int i =0;i<nrtr;i++){
		memcpy(&pay, total+ RTR_UPDATE_HDR_SIZE + (i*RTR_UPDATE_PAY_SIZE), RTR_UPDATE_PAY_SIZE);
		if((ntohs(pay.rcost)<(dv[srtr] + ntohs(pay.rcost)))&&(dv[srtr]<(dv[srtr] + ntohs(pay.rcost)))){
			if(dv[pos[i]] > (dv[srtr] + ntohs(pay.rcost))){
				dv[pos[i]] = dv[srtr] + ntohs(pay.rcost);
				nhop[pos[i]] = srtr;
			}
		}
	}
	timerholders[srtr] = 1;
	gettimeofday(&current,NULL);
	//clock_gettime(CLOCK_MONOTONIC, &current);
	ctr[srtr] = 0;
	start[srtr] = current;	
	//printf("numrtr%d\tsport%d\tsip%d\trip%d\trport%d\trid%d\trcost%d\n",ntohs(hdr.numrtr),ntohs(hdr.sport),ntohl(hdr.sip),ntohl(pay.rip),ntohs(pay.rport),ntohs(pay.rid),ntohs(pay.rcost));	
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
			//else{printf("Sent some stuff\n");}
		}		
}

