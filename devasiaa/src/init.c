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
#include <netinet/in.h>

#include "../include/global.h"


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
	int flag = 1;
	for(int i=nrtr;i>0;i--){
		if(!flag){
			break;
		}
		flag = 0;
		for(int j=0;j<i;j++){
			if(rtriddup[j] > rtriddup[j+1]){
				uint16_t temp1 = rtriddup[j];
				rtriddup[j] = rtriddup[j+1];
				rtriddup[j+1] = temp1;
				int temp2 = pos[j];
				pos[j] = pos[j+1];
				pos[j+1] = temp2;
				flag = 1;
			}
		}
	}							
}
