#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <limits.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
  
#define FIFO_NAME "logFifo"  
#define BUFFER_SIZE PIPE_BUF  
#define TEN_MEG (1024 * 100)  
  
int main()  
{  
    int pipe_fd;  
    int res;  
    int open_mode = O_WRONLY;  
  
    int bytes = 0;  
    char buffer[BUFFER_SIZE + 1];  
  
   
        
        if (mkfifo(FIFO_NAME, 0777) != 0)  
        {  
            fprintf(stderr, "Could not create fifo %s\n", FIFO_NAME);  
            exit(EXIT_FAILURE);  
        }  
     
  sleep(5);
    printf("Process %d opening FIFO O_WRONLY\n", getpid());  
    pipe_fd = open(FIFO_NAME, open_mode);  
    printf("Process %d result %d\n", getpid(), pipe_fd);  
 // write(pipe_fd, buffer, BUFFER_SIZE);  
   //sleep(20);
 
         snprintf( buffer,100, "%s", "ssss");
            res = write(pipe_fd, buffer, 100);  
            
        close(pipe_fd);  
    
  
    printf("Process %d finish\n", getpid());  
    exit(EXIT_SUCCESS);  
}
