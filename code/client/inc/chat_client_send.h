#ifndef __CHAT_CLIENT_SEND_H__
#define __CHAT_CLIENT_SEND_H__

#include "chat_global.h"

#define HEADER_SIZE 20

typedef struct {
    int sign_up_id;
    int client_id;
    char username[64];
} Client_g;

void serialize_header(int type, int target_id, char* buffer);
void deserialize_header(char* buffer, Custom_header* header);

#endif