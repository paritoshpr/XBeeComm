//Author Paritosh Ramanan
#include <sys/signal.h>
#include "xbee.h"
#include "LogUtil.h"

char data[5]={0x48,0x45,0x4C,0x4C,0x4F};


int xbee_fd;
unsigned char stream_control;
pthread_t listener_thread,server_thread,reader_thread;
unsigned char dest_address[8];

int main(int argc, char *argv[])
{	
	 	
struct sigaction saio;
f_setowner_ex arg;
dest_address[0]=0x00;
dest_address[1]=0x13;
dest_address[2]=0xA2;
dest_address[3]=0x00;
dest_address[4]=0x40;
dest_address[5]=0xB2;
dest_address[6]=0x7B;
dest_address[7]=0xB0;

	//FILE *station_file, *arrival_time_file;
    
   	int stn_in_file[STATION_NUM];
    	memset(stn_in_file, -1, STATION_NUM*sizeof(int));
    	
	if ( (arrival_time_file  = fopen(argv[2], "ab+")) == NULL) 
	{
		error (LOG_APPL,"error opening file %s\n", argv[2]);
	}

	if ( (station_file  = fopen(argv[3], "ab+")) == NULL) 
	{
		error (LOG_APPL,"error opening file %s\n", argv[3]);
	}
 	
	//if( access( argv[1], F_OK ) == -1 ) { 
	//	system("echo BB-UART2 > /sys/devices/bone_capemgr.*/slots");
	//}
	
	xbee_fd = open (argv[1], O_RDWR | O_NOCTTY | O_SYNC);
	if (xbee_fd < 0)
	{
		error(LOG_APPL,"error %d opening serial port %s: %s\n", errno, argv[1], strerror (errno));
		return -1;
	}

	int signum = 12;

	
//	saio.sa_handler = signal_handler_IO;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL; 
	//sigaction(SIGIO,&saio,NULL);
	 
	/*
	memset(&saio, 0, sizeof(saio));
	sigemptyset(&set);
	saio.sa_sigaction = signal_handler_IO;
	saio.sa_mask = set;
	saio.sa_flags = SA_SIGINFO;

	if (sigaction(signum, &saio, NULL) != 0)
		printf("sigaction failed");
	*/

	//fcntl(xbee_fd, F_SETFL, FNDELAY);
	//fcntl(fd, F_SETOWN, getpid());
	//fcntl(xbee_fd, F_SETFL,  O_ASYNC );	

	set_interface_attribs (xbee_fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (xbee_fd,1);                // set no blocking
	//set_interface_attribs(fdw,B9600,0);
	//set_blocking(fdw,0);

//*******Sender specific routine**********	
	
	
        //int n;
       
        unsigned char einfo[52];
        //bzero(einfo,100);
//    //test an event
        EventInfo event;
        event.id = 0;
        event.sps=45;
        event.x = 2.0;
        event.y = 3.0;
        event.z = 4.0;
        event.ptime=5.0;
	event.stime=6.0;
	event.ptime=7.0;       
        memcpy(einfo, &event.id,2);
        memcpy(einfo+2,&event.sps,2);
        memcpy(einfo+4,&event.x,8);
        memcpy(einfo+12,&event.y,8);
        memcpy(einfo+20,&event.z,8);
        memcpy(einfo+28,&event.ptime,8);
	memcpy(einfo+36,&event.stime,8);
	memcpy(einfo+44,&event.amplitude,8);
//      
//        printf("Sent 36 bytes payload always\n\n");

        
	//char buf[128];
	//int k;

	//spawn listener thread here....
	
	arg.type= 0;
	//arg.pid=
	//printf("%d",arg.pid);
	
	if(!pthread_create(&reader_thread,NULL,xbeeReader,NULL)){
		info(LOG_APPL,"Reader thread created!!");
	}else{
		info(LOG_APPL,"Error creating listener thread");
	}
	
	if(!pthread_create(&listener_thread, NULL, &XBeeListener, NULL))
		{
			
			info(LOG_APPL,"Listener/tcp client spawned successfully\n");
			arg.pid=listener_thread;
			fcntl(xbee_fd,15,&arg);

		}
	else
		{
			error(LOG_APPL, "Error creating server thread\n");
			return 1;
		}
	//to relay messages to GUI
	/*if(!pthread_create(&listener_thread, NULL, &tcp_socket_client, NULL))
		{
			
			info(LOG_APPL,"Listener/tcp client spawned successfully\n");
			arg.pid=listener_thread;
			fcntl(xbee_fd,15,&arg);

		}
	else
		{
			error(LOG_APPL, "Error creating server thread\n");
			return 1;
		}
	//to relay messages to xbee
	if(!pthread_create(&server_thread, NULL, &tcp_socket_server, NULL))
		{
			info(LOG_APPL,"TCP server spawned successfully\n");

		}
	else
		{
			error(LOG_APPL, "Error creating server thread\n");
			return 1;
		}*/
	long int i= 1000000;
	unsigned char cb_data[3];
	cb_data[0]=0xAE;
	cb_data[1]=0x92;
	cb_data[2]=0x9A;
	unsigned char testbuffer[2480];
	//send_xbee_msg(xbee_fd,cb_data,3,dest_address,CB_TYPE);
	//printf("TID:%d",pthread_self());
	double d;int n;	
	while(1)
	{	
		d=5;
		while(d>0)
			{
				d=sleep(d);
			}
		n=send_xbee_msg(xbee_fd,einfo,52,dest_address,DP_TYPE);
		printf("Sent %d bytes\n",n);
	}
	//while(1)
	//{
		//printf("looping\n");
		/*	
		if( flags[DR_FLAG] == UP)
		{
					dataRead(e);			
					rpos = rpos +36;
					flags[DR_FLAG] = DN;
					if(rpos == MAX_BUF_LEN)
						{
							rpos = 0;
						}					

		}
		else if(flags[CB_FLAG] == UP)
		{
			printf("Base Station Address changed %X %X %X\n",dest_address[5],dest_address[6],dest_address[7]);
			flags[CB_FLAG] = DN;
			
		}*/
		//send_xbee_msg(fd, data,5, dest_address, DP_TYPE);	
		//sleep(3);
		/**/
		//printf("main loop");
		//send_xbee_msg(xbee_fd, cb_data,3, dest_address, CB_TYPE);
		//int n=send_xbee_msg(xbee_fd,einfo,36,dest_address,DP_TYPE);	
		//sendXbee(xbee_fd,testbuffer,200,dest_address,STR_TYPE);	
		//printf("sent %d bytes\n",n);
		
		/*i=sleep(30);
		while(i>0)
			{
				i=sleep(i);
			}
		event.sps++;
		memcpy(einfo+2,&event.sps,2);
		*/
		//printf("%d\n",cb_data[0]);
			//cb_data[0]++;
		
		//sleep(10);
		 

		//listen on a tcp socket here
		

	//}
     
//*********Keep the following for both cases**********************************	
	fclose(arrival_time_file);
	fclose(station_file);

	return 0;

}
