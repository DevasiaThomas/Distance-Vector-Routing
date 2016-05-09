#ifndef STATS_HANDLER_H_
#define STATS_HANDLER_H_
#include<sys/queue.h>

#define TOTAL_DP_SIZE 1036
struct StatConn
{
    uint8_t tID;
	uint8_t TTL;
	uint16_t initseq;
	int seqcount;
    LIST_ENTRY(StatConn) next;
}*statistics;
LIST_HEAD(StatConnsHead, StatConn) stats_conn_list;


char ldp[TOTAL_DP_SIZE];
char pdp[TOTAL_DP_SIZE];

void new_stats_conn(uint8_t tID, uint8_t TTL, uint16_t initseq, int seqcount);
void send_stats(int sock_index, uint8_t tID);
void last_data_packet(int sock_index);
void penultimate_data_packet(int sock_index);

#endif
