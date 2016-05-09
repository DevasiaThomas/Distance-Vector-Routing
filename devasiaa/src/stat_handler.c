/**
 * @stats_handler
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
 * Handler for stat controls.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/queue.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "../include/global.h"
#include "../include/data_handler.h"
#include "../include/stats_handler.h"
#include "../include/network_util.h"
#include "../include/control_header_lib.h"

#define STATS_PACK_HDR_SIZE 4
struct __attribute__((__packed__)) STATS_PACK_HDR
{
	uint8_t tID;
    uint8_t TTL;
	uint16_t padding;
};


void new_stats_conn(uint8_t tID, uint8_t TTL, uint16_t initseq, int seqcount){
	statistics = malloc(sizeof(struct StatConn));
    statistics->tID = tID;
	statistics->TTL = TTL;
	statistics->initseq = initseq;
	statistics->seqcount = seqcount;
    LIST_INSERT_HEAD(&stats_conn_list, statistics, next);
}
void send_stats(int sock_index, uint8_t tID){
	LIST_FOREACH(statistics, &stats_conn_list, next)
        if(statistics->tID == tID){
			char *cntrl_response_header;
			cntrl_response_header = create_response_header(sock_index, 6, 0,(sizeof(uint16_t)*statistics->seqcount)+STATS_PACK_HDR_SIZE+CNTRL_RESP_HEADER_SIZE);
			struct STATS_PACK_HDR sph;
			sph.tID = statistics->tID;
			sph.TTL = statistics->TTL;
			sph.padding = 0;
			char * packet = (char *)malloc((sizeof(uint16_t)*statistics->seqcount)+STATS_PACK_HDR_SIZE+CNTRL_RESP_HEADER_SIZE);
			memcpy(packet,cntrl_response_header,CNTRL_RESP_HEADER_SIZE);
			memcpy(packet+CNTRL_RESP_HEADER_SIZE,&sph,STATS_PACK_HDR_SIZE);
			for(uint16_t i = 0,j=statistics->initseq;i <statistics->seqcount;i++,j++){
				uint16_t k =htons(j);
				memcpy(packet+STATS_PACK_HDR_SIZE+CNTRL_RESP_HEADER_SIZE+(i*sizeof(uint16_t)), &k,sizeof(uint16_t));
			}	
			sendALL(sock_index, packet, (sizeof(uint16_t)*statistics->seqcount)+STATS_PACK_HDR_SIZE+CNTRL_RESP_HEADER_SIZE);
			free(cntrl_response_header);
			free(packet);
			return;
		}
		char *cntrl_response_header;
		cntrl_response_header = create_response_header(sock_index, 6, 1, 0);
		sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
		free(cntrl_response_header);
}
void last_data_packet(int sock_index){
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 6, 0, 1036);
	char *packet = (char *)malloc(CNTRL_RESP_HEADER_SIZE+1036);
	memcpy(packet,cntrl_response_header,CNTRL_RESP_HEADER_SIZE);
	memcpy(packet+CNTRL_RESP_HEADER_SIZE,ldp,1036);
	sendALL(sock_index, packet, CNTRL_RESP_HEADER_SIZE+1036);
	free(cntrl_response_header);
	free(packet);
}
void penultimate_data_packet(int sock_index){
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 6, 0, 1036);
	char *packet = (char *)malloc(CNTRL_RESP_HEADER_SIZE+1036);
	memcpy(packet,cntrl_response_header,CNTRL_RESP_HEADER_SIZE);
	memcpy(packet+CNTRL_RESP_HEADER_SIZE,pdp,1036);
	sendALL(sock_index, packet, CNTRL_RESP_HEADER_SIZE+1036);
	free(cntrl_response_header);
	free(packet);
}
