#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <limits.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
  #define LOG_NAME 	"gateway.log" 
#define FIFO_NAME "logFifo"  
#define BUFFER_SIZE PIPE_BUF  
  FILE *fp_log;
int main()  
{  
    int pipe_fd;  
    int res;  
  
    int open_mode = O_RDONLY;  
    char buffer[BUFFER_SIZE + 1];  
    int bytes = 0;  
    fp_log=fopen(LOG_NAME,"w");
    memset(buffer, '\0', sizeof(100));  
  
    printf("Process %d opeining FIFO O_RDONLY\n", getpid());  
    pipe_fd = open(FIFO_NAME, open_mode);  printf("opened\n");
    printf("Process %d result %d\n", getpid(), pipe_fd);  
  
    if (pipe_fd <0) exit(EXIT_FAILURE); 
     
        do{  
            res = read(pipe_fd, buffer, 100);  
fprintf(fp_log,"%d %s",2,buffer);
            bytes += res;  
        printf("%s\n",buffer);
        }while(res > 0);  
        close(pipe_fd);  
      
     
  
    printf("Process %d finished, %d bytes read\n", getpid(), bytes);  
    exit(EXIT_SUCCESS);  
}
