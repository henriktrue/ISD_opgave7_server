/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdlib.h>
#include <sqlite3.h> 
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <string.h>
#include <pthread.h>

pthread_mutex_t lock;
sqlite3 *db;
int rc;
char *sql;
char *zErrMsg = 0;
float result = 0;

struct fetch_t {
    float result;
};

fetch_t Fetch;

static int callback(void *data, int argc, char **argv, char **azColName) {
    int i;
    fetch_t *Fetch = (fetch_t *) data;

    for (i = 0; i < argc; i++) {
        if (strcmp(azColName[i], "TEMPERATURE") == 0) {
            Fetch->result = atof(argv[i]);
            return(0);
        }
    }
    return 0;
}

void sqlite_opendb() {
    rc = sqlite3_open("test.db", &db);

    if (rc) {
        syslog(LOG_ERR, "Can't open database: %s",sqlite3_errmsg(db));
        exit(1);
    }

    // Create table SQL statement. The first column is an autoincrementing
    // primary key called entry, which is pointing to rowid

    sql = (char*) "CREATE TABLE TEMPERATURELOG("\
         "ENTRY INTEGER PRIMARY KEY,"\
         "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"\
         "TEMPERATURE    FLOAT     NOT NULL);";

    //try to lock. Returns 0 when succesfulll
    if (pthread_mutex_lock(&lock) == 0) {
    // Execute SQL statement
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);    
    }
    //the lock is only for adjusting the static value.
    pthread_mutex_unlock(&lock);
    
    if (rc != SQLITE_OK) {
        syslog(LOG_ERR, "SQL error in CREATE TABLE: %s", zErrMsg);
        sqlite3_free(zErrMsg);
    } 
}

void sqlite_closedb() {
    sqlite3_close(db);
}

void sqlite_insert(float value) {
    
    char buffer[200];
    sprintf(buffer, "INSERT INTO TEMPERATURELOG (TEMPERATURE) VALUES (%.1f);", value);
    // Create INSERT statement
    sql = buffer;
    
    //try to lock. Returns 0 when succesfulll
    if (pthread_mutex_lock(&lock) == 0) {
    // Execute SQL statement
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);    
    }
    //the lock is only for adjusting the static value.
    pthread_mutex_unlock(&lock);
    

    if (rc != SQLITE_OK) {
        syslog(LOG_ERR, "SQL error in INSERT: %s", zErrMsg);
        sqlite3_free(zErrMsg);
    } 
}

float sqlite_getlatest() {
    // Create SELECT statement to fetch results latest results
    sql = (char*) "SELECT * FROM TEMPERATURELOG ORDER BY TIMESTAMP DESC LIMIT 1;";

    //try to lock. Returns 0 when succesfulll
    if (pthread_mutex_lock(&lock) == 0) {
    // Execute SQL statement
    rc = sqlite3_exec(db, sql, callback, &Fetch, &zErrMsg);  
    }
    //the lock is only for adjusting the static value.
    pthread_mutex_unlock(&lock);
    
    
    if (rc != SQLITE_OK) {
        syslog(LOG_ERR, "SQL error in getlatest: %s", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return Fetch.result;
}

