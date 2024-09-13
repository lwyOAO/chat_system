#ifndef __CHAT_SERVER_MYSQL_H__
#define __CHAT_SERVER_MYSQL_H__

typedef struct {
    char user_name[64];
    char user_id[10];
    char password[16];
} mysql_users;

int init_mysql();
int insert_user(mysql_users* user);
int find_user_by_id(char* id, mysql_users* user);
#endif