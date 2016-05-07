#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/queue.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "../include/global.h"
#include "../include/data_handler.h"
#include "../include/network_util.h"


int new_data_conn(int sock_index)
{
    int fdaccept, caddr_len;
    struct sockaddr_in remote_data_addr;

    caddr_len = sizeof(remote_data_addr);
    fdaccept = accept(sock_index, (struct sockaddr *)&remote_data_addr, &caddr_len);
    if(fdaccept < 0)
        ERROR("accept() data failed");

    /* Insert into list of active data connection2s */
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

bool data_recv_hook(int sock_index){
	(void) sock_index;
}
