#include "chat_client_send.h"
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

Client_g client_info;

void serialize_header(int type, int target_id, char* buffer)
{
    // int *ptr = (int*)buffer;
    // ptr[0] = htonl(type);
    // ptr[1] = htonl(HEADER_SIZE);
    // ptr[2] = htonl(target_id);
    // ptr[3] = htonl(client_info.client_id);
    // ptr[4] = htonl(client_info.sign_up_id);
    sprintf(buffer, "%d", type);
    sprintf(buffer+strlen(buffer), "%d", HEADER_SIZE);
    sprintf(buffer+strlen(buffer), "%d", target_id);
    sprintf(buffer+strlen(buffer), "%d", client_info.client_id);
    sprintf(buffer+strlen(buffer), "%d", client_info.sign_up_id);
}

void deserialize_header(char* buffer, Custom_header* header)
{
    int* ptr = (int*)buffer;
    header->type = ntohl(ptr[0]);
    header->length = ntohl(ptr[1]);
    header->target_id = ntohl(ptr[2]);
    header->client_id = ntohl(ptr[3]);
}