#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/time.h>

typedef enum {FALSE, TRUE} bool;

#define ERROR(err_msg) {perror(err_msg); exit(EXIT_FAILURE);}
#define INF 0xFFFF

/* https://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/ */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)])) // Interesting stuff to read if you are interested to know how this works

fd_set master_list, watch_list;
int head_fd;

struct timeval timeout, current, start[5];
int timerholder;
int timerholders[5];
int ctr[5];

uint16_t CONTROL_PORT;
int self; // to identify oneself(router)
uint16_t tval;
uint16_t nrtr;
uint16_t rtrid[5];
uint16_t rtrport[5];
uint32_t rtrip[5];
uint16_t dataport[5];
uint16_t nhop[5];
uint16_t dv[5];
uint16_t dv_init[5];
int pos [5];
int neighbor[5];
#endif
