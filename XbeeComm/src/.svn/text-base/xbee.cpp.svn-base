//Author Paritosh Ramanan

#include "xbee.h"
#include "LogUtil.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int client_socket[NO_OF_CLIENTS+1];
//defunct 
int client_sockfd[NO_OF_CLIENTS+1];
int numclients;
volatile int flags[NO_OF_FLAGS];

volatile int numPacketsLost = 0;
volatile int wpos=0;
volatile int rpos=0;

volatile int strwpos=0;
volatile int strrpos=0;

volatile int statwpos=0;
volatile int statrpos=0;

volatile unsigned char checksum =0;
sig_atomic_t flag=0;
//int i;
unsigned char dataBuffer[DATA_BUF_LEN];
unsigned char tempDestBuf[3];
unsigned char streamDataBuffer[STR_BUF_LEN];
unsigned char statusDataBuffer[STATUS_BUF_LEN];
unsigned char uartData;
unsigned char inputBuffer[MAX_BUF_LEN];
int inputBufferIndex = 0;
unsigned char src_add[3];
int framelength;
FILE *station_file, *arrival_time_file;
unsigned char control;

pthread_mutex_t clients;
pthread_mutex_t cli_list_excl;
//unsigned char dest_address[8];
sigset_t mskvar_1;
sem_t interrupt_sem;

int stn_in_file[STATION_NUM];	
void sendXbee(int fd, unsigned char* buffer,unsigned int length, unsigned char*dest_address, unsigned char msg_type)
{
	
	double start_time,timesum,curr_time;
	unsigned int length_read;
	int ret,sps_rate;
	unsigned char MSB;
	unsigned char LSB;
	unsigned int numchars;

	unsigned char splicedBuffer[256];
	
	if(msg_type == STR_TYPE)
		{
			memcpy(&start_time,buffer,8);
			memcpy(&sps_rate,buffer+8,2);
			
			//memcpy(splicedBuffer,buffer,10);
			length_read=10;
			curr_time=0.0;
			timesum=0.0;
			
			//length = length -10;
		}

	numchars=0;	
	while(length_read<length)
		{	//splicedBuffer[2]=seq_no;	
			numchars=(length-length_read)>STRPLDLEN?STRPLDLEN:(length-length_read);
			curr_time=start_time+timesum;
			memcpy(splicedBuffer,&curr_time,8);
			memcpy(splicedBuffer+8,&sps_rate,2);
			LSB=numchars & 0xFF;
			MSB=(numchars & 0xFF00)>>8;
			splicedBuffer[10]=MSB;
			splicedBuffer[11]=LSB;
			memset((splicedBuffer+12),0,STRPLDLEN);
			memcpy((splicedBuffer+12),buffer+length_read,numchars);
			//printf("numchars : %d, length read : %d, total size : %d\n",numchars,length_read,(STRHDRLEN+STRPLDLEN));
			
			ret=send_xbee_msg(fd,splicedBuffer,(STRPLDLEN+STRHDRLEN),dest_address,msg_type);
			info(LOG_APPL,"send status %d\n",ret);
			length_read=length_read+numchars;
			timesum=timesum+(1.0/(double)sps_rate)*((double)(numchars)/4.0);
			info(LOG_APPL,"TimeSum = %f",timesum);
			//seq_no=seq_no+1;
		}
	
}



