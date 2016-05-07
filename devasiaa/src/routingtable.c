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
 * AUTHOR [Control Code: 0x02]
 */

#include <string.h>
#include <netinet/in.h>

#include "../include/global.h"
#include "../include/control_header_lib.h"
#include "../include/network_util.h"

struct __attribute__((__packed__)) RTABLE_PAYLOAD
{
	uint16_t rtrid;
    uint16_t padding;
	uint16_t nxthop;
	uint16_t cost;
};

void rtable_response(int sock_index)
{
	struct RTABLE_PAYLOAD rtable;
	uint16_t payload_len, response_len;
	char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;

	payload_len = sizeof(struct RTABLE_PAYLOAD)*nrtr;
	cntrl_response_payload = (char *) malloc(payload_len);

	for(int i=0;i<nrtr;i++){
		//printf("i:%d\n",i);
		//printf("posi:%d\n",pos[i]);
		rtable.rtrid = htons(rtrid[pos[i]]);
		//printf("Rtrid:%d\t",rtrid[pos[i]]);
		rtable.padding = 0;
		rtable.nxthop = (nhop[pos[i]]!=INF)?(htons(rtrid[nhop[pos[i]]])):(htons(INF));
		//printf("NXThop:%d\t",ntohs(rtable.nxthop));
		rtable.cost = htons(dv[pos[i]]);
		//printf("COST:%d\n",dv[pos[i]]);
		memcpy(cntrl_response_payload+(i*sizeof(struct RTABLE_PAYLOAD)), &rtable, (payload_len/nrtr));
	}
	printf("...%s...\n",cntrl_response_payload);
	cntrl_response_header = create_response_header(sock_index, 2, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
}
