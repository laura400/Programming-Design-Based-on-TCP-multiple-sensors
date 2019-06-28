#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include "config.h"
#include <poll.h>
#include <unistd.h>
#include "sbuffer.h"
#include "connmgr.h"
#include "lib/tcpsock.h"
#include "lib/tcpsock.c"
#include  <limits.h>
#define PORT 5678
#define MAX_CONN 3
#define OPEN_MAX 100
#define	PROTOCOLFAMILY	AF_INET		// internet protocol suite
#define	TYPE		SOCK_STREAM	// streaming protool type
#define	PROTOCOL	IPPROTO_TCP 	// TCP protocol
#ifndef TIMEOUT
#error "TIMEOUT not defined"
#endif
 

tcpsock_t * server;
void connmgr_listen(int port_number,sbuffer_t *buffer)
{   int timeout=TIMEOUT,nofs=0,bytes, sockfd,new_sock;
    struct sockaddr_in client;
 struct  timeval sock_time[MAX_CONN];
struct  timeval now;
 char buf[100];
 memset(buf,0,100);
     
 
     
    sensor_data_t data;
    in_addr_t len=sizeof(client);
    short newconnections[OPEN_MAX];
    if (tcp_passive_open(&server,port_number)!=TCP_NO_ERROR) exit(EXIT_FAILURE); 
    struct pollfd sockets[OPEN_MAX+1];
    
    for(int i=1;i<=OPEN_MAX;i++){sockets[i].fd = -1;newconnections[i]=-1;}

    tcp_get_sd(server,&sockfd);	
sockets[0].fd=sockfd;
    sockets[0].events = POLLIN;
    while (1){
 
        int ret;
//printf("nofs : %d\n",nofs);
if(nofs==0)ret=poll(sockets,nofs+1,TIMEOUT*1000);
else ret =poll(sockets,nofs+1,TIMEOUT*2000);

        switch( ret){
            case -1:
                  
                perror("poll");//*****************************************************
                exit(EXIT_FAILURE);
                break;
            case 0:
              //  printf("Connection timeout\n");
  
return; 
            default:{
 
                if(sockets[0].revents&POLLERR){
                    perror("POLLERR");//*****************************************************
                    exit(EXIT_FAILURE);
                }
                if(sockets[0].revents&POLLIN){
                    new_sock=accept(sockfd,(struct sockaddr*)&client,&len);

                    if((new_sock)<0){

                            printf("accept failed!\n");
                            exit(EXIT_FAILURE);

                    }
nofs++;
                    for(int i=1;i<=OPEN_MAX;i++){
                        if(sockets[i].fd==-1){
                             newconnections[i]=-2;
                             
                 sockets[i].fd=new_sock;
                            sockets[i].events=POLLIN;
gettimeofday(&sock_time[i-1],NULL);
                            break;
                        }

                    }
                    if(nofs==OPEN_MAX+1){
                        printf("server busy...\n");
                        continue;
                    }
                   // printf("nofs:     %d\n",nofs);
                    //printf("connect a client [%d]\n",new_sock);

                }
for(int i=1;i<=OPEN_MAX;i++){
 //printf("sockets[%d].fd    %d\n",i,sockets[i].fd);
}
                for(int i=1;i<=OPEN_MAX;i++){
                    if(sockets[i].fd==-1)continue;
   gettimeofday(&now,NULL);
 if(now.tv_sec-sock_time[i-1].tv_sec>timeout){
 printf("sockets[%d]: timeout\n",sockets[i].fd);
 close(sockets[i].fd);
 sockets[i].fd = -1;
nofs--;
}
 
                    if(sockets[i].revents&POLLIN){
                        bytes = sizeof(data.id);
                        int result = recv(sockets[i].fd, &data.id, bytes, 0);
                         gettimeofday(&sock_time[i-1],NULL);
 // printf("id:      %hd    ",data.id);  
if(newconnections[i]==-2){ 
newconnections[i]=data.id;       
   
	 
 snprintf( buf,100, "A sensor node with <%hd> has opened a new connection\n",data.id);
  fifo_log(buf);
 

}
            // read temperature
                        bytes = sizeof(data.value);
                        result = recv(sockets[i].fd, &data.value, bytes, 0); 
 //printf("value:      %lf    ",data.value);                       // read timestamp
                        bytes = sizeof(data.ts);
                         result = recv(sockets[i].fd, &data.ts, bytes, 0); 
// printf("ts:       %ld\n",data.ts);
                        if (result>0) 
                        {
if(sbuffer_insert(buffer,&data)==0){

//printf("connmgr.c : inserted\n===============");

} 
 
                            
                        }
                        else{
                            if (result==0){
 
 snprintf( buf,100, "The sensor node with <%hd> has closed the connection\n",newconnections[i]);
  fifo_log(buf);

                                printf("Peer has closed connection\n");}
                            else
                                printf("Error occured on connection to peer\n");
                            close(sockets[i].fd);
                            sockets[i].fd = -1;
nofs--;
                        }



                        }

                        }


                    }

                }
        }
}


void connmgr_free()
{
    
	if (tcp_close( &server )!=TCP_NO_ERROR) exit(EXIT_FAILURE);
	printf("Test server is shutting down\n");
}