void signal_handler_IO (int status)
{	
	
	int retval,oldwpos;
	retval=read(xbee_fd,&uartData,1);
	
	//printf("%X\n",uartData);
	//info(LOG_APPL,"retval = %d",retval);
	//info(LOG_APPL,"uartData = %X",uartData);
	if(retval > 0){	
		if(inputBufferIndex == 0)
			{	
				if(uartData == 0x7E)
					inputBuffer[inputBufferIndex++]=uartData;
			}			
		else if (inputBufferIndex > 0)
			{
				inputBuffer[inputBufferIndex++]=uartData;
			
				if(inputBufferIndex == 3) //length of frame
					{
						framelength = uartData + 4;
					}
				else if(inputBufferIndex == framelength)
					{	checksum=checksum+uartData;
						//if(inputBuffer[3] == 0x8B)
							//info(LOG_APPL,"\nTX STATUS : frame complete!");
						//else 
						//if(inputBuffer[3] == 0x90)
						//	info(LOG_APPL,"\nRX indicator : frame complete!");
						//else
							//info(LOG_APPL,"\nUnknown frame type complete!");
						
						switch(inputBuffer[3])
						{
							
							case 0x90:	
									if(checksum == 0xFF)
											{
												//printf("Packet recieved\n");
												checksum =0;
												switch(inputBuffer[15])
												{
													case DP_TYPE:
														flags[DR_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case CB_TYPE:
														flags[CB_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STR_TYPE:
														flags[STR_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STR_MSB_TYPE:
														flags[STR_MSB_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STR_CTL_TYPE:
														flags[STR_CTL_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STATUS_REQ_TYPE:
														flags[STATUS_REQ_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STATUS_RESP_TYPE:
														flags[STATUS_RESP_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;
													//make it for record request as well
													default:;
												}
													
												//return (framelength);
											}
										else
											{	numPacketsLost++;
												info(LOG_APPL,"Incorrect Checksum packet %X framelength %d\n",checksum,framelength);
												//switch on basis of message type and set wpos = wpos - (SIZE OF MESSAGE)
												checksum =0;
												switch(inputBuffer[15])
													{
														case DP_TYPE: 
																wpos=wpos-DP_PLD_LEN;
														case STR_MSB_TYPE:
																
																strwpos=strwpos-MAX_PLD_LEN;
														case STR_TYPE:
																strwpos=strwpos-MAX_PLD_LEN;
																printf("INTR:strwpos:%d,strrpos:%d\n",strwpos,strrpos);
																//strwpos=strwpos-MAX_PLD_LEN;
														default:;
														//TODO:case STATUS_RESP_TYPE: //to be filled later
													}
												
											}
							default:checksum =0;
						}
						framelength=0;
						inputBufferIndex=0;
					}
			
				else if(inputBufferIndex > 3) //type of frame
					{checksum=checksum+uartData;
						switch(inputBuffer[3])  
							{
							case 0x8B :
								//if(inputBufferIndex == 4)
								//info(LOG_APPL,"Transmit status recieved");
								
							break;
							
							case 0x90 :
								if(inputBufferIndex >=10 && inputBufferIndex <=12) //set src address
									{
										src_add[inputBufferIndex-10] = uartData;
										//info(LOG_APPL,"SAB %X",uartData);
									}
								else if(inputBufferIndex == 16)
									{
											switch(inputBuffer[15])
												{
													case DP_TYPE:
														if(framelength!=DP_FRAMELENGTH)
															{
																info(LOG_APPL,"DP:Framelength does not match expected packet size!");
																inputBufferIndex = 0;
																framelength = 0;
																checksum =0;
															}
														if(wpos%DP_PLD_LEN!=0)
															{
																oldwpos=wpos;
																wpos=wpos-(wpos%DP_PLD_LEN);
																info(LOG_APPL,"POC! newwpos=%d,oldwpos=%d",wpos,oldwpos);
															}
														break;
													case STR_TYPE:
														if(framelength!=STR_FRAMELENGTH)
															{
																info(LOG_APPL,"STR:Framelength does not match expected packet size!");
																inputBufferIndex = 0;
																framelength = 0;
																checksum =0;
															}
														if(strwpos%MAX_PLD_LEN!=0)
															{
																strwpos=strwpos-(strwpos%MAX_PLD_LEN);
															}
														break;
													default:;//TODO : fill other types later

												}
									}
								else if(inputBufferIndex >= 17 && inputBufferIndex <= framelength -1) //actual data recieved handle data here
									{
										//info(LOG_APPL,"%X ",uartData);
										/*if(inputBufferIndex == 17)
										printf("%d %X %X\n",inputBufferIndex,uartData,inputBuffer[inputBufferIndex-1]);
										
										if(inputBufferIndex == 18)
										printf("%d %X %X\n",inputBufferIndex,uartData,inputBuffer[inputBufferIndex-1]);
										//info(LOG_APPL,"%X",inputBuffer[15]);*/
										switch(inputBuffer[15])
											{
												case DP_TYPE: //data packet
													//info(LOG_APPL,"%d wpos %d",uartData,wpos);	
													//printf("%X\n",uartData);
													dataBuffer[wpos++]=uartData; //caching all the data payload
													if(inputBufferIndex == framelength-1 )
														{
															//flags[DR_FLAG]=UP;
															//sem_post(&interrupt_sem);
														}
													if(wpos == DATA_BUF_LEN)
														{
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	wpos = 0;
															
														}
													break;
														
												case CB_TYPE: //change of base station
													tempDestBuf[inputBufferIndex-17]=uartData;												
													if(inputBufferIndex == framelength-1 )
														{	dest_address[5]=tempDestBuf[0];
															dest_address[6]=tempDestBuf[1];
															dest_address[7]=tempDestBuf[2];
															//flags[CB_FLAG]=UP;
															//sem_post(&interrupt_sem);
														}
													break;
													
												case STR_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															//flags[STR_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												case STR_MSB_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															
															//flags[STR_MSB_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												case STR_CTL_TYPE:
													control = uartData;
													if(inputBufferIndex == framelength-1 )
														{
															stream_control = control;
															//flags[STR_CTL_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
												break;
												case STATUS_REQ_TYPE:
													if(inputBufferIndex == framelength-1)
														{
															//flags[STATUS_REQ_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
												break;

												case STATUS_RESP_TYPE:
									
													statusDataBuffer[statwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															
															//flags[STATUS_RESP_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STATUS_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	statwpos = 0;
															
														}
												break;
												
												case REC_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															flags[REC_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												case REC_MSB_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															
															flags[REC_MSB_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												//default:
															//info(LOG_APPL,"Unknown message");
											}
											
										
									}
							break;
							
							default :
																	
									
									framelength = 0;
									inputBufferIndex = 0;
									checksum =0;		
							}
					}
			}
		}else if(retval <=0){
			//info(LOG_APPL,"Read System CALL Failed");
		}
	
}




void dataRead(EventInfo *e)
{		char dstr[32];
		char timestring[60];
		//int i;
		struct tm lt;
		time_t t;
		//int i;
		
		e->id=0;
		memcpy(&e->id,dataBuffer+rpos,2);
        	memcpy(&e->sps,dataBuffer+rpos+2,2);
		memcpy(&e->x,dataBuffer+rpos+4,8);
		memcpy(&e->y,dataBuffer+rpos+12,8);
		memcpy(&e->z,dataBuffer+rpos+20,8);
		memcpy(&e->ptime,dataBuffer+rpos+28,8);
		memcpy(&e->stime,dataBuffer+rpos+36,8);
		memcpy(&e->amplitude,dataBuffer+rpos+44,8);   
		
		/*
		void * ptr = (void*) e;
		memcpy(ptr,dataBuffer+rpos,2);
        	memcpy(ptr+2,dataBuffer+rpos+2,2);
		memcpy(ptr+4,dataBuffer+rpos+4,8);
		memcpy(ptr+12,dataBuffer+rpos+12,8);
		memcpy(ptr+20,dataBuffer+rpos+20,8);
		memcpy(ptr+28,dataBuffer+rpos+28,8);*/ 
		printf("\n------------------------->%u, %u, %f, %f, %f, %f, %f, %f<-------------------------\n", e->id, e->sps, e->x, e->y, e->z, e->ptime, e->stime, e->amplitude);
		info(LOG_APPL,"rpos:%d,wpos:%d,numPacketsLost:%d",rpos,wpos,numPacketsLost);
		info(LOG_APPL,"\n%u, %u, %f, %f, %f, %f, %f, %f", e->id, e->sps, e->x, e->y, e->z, e->ptime, e->stime, e->amplitude);
		t = e->ptime;
		(void) localtime_r(&t, &lt);
		const char *format = "%c"; //"%a %b %d %Y";
		if (strftime(dstr, sizeof(dstr), format, &lt) == 0) {
		        (void) fprintf(stderr,  "strftime(3): cannot format supplied "
		                                "date/time into buffer of size %u "
		                                "using: '%s'\n",
		                                sizeof(dstr), format);
		        //return 1;
		}

		printf("%s\n", dstr);/*
		sprintf(dstr,"%f",e->time);
		strcpy(timestring,"date -d @");
		strcat(timestring,dstr);
		system(timestring);*/

}

unsigned int send_xbee_msg(int fd, unsigned char* buffer,unsigned int length, unsigned char*dest_address, unsigned char msg_type)
{
	unsigned char MSB,LSB;
	unsigned char message[MAX_PKT_LEN];//can go on till 65536..
	unsigned int i;
	unsigned int len;
	unsigned char checksum;
	len=length+15;
	if((14+length)>MAX_PKT_LEN)
		{
			return 0;//length too big
		}
	else if(length==0)
		{
			return 2; //length 0;
		}
	else if(!dest_address)
		{
			return 3;
		}
	else
		{
			//message=(unsigned char*)malloc(sizeof(unsigned char)*(length+18));
			LSB=len & 0xFF;
			MSB=(len & 0xFF00)>>8;
			message[0]=0x7E;
			message[1]=MSB;
			message[2]=LSB;
			message[3]=0x10;
			message[4]=0x01;
			message[5]=dest_address[0];
			message[6]=dest_address[1];
			message[7]=dest_address[2];
			message[8]=dest_address[3];
			message[9]=dest_address[4];
			message[10]=dest_address[5];
			message[11]=dest_address[6];
			message[12]=dest_address[7];
			message[13]=0xFF;
			message[14]=0xFE;
			message[15]=0x00;
			message[16]=0x00;
			message[17]=msg_type;
			//memcpy(message+17, buffer, length);
			for(i=0;i<length;i++)
				{

					message[(18+i)]=buffer[i];

				}

			checksum = 0;
			for (i = 3; i < (length+18); i++)
				{
					checksum += message[i];
				}
			checksum = (0xff - (checksum & 0xff));
			message[(length+18)]=checksum;
			/*
			for(i=0;i<(length+19);i++){
			printf("%X ",message[i]);
			}
			printf("\n");*/
			int s= -1;
			int tries=0;
			int ret = 0;
			int occupied = 0;
			ret=ioctl(fd,TIOCOUTQ,&occupied);

			while((4096-occupied)<((int)length+19)){
							tries++;
							//s=write (fd, message, length+19);
							ret=ioctl(fd,TIOCOUTQ,&occupied);
							info(LOG_APPL,"%d",occupied);			
						}
			s=write(fd,message,length+19);
		      	info(LOG_APPL,"s:%d tries:%d",s,tries);		
			return s;
		}


}

int set_interface_attribs (int fd, int speed, int parity)
{
	tcflush(fd,TCIOFLUSH);
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                info(LOG_APPL,"error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);
  //EventInfo event;
       //event.id = 5;
       //event.x = 1.0;
       //event.y = 2.0;
       //event.z = 3.0;
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                info(LOG_APPL,"error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                info(LOG_APPL,"error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                info(LOG_APPL,"error %d setting term attributes", errno);
}

void write_station_file(FILE *sta_file, int *status, EventInfo *arrvTimeInfo) {
    
    static StationInfo stn_info;
    static double cx, cy, cz;
    static double alpha = ALPHA;

    /*
    memcpy(&stn_info.id, sbuf, 4);
    memcpy(&stn_info.x, sbuf+8, 8);
    memcpy(&stn_info.y, sbuf+16, 8);
    memcpy(&stn_info.z, sbuf+24, 8); 
   
    
    memcpy(&stn_info.id, sbuf, 2);
    memcpy(&stn_info.x, sbuf+4, 8);
    memcpy(&stn_info.y, sbuf+12, 8);
    memcpy(&stn_info.z, sbuf+20, 8);
 */
    stn_info.id = arrvTimeInfo->id;
    stn_info.x = arrvTimeInfo->x;
    stn_info.y = arrvTimeInfo->y;
    stn_info.z = arrvTimeInfo->z;

    cx = stn_info.x;
    cy = stn_info.y;
    cz = stn_info.z;
    
    if(status[stn_info.id] == -1) {
        info(LOG_APPL,"station not in file");
        fseek(sta_file, 0L, SEEK_END);
        status[stn_info.id] = ftell(sta_file);
        
        fwrite(&stn_info, sizeof(StationInfo), 1, sta_file);
        fflush(sta_file);

    } else {
        info(LOG_APPL,"station in file");

        fseek(sta_file, status[stn_info.id], SEEK_SET);
        if (sizeof(StationInfo) != (fread(&stn_info, sizeof(StationInfo), 1, sta_file)))
	{
		info(LOG_APPL,"error in reading station file");
	}

        info(LOG_APPL,"%d, %f, %f, %f", stn_info.id, stn_info.x, stn_info.y, stn_info.z);

        stn_info.x = alpha*cx + (1-alpha)*stn_info.x;
        stn_info.y = alpha*cy + (1-alpha)*stn_info.y;
        stn_info.z = alpha*cz + (1-alpha)*stn_info.z;

        fseek(sta_file, status[stn_info.id], SEEK_SET);
        fwrite(&stn_info, sizeof(StationInfo), 1, sta_file);
        fflush(sta_file);

    }
}

void *tcp_socket_server(void *arg)
{
    int opt = TRUE;
    unsigned char data[256];
    unsigned char addr[8];
    struct sockaddr_in active_conn[NO_OF_CLIENTS+1];
    int len,tries,ret,n;
    int master_socket , addrlen , new_socket , max_clients = NO_OF_CLIENTS , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
    signal(SIGPIPE, SIG_IGN);
    //signal(SIGIO,SIG_IGN);
    char buffer[1025];  //data buffer of 1K
     
    //set of socket descriptors
    fd_set readfds;
    pthread_mutex_init(&clients, NULL);
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
     
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    fcntl(master_socket, F_SETFL, O_NONBLOCK);
    //set master socket to allow multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
 
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( 20000 );
     
    //bind the socket to port 20000
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", 20000);
	
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, NO_OF_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
     
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    
    while(TRUE) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
 
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
		
        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) 
        {
            //socket descriptor
			sd = client_socket[i];
            
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
            
			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
   	
        if ((activity < 0) && (errno!=EINTR)) 
        {
	    perror("select");
            printf("select error ERRNO: %d",errno);
            continue;
        }
        //If something happened on the master socket , then its an incoming connection or a disconnection
        if (FD_ISSET(master_socket, &readfds)) 
        	{			

				//printf("ISSET successful\n");
				new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
		
				if (new_socket<0)
            				{
               					 //perror("accept");
                				continue;
            				}

				//printf("Accept successful\n");
	    			for(n=0;n<numclients;n++)
					{
						if(address.sin_addr.s_addr==active_conn[n].sin_addr.s_addr)
						{
							getpeername(client_socket[n] , (struct sockaddr*)&address , (socklen_t*)&addrlen);
							pthread_mutex_lock(&clients);			
							//numclients=numclients-1;
							pthread_mutex_unlock(&clients);
						    	printf("\nHost disconnected , ip %s , port %d numclients %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port),numclients);
							close(client_socket[n]);
							client_socket[n]=0;
							for(int j=n+1;j<numclients;j++)
								{
									active_conn[j-1]=active_conn[j];
									//client_sockfd[j-1]=client_sockfd[j];
								}
							break;
						}
					}
				
				if(n!=numclients)
					{	
						pthread_mutex_lock(&clients);				
						numclients--;	
						pthread_mutex_unlock(&clients);
						printf("No of clients now %d\n",numclients);
						continue;	
					}



				active_conn[numclients]=address;
				pthread_mutex_lock(&clients);	
			    	numclients++;
				pthread_mutex_unlock(&clients);	
			    	//inform user of socket number - used in send and receive commands
			    	printf("\nNew connection , socket fd is %d , ip is : %s , port : %d number of clients %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port),numclients);
		       		
           
			    	//add new socket to array of sockets
				for (i = 0; i < max_clients; i++) 
				    {
					//if position is empty
					if( client_socket[i] == 0 )
						{
						    client_socket[i] = new_socket;
						    fcntl(new_socket,F_SETFL,O_NONBLOCK);
						    printf("Adding to list of sockets as %d\n" , i);
						    break;
						}
				    }
        	}

        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) 
        	{

		    sd = client_socket[i];
		
		    if (FD_ISSET( sd , &readfds)) 
		    	{
				//Check if it was for closing , and also read the incoming message
				//printf("examining %d\n",i);
				valread = read( sd , buffer, 1024);
				//printf("valread %d\n",valread);
				if (valread == 0)
					{
					    	//Somebody disconnected , get his details and print
						//printf("it is a disconnection\n");
					    	getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
						pthread_mutex_lock(&clients);			
						numclients=numclients-1;
						pthread_mutex_unlock(&clients);
					    	printf("\nHost disconnected , ip %s , port %d numclients %d\n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port),numclients);
					     
					    	//Close the socket and mark as 0 in list for reuse
					    	close( sd );
					    	client_socket[i] = 0;
					}
				else if(valread <0)
					{	
						//close(sd);
						//numclients=numclients-1;
						pthread_mutex_lock(&clients);			
						numclients=numclients-1;
						pthread_mutex_unlock(&clients);
						puts(strerror(errno));
						printf("valread<0\n");
					} 
				//Echo back the message that came in
				else
					{	

						    //set the string terminating NULL byte on the end of the data read
						    //printf("it is something else\n");
						    buffer[valread] = '\0';
						    //printf("buffer is %s\n",buffer);
							switch(buffer[0]){
								case CB_TYPE:
				
									    addr[0]=0x00;
									    addr[1]=0x13;
									    addr[2]=0xA2;
									    addr[3]=0x00;
									    addr[4]=0x40;
									    memcpy((addr+5),(buffer+1),3);
									    memcpy(data,(buffer+4),3);
									    //len=buffer[4];
									    //data[0]=buffer[5];
									    len=3;
									    printf("Type CB_TYPE to xbee @%X %X %X %X change to %X %X %X\n",buffer[0],addr[5],addr[6],addr[7],data[0],data[1],data[2]);
						
										tries =1;
										ret=-1;	
										ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
										while(ret!=22){
											ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
											tries++;
										}

									    printf("sent CB_TYPE to xbee @ %X %X %X -- %d bytes after %d tries \n",addr[5],addr[6],addr[7],ret,tries);
						   		break;	
			
								case STR_CTL_TYPE:
				
									    addr[0]=0x00;
									    addr[1]=0x13;
									    addr[2]=0xA2;
									    addr[3]=0x00;
									    addr[4]=0x40;
									    memcpy((addr+5),(buffer+1),3);
									    len=buffer[4];
									    data[0]=buffer[5];
									    printf("%X %X %X %X %X %X\n",buffer[0],addr[5],addr[6],addr[7],len,data[0]);
						
										tries =1;
										ret=-1;	
										ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
										while(ret!=20){
											ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
											tries++;
										}

									    printf("sent STR_CTL_TYPE to xbee @ %X %X %X -- %d bytes after %d tries \n",addr[5],addr[6],addr[7],ret,tries);
						   		break;
								case STATUS_REQ_TYPE:
									    addr[0]=0x00;
									    addr[1]=0x13;
									    addr[2]=0xA2;
									    addr[3]=0x00;
									    addr[4]=0x40;
									    memcpy((addr+5),(buffer+1),3);
									    len=1;
									    data[0]=0;
									    printf("%X %X %X %X\n",buffer[0],addr[5],addr[6],addr[7]);
						
										tries=1;
										ret=-1;	
										ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
										while(ret!=20){
											ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
											tries++;
										}
									    printf("sent STATUS_REQ_TYPE to xbee @ %X %X %X -- %d bytes after %d tries \n",addr[5],addr[6],addr[7],ret,tries);
							      break;	

							      case RECORD_REQ_TYPE:
									    addr[0]=0x00;
									    addr[1]=0x13;
									    addr[2]=0xA2;
									    addr[3]=0x00;
									    addr[4]=0x40;
									    memcpy((addr+5),(buffer+1),3);
									    memcpy(data,(buffer+4),18);	
									    len=18;
									    
									    printf("RECORD REQUESTED from -------------------------> %X %X %X\n",addr[5],addr[6],addr[7]);
						
										tries=1;
										ret=-1;	
										ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
										while(ret!=37){
											ret=send_xbee_msg(xbee_fd,data,len,addr,buffer[0]);
											tries++;
										}
										printf("sent RECORD_REQ_TYPE to xbee @ %X %X %X -- %d bytes after %d tries \n",addr[5],addr[6],addr[7],ret,tries);
							}
						   // send(sd , buffer , strlen(buffer) , 0 );
					}
		    		}
        	}
    }

}


/*
XbeeListener is designed with the logic of intrupt handling as it runs on BaseStation.
Typically Basestation has to service many message at the same time.
*/

void *XBeeListener (void *param) {
	EventInfo event;
	int returnSem;
	struct eventMsgGUI{
		unsigned char src_add[3];		
		unsigned int length;
		unsigned int type;		
		EventInfo event;
			
	} msg;
	
	struct streamMsgGUI{
		unsigned char src_addr[3];
		unsigned int length;
	};
		
	unsigned char arrtimeBuffer[58];
	unsigned char strBuffer[MAX_PLD_LEN+4];
	unsigned char statusRespBuffer[46];
	int val;

	while (1)
	   {


		//info(LOG_APPL,"Waiting for input to show up");
		if(anyFlagUp()==0)
			{
				returnSem=sem_wait(&interrupt_sem);
				while(returnSem<0)
					{
						returnSem=sem_wait(&interrupt_sem);
					}
			}
		//printf("Broke out of sem_wait flags DR %d returnSem %d\n",flags[DR_FLAG],returnSem);
		if( flags[DR_FLAG] == UP)
		{			
					arrtimeBuffer[0]=DP_TYPE;
					memcpy(arrtimeBuffer+1,src_add,3);			
					msg.type=DP_TYPE;	
					dataRead(&event);			
					rpos = rpos + DP_PLD_LEN;
					
					if(rpos == DATA_BUF_LEN)
						{
							rpos = 0;
						}
					if(wpos-rpos==0 || wpos-rpos<DP_PLD_LEN)
						{
							flags[DR_FLAG] = DN;
						}
					// TODO:uncomment until clear how to handle station information
					// FATAL:main cause of segmentation fault when corrupt packets of arrival time recieved
					write_station_file(station_file, stn_in_file, &event);
							
					msg.event=event;
					msg.length=DP_PLD_LEN;
					memcpy(arrtimeBuffer+4,&msg.length,2);	
					memcpy(arrtimeBuffer+6,&event.id,2);
        				memcpy(arrtimeBuffer+8,&event.sps,2);
					memcpy(arrtimeBuffer+10,&event.x,8);
					memcpy(arrtimeBuffer+18,&event.y,8);
					memcpy(arrtimeBuffer+26,&event.z,8);
					memcpy(arrtimeBuffer+34,&event.ptime,8);
					memcpy(arrtimeBuffer+42,&event.stime,8);
					memcpy(arrtimeBuffer+50,&event.amplitude,8);  
		
					val=sendToClient(arrtimeBuffer,58);
					printf("********Arrtime information Broadcasted to %d clients*********\n",val);
					fwrite(&event, sizeof(EventInfo), 1, arrival_time_file);
                			fflush(arrival_time_file);			

		}
		else if(flags[CB_FLAG] == UP)
		{
			info(LOG_APPL,"Base Station Address changed %X %X %X",dest_address[5],dest_address[6],dest_address[7]);
			flags[CB_FLAG] = DN;
			
		}
		else if(flags[STR_CTL_FLAG] == UP)
		{
			if(stream_control == 1)			
				info(LOG_APPL,"Streaming is on");
			else 
				info(LOG_APPL,"Streaming is off");


			flags[STR_CTL_FLAG] = DN;

		}
		else if(flags[STR_FLAG] == UP || flags[STR_MSB_FLAG] == UP)
		{			//int ii;
					//printf("strrpos %d\n",strrpos);
					/*for(ii=strrpos;ii<(strrpos+MAX_PLD_LEN);ii++)
						{	
							printf("%X",streamDataBuffer[ii]);
						}
					*/
					
					/*for(ii=strrpos;ii<(10+strrpos);ii++)
						{	
							//printf("%X ",streamDataBuffer[ii]);
						}*/
					printf("strrpos %d,strwpos %d\n",strrpos,strwpos);
					double utc_time;
					utc_time=0;
					memcpy(&utc_time,streamDataBuffer+strrpos,8);
					printf("Segment \n %f\n",utc_time);
					unsigned int sam_rate;
					sam_rate = 0;
					memcpy(&sam_rate,streamDataBuffer+strrpos+8,2);
					//printf("\n %d\n",sam_rate);
					unsigned int str_len;
					memcpy(&str_len,streamDataBuffer+strrpos+10,2);
					//printf("\n %d\n",(int)str_len);
					//printf("\n %X %X\n",streamDataBuffer[strrpos+10],streamDataBuffer[strrpos+11]);
					//printf("\n");
					//printf("Recieved streaming data\n");

					


					strBuffer[0] = (flags[STR_FLAG] == UP?STR_TYPE:STR_MSB_TYPE);
			
					//printf("type %d\n",strBuffer[0]);

					memcpy(strBuffer+1,src_add,3);
					memcpy(strBuffer+4,streamDataBuffer+strrpos,MAX_PLD_LEN);
					strrpos = strrpos +MAX_PLD_LEN;
					if(strrpos == STR_BUF_LEN)
						{
							strrpos = 0;
						}
					if(strwpos==strrpos || strwpos-strrpos<MAX_PLD_LEN )
					{
						if(flags[STR_FLAG] == UP) 
							flags[STR_FLAG] = DN;
						else
							flags[STR_MSB_FLAG]=DN;
					}					
					//printf("\nPacket finished %d %d\n",strrpos,ii);					
					
					/*if(err_connect >= 0){	
						val = sendto(sockfd,strBuffer,(MAX_PLD_LEN+4),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
						info(LOG_APPL,"STR_TYPE:sendTo returned %d",val);
					}*/
					val=sendToClient(strBuffer,(MAX_PLD_LEN+4));
					printf("Streaming information Broadcasted to %d clients\n",val);
					//printf("val : %d\n",val);					
					//printf("Sent streaming data\n");
		}
		
		else if(flags[STATUS_REQ_FLAG] == UP)
		{


		}

		else if(flags[STATUS_RESP_FLAG] == UP)
		{
			double input_v,battery_v,lat,lon,alt;
			unsigned char num_satellites,stream_ctl;
			input_v=0;battery_v=0;lat=0;lon=0;alt=0;
			memcpy(&input_v,statusDataBuffer+statrpos,8);
			memcpy(&battery_v,statusDataBuffer+statrpos+8,8);
			memcpy(&lat,statusDataBuffer+statrpos+16,8);
			memcpy(&lon,statusDataBuffer+statrpos+24,8);
			memcpy(&alt,statusDataBuffer+statrpos+32,8);
			memcpy(&num_satellites,statusDataBuffer+statrpos+40,1);
			memcpy(&stream_ctl,statusDataBuffer+statrpos+41,1);
			statrpos = statrpos + STAT_RESP_PLD_LEN;
			if(statrpos == STATUS_BUF_LEN)
				{
					statrpos = 0;
				}
			if(statwpos==statrpos || statwpos-statrpos<STAT_RESP_PLD_LEN)//statwpos==statrpos || statwpos-statrpos<42
				{
					flags[STATUS_RESP_FLAG] = DN;
				}
			
			statusRespBuffer[0]=STATUS_RESP_TYPE;
			memcpy(statusRespBuffer+1,src_add,3);
			memcpy(statusRespBuffer+4,&input_v,8);
			memcpy(statusRespBuffer+12,&battery_v,8);
			memcpy(statusRespBuffer+20,&lat,8);
			memcpy(statusRespBuffer+28,&lon,8);
			memcpy(statusRespBuffer+36,&alt,8);
			memcpy(statusRespBuffer+44,&num_satellites,1);
			memcpy(statusRespBuffer+45,&stream_ctl,1);
			val=sendToClient(statusRespBuffer,46);
			printf("\n#######################stream status from %X %X %X %d#######################\n",src_add[0],src_add[1],src_add[2],stream_ctl);
			printf("Status information Broadcasted to %d clients\n",val);
				

		}		
		/*
		else if(flags[REC_FLAG] == UP || flags[REC_MSB_FLAG] == UP)
		{			
					int ii;
					
					double utc_time;
					utc_time=0;
					memcpy(&utc_time,streamDataBuffer+strrpos,8);
					printf("\n %f\n",utc_time);
					unsigned int sam_rate;
					sam_rate = 0;
					memcpy(&sam_rate,streamDataBuffer+strrpos+8,2);
					//printf("\n %d\n",sam_rate);
					unsigned int str_len;
					memcpy(&str_len,streamDataBuffer+strrpos+10,2);
					//printf("\n %d\n",(int)str_len);
					//printf("\n %X %X\n",streamDataBuffer[strrpos+10],streamDataBuffer[strrpos+11]);
					//printf("\n");
					//printf("Recieved streaming data\n");
					strBuffer[0] = (flags[REC_FLAG] == UP?REC_TYPE:REC_MSB_TYPE);
			
					printf("type %d\n",strBuffer[0]);

					memcpy(strBuffer+1,src_add,3);
					memcpy(strBuffer+4,streamDataBuffer+strrpos,MAX_PLD_LEN);
					strrpos = strrpos +MAX_PLD_LEN;
					if(flags[REC_FLAG] == UP) 
						flags[REC_FLAG] = DN;
					else
						flags[REC_MSB_FLAG]=DN;
					//printf("\nPacket finished %d %d\n",strrpos,ii);					
					if(strrpos == STR_BUF_LEN)
						{
							strrpos = 0;
						}
					
					val=sendToClient(strBuffer,(MAX_PLD_LEN+4));
					printf("RECORD information Broadcasted to %d clients\n",val);
					//printf("val : %d\n",val);					
					//printf("Sent streaming data\n");
		}
		*/
	


	}


	      //sendto(sockfd,sendline,strlen(sendline),0,(struct sockaddr *)&servaddr,sizeof(servaddr));
	      //n=recvfrom(sockfd,recvline,10000,0,NULL,NULL);
	      //recvline[n]=0;
	      //fputs(recvline,stdout);
	   return NULL;
}


int anyFlagUp()
{
	int ups = 0;
	int flagNo=0;
	for(flagNo=0;flagNo<NO_OF_FLAGS;flagNo++)
	{
		ups=ups+(flags[flagNo]>0?1:0);
	}
	return ups;
}


int sendToClient(unsigned char * buffer,int size)
{	int sentTo,n,num;
	sentTo=0;
	pthread_mutex_lock(&clients);
	num=numclients;
	pthread_mutex_unlock(&clients);	
	for(int i=0;i<num;i++)
		{	
			//printf("sending to client %d\n",i);
			n=send(client_socket[i],buffer,size,NULL);
			if(n==size)
				{
					//printf("wrote to socket %d\n",client_socket[i]);
					sentTo++;
				}
			else
				{	
					puts(strerror(errno));
					//printf("could not write to socket%d\n",n);
				}
		}
	return sentTo;
}



/*

XBee BBB specific code startes here
XbeeReader is designed to run on each BBB. Initially we had intrupt based logic similar to XbeeListner.
But later we found that this was not efficient when it had to recieve message from GUI (change of base station etc)
Therefore we used a blocking read call to prevent interrupt
*/

void* xbeeReader(void* param)
{	
	
	
	int retval,oldwpos;
	while(1){
	retval=read(xbee_fd,&uartData,1);
	
	//printf("%X\n",uartData);
	//info(LOG_APPL,"retval = %d",retval);
	//info(LOG_APPL,"uartData = %X",uartData);
	if(retval > 0){	
		if(inputBufferIndex == 0)
			{	
				if(uartData == 0x7E)
					inputBuffer[inputBufferIndex++]=uartData;
			}			
		else if (inputBufferIndex > 0)
			{
				inputBuffer[inputBufferIndex++]=uartData;
			
				if(inputBufferIndex == 3) //length of frame
					{
						framelength = uartData + 4;
					}
				else if(inputBufferIndex == framelength)
					{	
						checksum=checksum+uartData;											
						switch(inputBuffer[3])
						{
							
							case 0x90:	
									if(checksum == 0xFF)
											{
												//printf("Packet recieved\n");
												checksum =0;
												switch(inputBuffer[15])
												{
													case DP_TYPE:
														flags[DR_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case CB_TYPE:
														flags[CB_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STR_TYPE:
														flags[STR_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STR_MSB_TYPE:
														flags[STR_MSB_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STR_CTL_TYPE:
														flags[STR_CTL_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STATUS_REQ_TYPE:
														flags[STATUS_REQ_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;

													case STATUS_RESP_TYPE:
														flags[STATUS_RESP_FLAG]=UP;
														sem_post(&interrupt_sem);
														break;
													//make it for record request as well
													default:;
												}
													
												//return (framelength);
											}
										else
											{	numPacketsLost++;
												info(LOG_APPL,"Incorrect Checksum packet %X framelength %d\n",checksum,framelength);
												//switch on basis of message type and set wpos = wpos - (SIZE OF MESSAGE)
												checksum =0;
												switch(inputBuffer[15])
													{
														case DP_TYPE: 
																wpos=wpos-DP_PLD_LEN;
														case STR_MSB_TYPE:
																strwpos=strwpos-MAX_PLD_LEN;
														case STR_TYPE:
																strwpos=strwpos-MAX_PLD_LEN;
														default:;
														//TODO:case STATUS_RESP_TYPE: //to be filled later
													}
												
											}
							default:checksum =0;
						}
						framelength=0;
						inputBufferIndex=0;
					}
			
				else if(inputBufferIndex > 3) //type of frame
					{checksum=checksum+uartData;
						switch(inputBuffer[3])  
							{
							case 0x8B :
								//if(inputBufferIndex == 4)
								//info(LOG_APPL,"Transmit status recieved");
								
							break;
							
							case 0x90 :
								if(inputBufferIndex >=10 && inputBufferIndex <=12) //set src address
									{
										src_add[inputBufferIndex-10] = uartData;
										//info(LOG_APPL,"SAB %X",uartData);
									}
								else if(inputBufferIndex == 16)
									{
											switch(inputBuffer[15])
												{
													case DP_TYPE:
														if(framelength!=DP_FRAMELENGTH)
															{
																info(LOG_APPL,"Framelength does not match expected packet size!");
																inputBufferIndex = 0;
																framelength = 0;
																checksum =0;
															}
														if(wpos%DP_PLD_LEN!=0)
															{
																oldwpos=wpos;
																wpos=wpos-(wpos%DP_PLD_LEN);
																info(LOG_APPL,"POC! newwpos=%d,oldwpos=%d",wpos,oldwpos);
															}
														break;
													case STR_TYPE:
														if(framelength!=STR_FRAMELENGTH)
															{
																inputBufferIndex = 0;
																framelength = 0;
																checksum =0;
															}
														if(strwpos%MAX_PLD_LEN!=0)
															{
																strwpos=strwpos-(strwpos%MAX_PLD_LEN);
															}
													default:;//TODO : fill other types later

												}
									}
								else if(inputBufferIndex >= 17 && inputBufferIndex <= framelength -1) //actual data recieved handle data here
									{
										//info(LOG_APPL,"%X ",uartData);
										/*if(inputBufferIndex == 17)
										printf("%d %X %X\n",inputBufferIndex,uartData,inputBuffer[inputBufferIndex-1]);
										
										if(inputBufferIndex == 18)
										printf("%d %X %X\n",inputBufferIndex,uartData,inputBuffer[inputBufferIndex-1]);
										//info(LOG_APPL,"%X",inputBuffer[15]);*/
										switch(inputBuffer[15])
											{
												case DP_TYPE: //data packet
													//info(LOG_APPL,"%d wpos %d",uartData,wpos);	
													//printf("%X\n",uartData);
													dataBuffer[wpos++]=uartData; //caching all the data payload
													if(inputBufferIndex == framelength-1 )
														{
															//flags[DR_FLAG]=UP;
															//sem_post(&interrupt_sem);
														}
													if(wpos == DATA_BUF_LEN)
														{
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	wpos = 0;
															
														}
													break;
														
												case CB_TYPE: //change of base station
													tempDestBuf[inputBufferIndex-17]=uartData;												
													if(inputBufferIndex == framelength-1 )
														{	dest_address[5]=tempDestBuf[0];
															dest_address[6]=tempDestBuf[1];
															dest_address[7]=tempDestBuf[2];
															//flags[CB_FLAG]=UP;
															//sem_post(&interrupt_sem);
														}
													break;
													
												case STR_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															//flags[STR_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												case STR_MSB_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															
															//flags[STR_MSB_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												case STR_CTL_TYPE:
													control = uartData;
													if(inputBufferIndex == framelength-1 )
														{
															stream_control = control;
															//flags[STR_CTL_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
												break;
												case STATUS_REQ_TYPE:
													if(inputBufferIndex == framelength-1)
														{
															//flags[STATUS_REQ_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
												break;

												case STATUS_RESP_TYPE:
									
													statusDataBuffer[statwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															
															//flags[STATUS_RESP_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STATUS_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	statwpos = 0;
															
														}
												break;
												
												case REC_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															flags[REC_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												case REC_MSB_TYPE: //stream type data
													
													streamDataBuffer[strwpos++]=uartData;
													
													if(inputBufferIndex == framelength-1 )
														{
															
															flags[REC_MSB_FLAG]=UP;
												
															//sem_post(&interrupt_sem);
														}
													if(strwpos == STR_BUF_LEN)
														{
														
															//info(LOG_APPL,"Buffer overflow!...starting again");
																	strwpos = 0;
															
														}
												break;
												//default:
															//info(LOG_APPL,"Unknown message");
											}
											
										
									}
							break;
							
							default :
																	
									
									framelength = 0;
									inputBufferIndex = 0;
									checksum =0;		
							}
					}
			}
		}else if(retval <= 0){
			info(LOG_APPL,"Read System CALL Failed");
		}
	}
}
