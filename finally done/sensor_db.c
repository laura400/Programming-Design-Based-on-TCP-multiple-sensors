#define _GNU_SOURCE

#include "sensor_db.h"
#include <string.h>
#include "config.h"



  
sqlite3 * opendatabase(char * name){
   sqlite3 *db;
 
   int rc;

   rc = sqlite3_open(name, &db);

   if( rc ) {

fifo_log("Unable to connect to SQL server.\n");
    
      
sqlite3_close(db);return(0);
   } else {
 
  fifo_log("Connection to SQL server established\n");
   }
   return db;

}
sqlite3* creattable(char * database,char * table,char clear_up_flag){
   char  sql[500]  ;
   char buf[100];
   int rc=0;
   char *zErrMsg = 0;
   sqlite3 * db=opendatabase(database);
  /*  Create SQL statement */

if(clear_up_flag==1)
    { 
        snprintf(sql,500,"DROP TABLE IF EXISTS %s;" 
                "CREATE TABLE %s(Id INTEGER PRIMARY KEY,sensor_id INT, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);" ,table,table);
    }
else{
   snprintf(sql,500,"CREATE TABLE IF NOT EXISTS %s(Id INTEGER PRIMARY KEY,sensor_id INT, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);",table);
}
  /* sql="CREATE TABLE SMARTPOOL ("  \
         "ID INTEGER PRIMARY KEY      AUTOINCREMENT," \
         "YEAR           INT    NOT NULL," \
         "MONTH           INT    NOT NULL," \
         "DAY           INT    NOT NULL," \
         "HOUR           INT    NOT NULL," \
         "MINUTE           INT    NOT NULL," \
         "SECOND           INT    NOT NULL," \
         "PH            INT     NOT NULL," \
         "TEMPERATURE            INT     NOT NULL);";*/
   //printf("%s\n",sql);
 
   rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);   
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
sqlite3_close(db);
return NULL;
   } else {
 
snprintf(buf,100,"New table <%s> created.",table);
      
      return db;
   }
   
    
}

DBCONN * init_connection(char clear_up_flag)
{
    
    return creattable(TO_STRING(DB_NAME),TO_STRING(TABLE_NAME), clear_up_flag);
 
   
}

void disconnect(DBCONN *conn)
{
    sqlite3_close(conn);
}

int insert_sensor(DBCONN * conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{
    char sql[500];
    char *zErrMsg = 0;
//printf("inserting.... VALUES(%d,   %f,    %ld);\n" ,id, value, ts);
 
    snprintf(sql,500,"INSERT INTO %s (sensor_id,sensor_value,timestamp) VALUES(%d, %f, %ld);" , TO_STRING(TABLE_NAME), id, value, ts);
 
    
    
    int rc = sqlite3_exec(conn, sql, NULL, 0, &zErrMsg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        
        sqlite3_free(zErrMsg);        
        sqlite3_close(conn);
        
        return -1;
    }
    
    return 0;
}

int insert_sensor_from_file(DBCONN * conn, FILE * sensor_data)
{
    
    /*dplist_t* list;
    sensor_data_t * element=malloc(sizeof (sensor_data_t));
    while(!feof(sensor_data)) {
        fread(&(element->id),sizeof( sensor_id_t),1,sensor_data);
        fread(&(element->value),sizeof( sensor_data_t),1,sensor_data);
        fread(&(element->ts),sizeof( sensor_ts_t),1,sensor_data);
        list=dpl_insert_at_reference( list, element, NULL, true);
    }
     
free(element);
dpl_print( list);
return 0;
*/
sensor_id_t id;
sensor_value_t value;
sensor_ts_t ts;
    while(fread(&id,sizeof(id),1,sensor_data)!=0)
    {
  
            
        
        fread(&value,sizeof(value),1,sensor_data);
        fread(&ts,sizeof(ts),1,sensor_data);
        
        if(insert_sensor(conn,id, value, ts)!=0)return -1;
        
         
            
        
    }
    
    return 0;
}

int find_sensor_all(DBCONN * conn, callback_t f)
{
   char sql[500];
   char *zErrMsg = 0;
         snprintf(sql,500,"SELECT * FROM %s" , TO_STRING(TABLE_NAME));
        
    int rc = sqlite3_exec(conn, sql, f, 0, &zErrMsg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed \n");
        fprintf(stderr, "SQL error: %s\n", zErrMsg);

        sqlite3_free(zErrMsg);
        sqlite3_close(conn);
        
        return -1;
    }
    
    return 0;
}

int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f)
{
    char sql[500];
   char *zErrMsg = 0;
    snprintf(sql,500, "SELECT * FROM %s WHERE sensor_value= %f" , TO_STRING(TABLE_NAME), value);
    
    int rc = sqlite3_exec(conn, sql, f, 0, &zErrMsg);
     if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed \n");
        fprintf(stderr, "SQL error: %s\n", zErrMsg);

        sqlite3_free(zErrMsg);
        sqlite3_close(conn);
        
        return -1;
    }
    
    return 0;
}

int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f)
{
    char sql[500];
   char *zErrMsg = 0;
    snprintf(sql,500, "SELECT * FROM %s WHERE sensor_value > %f" , TO_STRING(TABLE_NAME), value);
    
    int rc = sqlite3_exec(conn, sql, f, 0, &zErrMsg);
     if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed\n");
        fprintf(stderr, "SQL error: %s\n", zErrMsg);

        sqlite3_free(zErrMsg);
        sqlite3_close(conn);
        
        return -1;
    }
    
    return 0;
}

int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f)
{
 char sql[500];
   char *zErrMsg = 0;
    snprintf(sql,500, "SELECT * FROM %s WHERE timestamp = %ld" , TO_STRING(TABLE_NAME), ts);
    
    int rc = sqlite3_exec(conn, sql, f, 0, &zErrMsg);
     if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed \n");
        fprintf(stderr, "SQL error: %s\n", zErrMsg);

        sqlite3_free(zErrMsg);
        sqlite3_close(conn);
        
        return 1;
    }
    
    return 0; 
}

int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f)
{
    char *sql ; 
    char *err_msg = 0;
    asprintf(&sql, "SELECT * FROM %s WHERE timestamp > %ld" , TO_STRING(TABLE_NAME), ts);
    
    int rc = sqlite3_exec(conn, sql, f, 0, &err_msg);
     if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(conn);
        
        return 1;
    }
    
    return 0; 
}
