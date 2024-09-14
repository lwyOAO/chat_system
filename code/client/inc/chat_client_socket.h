#ifndef __CHAT_CLIENT_SOCKET_H__
#define __CHAT_CLIENT_SOCKET_H__

#define HEADER_SIZE 20

typedef struct {
    char online_id[10];
    char client_id[10];
    char username[64];
} Client_g;

typedef struct my_friend {
    char id[10];
    int ban;
    int banned;
    int love;
    struct my_friend* next;
} my_friend;

void build_header(int type, char* target_id, char* buffer);

int chat_client_start(char* ip, int port);

#endif