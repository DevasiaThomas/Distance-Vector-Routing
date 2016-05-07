/**
 * @connection_manager
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
 * Connection Manager listens for incoming connections/messages from the
 * controller and other routers and calls the desginated handlers.
 */

#include <sys/select.h>

#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/routing_handler.h"


void main_loop()
{
    int selret, sock_index, fdaccept, ctr[5];

    while(TRUE){
        watch_list = master_list;
        selret = pselect(head_fd+1, &watch_list, NULL, NULL,((tval>0)?(&timeout):NULL),NULL);

        if(selret < 0)
            ERROR("select failed.");
		if(selret == 0){
			if(timerholder == self){
				send_conn(router_socket);
				clock_gettime(CLOCK_MONOTONIC, &current);
				start[self] = current;
			}
			else{
				ctr[timerholder]++;
				if(ctr[timerholder]==3){
					for(int i=0;i<nrtr;i++){
						if((pos[i]!=self)&&(nhop[pos[i]]==timerholder)){
							dv[pos[i]] = INF;
							nhop[pos[i]] = INF;
						}
					}
					timerholders[timerholder] = 0;
				}
				else{
					clock_gettime(CLOCK_MONOTONIC, &current);
					start[timerholder] = current;
				}
			}
			clock_gettime(CLOCK_MONOTONIC, &current);
			int flag =0, min =tval;
			for(int i=0;i<nrtr;i++){
				if((i!=timerholder)&&(timerholders[i])){
					struct timeval diff;
					diff.tv_sec = current.tv_sec - start[i].tv_sec;
					if((tval-diff.tv_sec) < 0){
						ctr[i]++;
						if(ctr[i]==3){
							for(int j=0;j<nrtr;j++){
								if((pos[j]!=self)&&(nhop[pos[j]]==i)){
									dv[pos[j]] = INF;
									nhop[pos[j]] = INF;
								}
							}
							timerholders[i] = 0;
						}
						else{
							clock_gettime(CLOCK_MONOTONIC, &current);
							start[i] = current;
						}
					}
					else{
						flag =1;
						if(min > (tval-diff.tv_sec)){
							min = tval-diff.tv_sec;
							timerholder = i;
						}
					}
				}
			}
			if(!flag){
				timerholder = self;
				timeout.tv_sec = tval;
				timeout.tv_nsec = 0;
			}
			else{
				timeout.tv_sec = min;
				timeout.tv_nsec = 0;
			}
		}
		else{
       		/* Loop through file descriptors to check which ones are ready */
        	for(sock_index=0; sock_index<=head_fd; sock_index+=1){

            	if(FD_ISSET(sock_index, &watch_list)){

                /* control_socket */
                	if(sock_index == control_socket){
                	    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    	FD_SET(fdaccept, &master_list);
                    	if(fdaccept > head_fd) head_fd = fdaccept;
                	}

                	/* router_socket */
                	else if(sock_index == router_socket){
                	    printf("In router select\n");
						read_conn(sock_index);
                	}

                	/* data_socket */
                	else if(sock_index == data_socket){
                	    //new_data_conn(sock_index);
                	}

                	/* Existing connection */
                	else{
                	    if(isControl(sock_index)){
                	        if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                	    }
                	    //else if isData(sock_index);
                	    else ERROR("Unknown socket index");
                	}
            	}
        	}
    	}
	}
}
void init()
{
    control_socket = create_control_sock();

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;
	tval = 0;
	self = 0;
    main_loop();
}
