/*

sbuffer free()
fopen error

*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "config.h"
#include "sbuffer.h"
#include "sensor_db.h"
#include "datamgr.h"
#include "connmgr.h"
#define MAX 		100
#define FIFO_NAME 	"logFifo" 
#define LOG_NAME 	"gateway.log" 
void pthread_err_handler( int err_code, char *msg, char *file_name, char line_nr );
FILE *fp_fiforead;
FILE *fp_log;
pthread_mutex_t mutex_fifo = PTHREAD_MUTEX_INITIALIZER;
sbuffer_t * buffer; 
int PORT,reconnecting=0,fd_fifo;
void fifo_log(char* info)
{
 
int presult = pthread_mutex_lock( &mutex_fifo);
  pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
	char buf[MAX];
 memset(buf,0,MAX);
 
 snprintf( buf,MAX, "%s", info);
//printf("going to write    %s\n",buf);
         write(fd_fifo, buf, MAX);   
	  
	 presult = pthread_mutex_unlock( &mutex_fifo);
  pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );

  
           
}
void run_child (void)
{
	printf("Child process is started \n");

    int fd;
       char buf[MAX];
 
      // umask(0);
	 
	//int len;

    fd = open(FIFO_NAME,O_RDONLY);
//printf("****read\n");
fp_log=fopen(LOG_NAME,"w");
//printf("opened\n");
if(fd == -1){
        perror("open error");
        exit(EXIT_FAILURE);
    }
 

//fp_fiforead=fopen(FIFO_NAME,"r");
//fgets(buf, MAX, fp_fiforead);
//fgets(buf, MAX, fp_fiforead)!=NULL
 
while(read(fd,buf,MAX)>0){
  //printf("fprint\n");
  fprintf(fp_log,"%d %s",2,buf);
  //printf("******************\n****************\nchild get:  %s\n",buf);
    //fflush(fp_log);//更新缓冲区
}
 
	 
	//FILE_OPEN_ERROR(fp_fiforead);
	//FILE_OPEN_ERROR(fp_log);
 
//read(fd,buf,MAX);
 
 
    //printf("Message received: %s", buf);
     
	 
	 
	close(fd);
	//fclose(fp_log);
	printf("Child process end\n");
	
	int rm=remove(FIFO_NAME);
	
	if(rm == 0) 
	{
		printf("FIFO File deleted successfully\n");
	}
	else 
	{
		printf("Error: unable to delete the file\n");
	}
	exit(EXIT_SUCCESS);
}
void *conmanager()
{
printf("conmanager:  \n");
	 connmgr_listen(PORT,buffer);
	 connmgr_free();
       printf("conmanager:  exit\n"); 
	pthread_exit(NULL);
}


void *datamanger()
{
	  
 
    
    datamgr_parse_sensor_files("room_sensor.map",buffer);
	pthread_exit(NULL);
}

void *stormanager()
{   
int sleeptime=1;
printf("stormanager:  \n");
sensor_data_t data;
DBCONN * conn = init_connection(1);
while(reconnecting<3){
if(sbuffer_getosql(buffer,&data,0)!=0)
{//printf("no new data\n");
usleep(sleeptime);
sleeptime*=sleeptime<=MAXWATIME?2:1;
continue;
}
//printf("***********\nstor: sleep  %d\n",sleeptime);
 sleeptime/=2;

while(insert_sensor(conn, data.id, data.value, data.ts)!=0){
disconnect(conn);
conn =init_connection(0);
reconnecting++;
if(reconnecting==3){
disconnect(conn);
pthread_exit(NULL);

}

}
//sleep(1);
for(int i=0;sbuffer_getosql(buffer,&data,1)!=0;i+=1000)usleep(i);
//printf("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDD\n");
//fifo_log("deleted\n");
//close(fd_fifo);
}
//printf("out");
 
	disconnect(conn);
	pthread_exit(NULL);
}
//MAIN****************************************************
int main( int argc, char *argv[] )
{
printf("%s:   \n",argv[1]);
if (argc != 2)

	{
  printf("retype\n");
		exit(EXIT_SUCCESS);
	}
	else
	{
		PORT = atoi(argv[1]);
	}

if (fork() == 0  )
	{  
		  run_child();
	}
else{
  
     
    int open_mode = O_WRONLY;  
  
  
  
   
        
        if (mkfifo(FIFO_NAME, 0777) != 0)  
        {  
            fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);  
            exit(EXIT_FAILURE);  
        }  
     
  
    fd_fifo = open(FIFO_NAME, open_mode);  
     
 // write(pipe_fd, buffer, BUFFER_SIZE);  
   //sleep(20);
 
 
 
//printf("wrting\n");
//write(fd_fifo,"writed\n",5);
//fifo_log("xxxxxxxxxxx\n");
//if (mkfifo(FIFO_NAME, 0777) != 0)  
       // {  
     //       fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);  
     //       exit(EXIT_FAILURE);  
     //   }  
sbuffer_init(& buffer);
pthread_t tcp,sql,manager;
pthread_create(&tcp, NULL, conmanager, NULL);
pthread_create(&sql, NULL, stormanager, NULL);
pthread_create(&manager, NULL, datamanger, NULL);
 
pthread_join(tcp, NULL);
pthread_join(sql, NULL);
pthread_join(manager, NULL);
   close(fd_fifo);
sbuffer_free(&buffer);
datamgr_free();
 
 }

return 0;
}
