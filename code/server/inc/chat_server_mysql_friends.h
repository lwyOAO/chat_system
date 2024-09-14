#ifndef __CHAT_SERVER_MYSQL_FRIENDS_H__
#define __CHAT_SERVER_MYSQL_FRIENDS_H__

#include "chat_server_mysql.h"

typedef struct mysql_friends{
    char user_id_1[10];
    char user_id_2[10];
    int black_1;
    int black_2;
    int white_1;
    int white_2;
    struct mysql_friends* next;
} mysql_friends;

mysql_friends* find_friends_by_id(char* id);
int insert_friend(mysql_friends* friend);
char** find_common_friends_by_id(char* id);
#endif