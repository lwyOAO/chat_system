#ifndef __CHAT_SERVER_MYSQL_H__
#define __CHAT_SERVER_MYSQL_H__

#include <mysql/mysql.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_CONNECTIONS 4

typedef struct {
    char user_name[64];
    char user_id[10];
    char password[16];
} mysql_users;

typedef struct {
    int using;
    MYSQL_STMT* stmt;
} MY_STMT;

typedef struct {
    MYSQL *connections[MAX_CONNECTIONS];
    MY_STMT* stmts[MAX_CONNECTIONS];
    pthread_mutex_t lock;
    int count;
} Connection_pool;

int init_mysql();
MYSQL_STMT *get_stmt();
void release_stmt(MYSQL_STMT *stmt);
int insert_user(mysql_users* user);
int find_user_by_id(char* id, mysql_users* user);

#endif