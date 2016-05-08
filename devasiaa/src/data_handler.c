/**
 * @data_handler
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
 * Handler for the data plane.
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
#include "../include/network_util.h"

#define DATA_PACKET_HDR_SIZE 12
#define DATA_PACKET_PAY_SIZE 1024
#define NAME "FILE-"

struct __attribute__((__packed__)) DATA_PACKET_HDR
{
	uint32_t dIP;
	uint8_t tID;
    uint8_t TTL;
	uint16_t seq;
	uint8_t fin;
	uint8_t pad1;
	uint16_t pad2;
};

int new_data_conn(int sock_index)
{
    int fdaccept, caddr_len;
    struct sockaddr_in remote_data_addr;

    caddr_len = sizeof(remote_data_addr);
    fdaccept = accept(sock_index, (struct sockaddr *)&remote_data_addr, &caddr_len);
    if(fdaccept < 0)
        ERROR("accept() data failed");

    /* Insert into list of active data connections */
    connection2 = malloc(sizeof(struct DataConn));
    connection2->sockfd = fdaccept;
    LIST_INSERT_HEAD(&data_conn_list, connection2, next);

    return fdaccept;
}

void remove_data_conn(int sock_index)
{
    LIST_FOREACH(connection2, &data_conn_list, next) {
        if(connection2->sockfd == sock_index) LIST_REMOVE(connection2, next); // this may be unsafe?
        free(connection2);
    }

    close(sock_index);
}

bool isData(int sock_index)
{
    LIST_FOREACH(connection2, &data_conn_list, next)
        if(connection2->sockfd == sock_index) return TRUE;

    return FALSE;
}

void data_recv_hook(int sock_index){
	char *data_packet;int dIndex;
	struct DATA_PACKET_HDR dp_hdr;
    
    /* Get 1st data packet */
    data_packet = (char *) malloc(sizeof(char)*(DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE));
    bzero(data_packet, (DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE));

    if(recvALL(sock_index, data_packet, (DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE)) < 0){
        remove_data_conn(sock_index);
        free(data_packet);
        return;
    }
	memcpy(&dp_hdr,data_packet,DATA_PACKET_HDR_SIZE);
	for(int i=0;i<nrtr;i++){
		if(rtrip[pos[i]] == ntohl(dp_hdr.dIP)){
			dIndex = pos[i];
			break;
		}
	}
	dp_hdr.TTL -=1;
	int sentcount =0;
	if(dIndex == self){
		int fd;
		char *filename = (char *)malloc(sizeof(NAME)+3);
		char *filebuff = (char *)malloc(sizeof(char)*10*1024*1024);
		memcpy(filename,NAME,(sizeof(NAME)-1));
		//itoa(dp_hdr.tID,(filename+sizeof(NAME)-1),10);
		sprintf(filename+(sizeof(NAME)-1),"%d",dp_hdr.tID);
		memcpy(filebuff,data_packet+DATA_PACKET_HDR_SIZE,DATA_PACKET_PAY_SIZE);
		sentcount++;
		while(recvALL(sock_index, data_packet, (DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE)) > 0){
			memcpy(filebuff+(sentcount*DATA_PACKET_PAY_SIZE),data_packet+DATA_PACKET_HDR_SIZE,DATA_PACKET_PAY_SIZE);
			sentcount++;
		}
		if ((fd=open(filename, O_WRONLY|O_CREAT, 0644))<0)
			ERROR("FILE CREATION ZZ");
		for(int i=0;i<sentcount/10;i++){
			if(write(fd,filebuff+(i*10*1024),(10*1024))<0)
				ERROR("WRITING");
		}
		if(sentcount%10!=0){
			if(write(fd,filebuff+(sentcount*1024),((sentcount%10)*1024))<0)
				ERROR("Final write is wrong");
		}
		close(fd);
		remove_data_conn(sock_index);
	}
	else{
		int sd;
		struct sockaddr_in nxt;
		sd = socket(AF_INET, SOCK_STREAM, 0);
    	if(sd < 0)
       		ERROR(" data send socket() failed");
		bzero(&nxt, sizeof(nxt));

    	nxt.sin_family = AF_INET;
    	nxt.sin_addr.s_addr = htonl(rtrip[nhop[dIndex]]);
    	nxt.sin_port = htons(dataport[nhop[dIndex]]);
	
		if(connect(sd,(struct sockaddr *)&nxt,sizeof(nxt))<0)
			ERROR(" data send connect failed");
		memcpy(data_packet,&dp_hdr,DATA_PACKET_HDR_SIZE);
		sendALL(sd, data_packet, (DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE));
		sentcount ++;
		while(recvALL(sock_index, data_packet, (DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE)) < 0){
			memcpy(&dp_hdr,data_packet,DATA_PACKET_HDR_SIZE);
			dp_hdr.TTL-=1;
			memcpy(data_packet,&dp_hdr,DATA_PACKET_HDR_SIZE);
			sendALL(sd, data_packet, (DATA_PACKET_HDR_SIZE+DATA_PACKET_PAY_SIZE));
			sentcount++;
		}
		close(sd);
		remove_data_conn(sock_index);
	}

    /* Get IP from the header */
    

    free(data_packet);

    
    return;
}
