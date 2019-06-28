#include <stdio.h>
#include "sensor_db.h"
int callback1(void *NotUsed, int argc, char **argv, char **azColName) {
	int i;
        

	for (i = 1; i < argc; i++) {
		printf("%s\t\t", argv[i] ? argv[i] : "NULL");
                 
	}
	printf("\n");
         
	return 0;
}
int main(void){
 	printf("ok\n");
	FILE * ptr;
	ptr = fopen("sensor_data", "r");
char a = 0;
	DBCONN * conn = init_connection(1); 

find_sensor_all(conn, callback1);
disconnect(conn);
   //  disconnect(conn);
     
	
 fclose(ptr);
	return 0;
}
