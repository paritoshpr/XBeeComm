//Author Paritosh Ramanan
#ifndef XBEE_H
#define XBEE_H


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/signal.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "ArrivalTime.h"
#include <errno.h>
#include <sys/select.h>



#define _GNU_SOURCE 1

#define STATION_NUM 50
#define ALPHA 0.5


#define MAX_PKT_LEN 65536




#define DP_PLD_LEN 52
#define STAT_RESP_PLD_LEN 42
#define STRPLDLEN 200
#define STRHDRLEN 12
#define MAX_PLD_LEN 212


#define NO_OF_FLAGS 8


#define MAX_BUF_LEN 65536
#define DATA_BUF_LEN 5200
#define STR_BUF_LEN 21200
#define STATUS_BUF_LEN 420

#define UP 1
#define DN 0


/*

Flags

*/

#define DR_FLAG 0
#define CB_FLAG	1
#define STR_FLAG 2
#define STR_CTL_FLAG 3
#define STR_MSB_FLAG 4
#define STATUS_REQ_FLAG 5
#define STATUS_RESP_FLAG 6
#define RECORD_REQ_FLAG 7
#define REC_FLAG 8
#define REC_MSB_FLAG 9


/*

Types

*/

#define DP_TYPE 0
#define CB_TYPE 1
#define STR_TYPE 2
#define STR_CTL_TYPE 3
#define STR_MSB_TYPE 4
#define STATUS_REQ_TYPE 5
#define STATUS_RESP_TYPE 6
#define RECORD_REQ_TYPE 7
#define REC_TYPE 8
#define REC_MSB_TYPE 9


#define DP_FRAMELENGTH 69
#define STR_FRAMELENGTH 229

#define NO_OF_CLIENTS 8


extern int errno;

extern int stn_in_file[STATION_NUM];
extern int xbee_fd;
extern pthread_t listener_thread;
extern unsigned char dest_address[8];
extern unsigned char stream_control;
extern FILE *station_file, *arrival_time_file;
typedef struct {
    unsigned int id;
    double x,y,z;
}StationInfo;

typedef struct {
	int type;
	pid_t pid;
}f_setowner_ex;
//extern f_setowner_ex arg;

//extern EventInfo e;

//struct f_setowner_ex arg;


void* xbeeReader(void* param);
void signal_handler_IO (int status);
void dataRead(EventInfo *e);
void sendXbee(int fd, unsigned char* buffer,unsigned int length, unsigned char*dest_address, unsigned char msg_type);
unsigned int send_xbee_msg(int fd, unsigned char* buffer,unsigned int length, unsigned char*dest_address, unsigned char msg_type);
void* XBeeListener(void* param);
//void* handleClientMessages(void* param);
void* tcp_socket_server(void* param);
//void* tcp_socket_server_old(void* param);
void* tcp_socket_client(void* param);
int set_interface_attribs (int fd, int speed, int parity);
void set_blocking (int fd, int should_block);
void write_station_file(FILE *sta_file, int *status, EventInfo *arrvTimeInfo);
int sendToClient(unsigned char *, int);
int anyFlagUp();
#endif
//char data[5]={0x48,0x45,0x4C,0x4C,0x4F};

