#ifndef __CHAT_SERVER_SOCKET_H__
#define __CHAT_SERVER_SOCKET_H__

int chat_server_start(char* ip, int port);
void build_header(Custom_header* header, char* buffer);
#endif