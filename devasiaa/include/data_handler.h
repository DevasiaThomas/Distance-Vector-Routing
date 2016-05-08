#ifndef DATA_HANDLER_H_
#define DATA_HANDLER_H_
#include<sys/queue.h>

struct DataConn
{
    int sockfd;
    LIST_ENTRY(DataConn) next;
}*connection2, *conn_temp2;
LIST_HEAD(DataConnsHead, DataConn) data_conn_list;

int new_data_conn(int sock_index);
bool isData(int sock_index);
void data_recv_hook(int sock_index);

#endif
