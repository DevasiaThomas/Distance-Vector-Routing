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
 * UPDATE [Control Code: 0x03]
 */

#include <string.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

#define UPDATE_PAY_SIZE 4

struct __attribute__((__packed__)) UPDATE_PAY
{
	uint16_t rtrid;
    uint16_t cost;
};


void update_handler(int sock_index,char *cntrl_payload)
{
	struct UPDATE_PAY updatepay;
	int rtrindex,rtrdiff;
	memcpy(&updatepay, cntrl_payload, UPDATE_PAY_SIZE);
	for(int i=0;i<nrtr;i++){
		if(rtrid[pos[i]] == ntohs(updatepay.rtrid)){
			rtrindex = pos[i];
			break;
		}
	}
	//printf("DV entries before change\n");
	//for(int i=0;i<nrtr;i++)
		//printf("%d\t",dv[pos[i]]);
	rtrdiff = htons(updatepay.cost) - dv_init[rtrindex];
	dv_init[rtrindex]+=rtrdiff;
	//printf("\nDiff in cost%d\n",rtrdiff);
	//printf("DV entries after change\n");
	for(int i=0;i<nrtr;i++){
		if((pos[i]!=self)&&(nhop[pos[i]]==rtrindex)){
			//check overflow here
			if((dv[pos[i]]<(dv[pos[i]]+rtrdiff))&&(rtrdiff<(dv[pos[i]]+rtrdiff)))
				dv[pos[i]]+=rtrdiff;
		}
		//printf("%d\t",dv[pos[i]]);
	}
	//printf("\n");	
	char *cntrl_response_header;
	cntrl_response_header = create_response_header(sock_index, 3, 0, 0);
	sendALL(sock_index, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);										
}
