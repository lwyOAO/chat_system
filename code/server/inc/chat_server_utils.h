#ifndef __CHAT_SERVER_UTILS_H__
#define __CHAT_SERVER_UTILS_H__

typedef struct Online_user
{
    char user_id[10];
    int sockfd;
    char online_id[10];
    int first_scan_online;
    struct Online_user* next;
} Online_user;

int find_sockfd_by_id(char* id);
int add_online_user(char* user_id, int sockfd, char* online_id);
Online_user* find_online_user_by_id(char* id);
Online_user* del_online_user_by_id(char* id);
int online_id_exist(char* id);

#endif