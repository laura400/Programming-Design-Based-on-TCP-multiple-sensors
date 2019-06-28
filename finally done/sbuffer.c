#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "sbuffer.h"
#include "config.h"
#include <pthread.h>

/*
 * All data that can be stored in the sbuffer should be encapsulated in a
 * structure, this structure can then also hold extra info needed for your implementation
 */
void pthread_err_handler( int err_code, char *msg, char *file_name, char line_nr )
{
	if ( 0 != err_code )
	{
		fprintf( stderr, "\n%s failed with error code %d in file %s at line %d\n", msg, err_code, file_name, line_nr );
	}
}
struct sbuffer_data {
    sensor_data_t data;
};

typedef struct sbuffer_node {
    struct sbuffer_node * next;
    sbuffer_data_t element;
 

} sbuffer_node_t;

struct sbuffer {
    int firstused;
    sbuffer_node_t * head;
    sbuffer_node_t * tail;
    sbuffer_node_t * pread;
 
};
pthread_mutex_t mutex  = PTHREAD_MUTEX_INITIALIZER;
 

int sbuffer_init(sbuffer_t ** buffer)
{
  *buffer = malloc(sizeof(sbuffer_t));
  if (*buffer == NULL) return SBUFFER_FAILURE;
  (*buffer)->head = NULL;
  (*buffer)->tail = NULL;
  (*buffer)->pread = NULL;



  return SBUFFER_SUCCESS;
}


int sbuffer_free(sbuffer_t ** buffer)
{
  sbuffer_node_t * dummy;
  if ((buffer==NULL) || (*buffer==NULL))
  {
    return SBUFFER_FAILURE;
  }
  while ( (*buffer)->head )
  {
    dummy = (*buffer)->head;
    (*buffer)->head = (*buffer)->head->next;
    free(dummy);
  }
  free(*buffer);
  *buffer = NULL;
  return SBUFFER_SUCCESS;
}




int sbuffer_getosql(sbuffer_t * buffer,sensor_data_t * data,int remove)
{  //printf("----------------------------------getosql\n");
  int presult,removelast=0;
  sbuffer_node_t * dummy;
  if (buffer == NULL) return SBUFFER_FAILURE; 
  if (buffer->head == NULL) return SBUFFER_NO_DATA;
 
  if(buffer->head==buffer->pread) return SBUFFER_NOTREAD;
 
 *data = buffer->head->element.data;
 if(remove){
if(buffer->head==buffer->tail){
removelast=1;

 
 presult = pthread_mutex_lock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  }
sbuffer_print(buffer);
  dummy = buffer->head;
  buffer->head = dummy->next;  
 
    free(dummy);
if(removelast){
 presult = pthread_mutex_unlock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );
}
 
}
 
 
  
   
 
  return SBUFFER_SUCCESS;
}
int sbuffer_read(sbuffer_t * buffer,sensor_data_t  * data)
{
 int presult;
 int readlast=0;
 sbuffer_node_t * dummy;

  if (buffer == NULL) return SBUFFER_FAILURE;
  if (buffer->pread == NULL) return SBUFFER_NO_DATA;
dummy=buffer->pread;
  if(dummy==NULL) return SBUFFER_NO_NEWDATA;
 
   
if(dummy==buffer->tail){
  readlast=1;
  presult = pthread_mutex_lock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
}
*data = dummy->element.data;
buffer->pread = dummy->next;
if(readlast){
presult = pthread_mutex_unlock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );
}  
  return SBUFFER_SUCCESS;
}


int sbuffer_insert(sbuffer_t * buffer, sensor_data_t * data)
{
 
  int presult = pthread_mutex_lock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
  sbuffer_node_t * dummy;
  if (buffer == NULL) return SBUFFER_FAILURE;
  dummy = malloc(sizeof(sbuffer_node_t));
  if (dummy == NULL)return SBUFFER_FAILURE;
  dummy->element.data = *data;
  dummy->next = NULL; 

 
if(buffer->head==NULL){
 buffer->tail=dummy;
buffer->pread=dummy;
buffer->head=dummy;
  
 presult = pthread_mutex_unlock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );
  
 //printf("head null\n");
  return SBUFFER_SUCCESS;
}
 buffer->tail->next=dummy;
buffer->tail=dummy;

if(buffer->pread==NULL){
buffer->pread=dummy;
}
  
presult = pthread_mutex_unlock( &mutex);
  pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ ); 
  return SBUFFER_SUCCESS;
}

int sbuffer_print(sbuffer_t * buffer){
   printf("***************\nsubffer list :\n================\n");
sensor_data_t da;
sbuffer_node_t * dummy=buffer->head;
if (buffer == NULL) return SBUFFER_FAILURE;
  dummy = buffer->head;
int i=0;
while(dummy!=NULL){

i++;
da=dummy->element.data;
printf(" **************%d:  %hd    %lf  %ld\n",i,da.id,da.value,da.ts);
dummy = dummy->next;
}
  return SBUFFER_SUCCESS;
}



