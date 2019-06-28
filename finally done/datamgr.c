#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib/dplist.h"
#include "lib/dplist.c"
#include <time.h>
#include "datamgr.h"
#include "sbuffer.h"
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif
 
 
#ifndef SET_MIN_TEMP
#error "SET_MIN_TEMP not defined"
#endif

#ifndef SET_MAX_TEMP
#error "SET_MAX_TEMP not defined"
#endif

#define SENSOR_INVALID_ERROR "Invalid sensor id"

void* element_copy(void * src_element);
void element_free(void ** element);
void dpl_print(dplist_t * list);
dplist_t* update(dplist_t* list,sensor_id_t id,sensor_value_t val,sensor_ts_t ts);
int element_compare(void * x, void * y);

typedef uint16_t room_id_t;
typedef struct {
    sensor_id_t sensor_id;
    room_id_t room_id;
    sensor_value_t avg;
    sensor_value_t values[RUN_AVG_LENGTH];
    sensor_ts_t ts;
    int measurements;
}element;
dplist_t* list;

int len=RUN_AVG_LENGTH;
void datamgr_parse_sensor_files(char * filename, sbuffer_t * buffer) { 
FILE *fp_sensor_map= fopen(filename, "r");
 
sensor_data_t  data;
int sleeptime=1;

    list = dpl_create(&element_copy, &element_free, &element_compare);
     
    element * telement=malloc(sizeof (element));
    while(fscanf(fp_sensor_map,"%"SCNd16 "%"SCNd16 ,&(telement->room_id),&(telement->sensor_id)) != EOF)    {
printf("%hd     %hd   \n",telement->room_id,telement->sensor_id);
        telement->measurements=0;
        list=dpl_insert_at_reference( list, telement, NULL, true);
    }
 fclose(fp_sensor_map);
    free(telement);
    int i=0;
    while(true) {
      //  printf("want to read");
  
//printf("data sleeptime: %d\n",sleeptime);
        if(sbuffer_read(buffer,&data)!=SBUFFER_SUCCESS){
usleep(sleeptime);
sleeptime*=sleeptime<=MAXWATIME?2:1;
continue;}
//printf("data sleeptime: %d\n",sleeptime);
sleeptime/=2;
//printf("rrrrrrrrrrrrrrrrrrrr\nread to manager: %hd    %lf  %ld\n",data.id,data.value,data.ts);
        list=update(list,data.id,data.value,data.ts);
i++;
        if(i==5){i=0;
//dpl_print(list);
}
    }


 

}
void* element_copy(void * telement)
{
     
     

    element * copy = malloc(sizeof (element));
 
     
    assert(copy != NULL);
    copy->sensor_id = ((element*)telement)->sensor_id;
    copy->room_id = ((element*)telement)->room_id;
    copy->avg = ((element*)telement)->avg;
    copy->ts = ((element*)telement)->ts;
    copy->measurements = ((element*)telement)->measurements;
    for(int i=0;i<=copy->measurements-1;i++){
    copy->values[i] = ((element*)telement)->values[i];
    }
    
    
    return (void *) copy;
}
void element_free(void ** element)
{
	free(*element);
}

int element_compare(void * x, void * y) {
    return ((((element*)x)->sensor_id < ((element*)y)->sensor_id) ? -1 : (((element*)x)->sensor_id == ((element*)y)->sensor_id) ? 0 : 1);
}
void dpl_print(dplist_t * list) {
    int i, length;
    length = dpl_size(list);
      printf("************\n");
    for (i = 0; i < length; i++) {
       
        element* telement = dpl_get_element_at_index(list, i);
         
        printf(" %hd     %hd      %d\n", telement->sensor_id,telement->room_id, telement->measurements);
for(int j=0;j<= telement->measurements-1;j++){
printf(" %f\n ",telement->values[j]);
}
printf(" avg:  %f\n ",telement-> avg);

 
     
    }
   
}
 

dplist_t* update(dplist_t* list,sensor_id_t id,sensor_value_t val,sensor_ts_t ts){
char buf[100];
 memset(buf,0,100);
    element * telement=malloc(sizeof (element));
    telement->sensor_id=id;
    dplist_node_t * node=dpl_get_reference_of_element(  list, telement );
    free(telement);
    if(node==NULL){
snprintf( buf,100,"Received sensor data with invalid sensor node ID <%hd>",id);
fifo_log(buf);
       // fprintf(stderr,"Sensor    %hd is not found ! \n", id);
        return list;
    }
    ((element*)(node->element))->ts=ts;
    ((element*)(node->element))->measurements+=((element*)(node->element))->measurements<5?1:0;
    for(int i=4;i>=1;i--){
       ((element*)(node->element))-> values[i]=((element*)(node->element))-> values[i-1];
    }
    ((element*)(node->element))-> values[0]=val;
    ((element*)(node->element))->avg=0;
    if(((element*)(node->element))->measurements==5){
        for(int i=0;i<=4;i++){
            ((element*)(node->element))->avg+=((element*)(node->element))->values[i];
        }
       ((element*)(node->element))->avg/=5;
       if(((element*)(node->element))->avg<SET_MIN_TEMP)snprintf( buf,100, "The sensor node with <%hd> reports it’s too cold (running avg temperature = <%lf>) \n", ((element*)(node->element))->sensor_id,((element*)(node->element))->avg);
       else if(((element*)(node->element))->avg>SET_MAX_TEMP)snprintf( buf,100, "The sensor node with <%hd> reports it’s too cold (running avg temperature = <%lf>) \n", ((element*)(node->element))->sensor_id,((element*)(node->element))->avg);
  fifo_log(buf);
    }
    return list;

}

uint16_t datamgr_get_room_id(sensor_id_t id){
 
    element * telement=malloc(sizeof (element));
    telement->sensor_id=id;
    dplist_node_t * node=dpl_get_reference_of_element(  list, telement );
    free(telement);
    if(node==NULL){
       // fprintf(stderr,"Sensor " "%" SCNd16 " is not found !"  "\n", id);
        return 0;
    }
    return ((element*)node->element)->room_id;
}
sensor_value_t datamgr_get_avg(sensor_id_t id){
 
    element * telement=malloc(sizeof (element));
    telement->sensor_id=id;
    dplist_node_t * node=dpl_get_reference_of_element( list, telement );
    free(telement);
    if(node==NULL){
       // fprintf(stderr,"Sensor " "%" SCNd16 " is not found !"  "\n", id);
        return 0;
    }
    return ((element *)node->element)->avg;
}
time_t datamgr_get_last_modified(sensor_id_t id){
    //ERROR_HANDLER(sensor_id==NULL,SENSOR_INVALID_ERROR);
    element * telement=malloc(sizeof (element));
    telement->sensor_id=id;
    dplist_node_t * node=dpl_get_reference_of_element(   list, telement );
    free(telement);
    if(node==NULL){
        //fprintf(stderr,"Sensor " "%" SCNd16 " is not found !"  "\n", id);
        return 0;
    }
    return ((element*)node->element)->ts;
}
int datamgr_get_total_sensors(){
    return dpl_size(  list );
}

void datamgr_free()
{
    dpl_free(&list, true);
}

