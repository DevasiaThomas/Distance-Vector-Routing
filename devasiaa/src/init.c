/**
 * @author
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
 * INIT [Control Code: 0x01]
 */

#include <string.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/connection_manager.h"
#include "../include/data_handler.h"
#include "../include/routing_handler.h"


#define INIT_PAYINFO_SIZE 4
#define INIT_RTRINFO_SIZE 12


struct __attribute__((__packed__)) INIT_PAYINFO
{
	uint16_t number;
    uint16_t upi;
};
struct __attribute__((__packed__)) INIT_RTRINFO
{
	uint16_t rtrid;
    uint16_t rtrport;
	uint16_t dataport;
	uint16_t cost;
	uint32_t rtrip;
};



void init_handler(char *cntrl_payload)
{
	struct INIT_PAYINFO payinfo;
	struct INIT_RTRINFO rtrinfo;
	uint16_t rtriddup[5];
	memcpy(&payinfo, cntrl_payload, INIT_PAYINFO_SIZE);
	nrtr = ntohs(payinfo.number);
	tval = ntohs(payinfo.upi);
	for(int i =0; i<nrtr; i++){
		memcpy(&rtrinfo, cntrl_payload + INIT_PAYINFO_SIZE + (i*INIT_RTRINFO_SIZE), INIT_RTRINFO_SIZE);
		pos[i] = i;
		rtrid[i] = ntohs(rtrinfo.rtrid);
		//printf("rtrid:%d\t",rtrid[i]);
		rtriddup[i] = ntohs(rtrinfo.rtrid);
		rtrport[i] = ntohs(rtrinfo.rtrport);
		//printf("rtrport:%d\t",rtrport[i]);
		rtrip[i] = ntohl(rtrinfo.rtrip);
		//printf("rtrip:%d\t",rtrip[i]);
		dataport[i] = ntohs(rtrinfo.dataport);
		//printf("dataport:%d\t",dataport[i]); 
		dv[i] = ntohs(rtrinfo.cost);
		//printf("Cost to rtr:%d\n",dv[i]);
		if(dv[i] == INF){
			//printf("Yay INF!!!\n");
			nhop[i] = INF;
			neighbor[i] = 0;
		}
		else{
			//printf("Yay NOT INF!!!\n");
			nhop[i] = i;
			neighbor[i] = 1;
			if(dv[i] == 0){
				neighbor[i] = 0;
				self = i;
				//printf("Found myself\n");
			}
		}	
	}
	//for(int i=0;i<nrtr;i++)
		//printf("Pos[i]:%d\n",pos[i]);

	for(int i=(nrtr-1);i>0;i--){
		for(int j=0;j<i;j++){
			if(rtriddup[j] > rtriddup[j+1]){
				uint16_t temp1 = rtriddup[j];
				rtriddup[j] = rtriddup[j+1];
				rtriddup[j+1] = temp1;
				int temp2 = pos[j];
				pos[j] = pos[j+1];
				pos[j+1] = temp2;
			}
		}
	}
	//////////////////////////////////////////////////Routing Port
    struct sockaddr_in router_addr;
    socklen_t addrlen = sizeof(router_addr);

    router_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(router_socket < 0)
        ERROR("router socket() failed");

    /* Make socket re-usable */
    if(setsockopt(router_socket, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("router setsockopt() failed");

    bzero(&router_addr, sizeof(router_addr));

    router_addr.sin_family = AF_INET;
    router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    router_addr.sin_port = htons(rtrport[self]);

    if(bind(router_socket, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0)
        ERROR("router bind() failed");

	FD_SET(router_socket, &master_list);
    if(head_fd < router_socket)
		head_fd = router_socket;
	/////////////////////////////////////////////////////
	///////////////////////////////////////////////////// DATA PORT
    struct sockaddr_in data_addr;
    addrlen = sizeof(data_addr);

    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(data_socket < 0)
        ERROR("data socket() failed");

    /* Make socket re-usable */
    if(setsockopt(data_socket, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
        ERROR("data setsockopt() failed");

    bzero(&data_addr, sizeof(data_addr));

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = htons(dataport[self]);

    if(bind(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0)
        ERROR("bind() failed");

    if(listen(data_socket, 20) < 0)
        ERROR("listen() failed");

    LIST_INIT(&data_conn_list);
	FD_SET(data_socket, &master_list);
    if(head_fd < data_socket)
		head_fd = data_socket;	
	//////////////////////////////////////////////////////
	//////////////////////////////////////////////////////
	send_conn(router_socket);					
}

