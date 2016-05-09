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
 * SENDFILE [Control Code: 0x05]
 */

#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"
#include "../include/sendfile.h"
#include "../include/stats_handler.h"

#define SENDFILE_HDR_SIZE 8
#define DATA_PACKET_SIZE 12
#define FILE_DATA_SIZE 1024
#define FIN_SET 128

struct __attribute__((__packed__)) SENDFILE_HDR
{
	uint32_t dIP;
    uint8_t TTL;
	uint8_t tID;
	uint16_t seq;
};
struct __attribute__((__packed__)) DATA_PACKET
{
	uint32_t dIP;
	uint8_t tID;
    uint8_t TTL;
	uint16_t seq;
	uint8_t fin;
	uint8_t pad1;
	uint16_t pad2;
};

void sendfile_handler(int sock_index,char *cntrl_payload,int payload_len)
{
	struct SENDFILE_HDR sendfile_hdr;
	int dIndex;
	memcpy(&sendfile_hdr, cntrl_payload, SENDFILE_HDR_SIZE);
	for(int i=0;i<nrtr;i++){
		if(rtrip[pos[i]] == ntohl(sendfile_hdr.dIP)){
			dIndex = pos[i];
			break;
		}
	}
	char buff[FILE_DATA_SIZE];
	char *filename = (char *) malloc(sizeof(char)*(payload_len-SENDFILE_HDR_SIZE+1));
	memcpy(filename, cntrl_payload+SENDFILE_HDR_SIZE, (payload_len-SENDFILE_HDR_SIZE));
	filename[(payload_len-SENDFILE_HDR_SIZE)] = '\0';
	printf("Filename%s\n",filename);
	int fd,sd;
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

	struct DATA_PACKET dp;
	char *data_packet = (char *) malloc(sizeof(char)*(DATA_PACKET_SIZE+FILE_DATA_SIZE));
	int sent_count = 0;
	if((fd = open(filename, O_RDONLY))< 0){
		ERROR("Cannot Open File");
	}
	else{
		int h =0;
		while( read(fd, buff, FILE_DATA_SIZE) > 0 ){
			if(h != 0){
				sendALL(sd, data_packet, DATA_PACKET_SIZE+FILE_DATA_SIZE);
				memcpy(pdp,data_packet,1036);
				sent_count++;
				dp.seq = htons(ntohs(dp.seq)+1);
				memcpy(data_packet, &dp, DATA_PACKET_SIZE);
				memcpy(data_packet+DATA_PACKET_SIZE,buff,FILE_DATA_SIZE);
			}
			else{
				h=1;
				dp.dIP = sendfile_hdr.dIP;
				dp.tID = sendfile_hdr.tID;
				dp.seq = sendfile_hdr.seq;
				dp.TTL = sendfile_hdr.TTL;
				dp.fin = 0;
				dp.pad1 = 0;
				dp.pad2 = 0;
				memcpy(data_packet, &dp, DATA_PACKET_SIZE);
				memcpy(data_packet+DATA_PACKET_SIZE, buff,FILE_DATA_SIZE);
			}
		}
		dp.fin = FIN_SET;
		memcpy(data_packet, &dp, DATA_PACKET_SIZE);
		sendALL(sd, data_packet, DATA_PACKET_SIZE+FILE_DATA_SIZE);
		memcpy(ldp,data_packet,1036);
		sent_count++;
		close(fd);
	}
	new_stats_conn(sendfile_hdr.tID, sendfile_hdr.TTL, ntohs(sendfile_hdr.seq), sent_count);
	printf("File SENT in %d chunks\n", sent_count);
	close(sd);	
	free(filename);
	free(data_packet);
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 5, 0, 0);
	sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);										
}
