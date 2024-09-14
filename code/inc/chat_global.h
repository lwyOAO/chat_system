#ifndef __CHAT_GLOBAL_H__
#define __CHAT_GLOBAL_H__

#include <stdio.h>

typedef struct 
{
    char server_host[128];
    char server_port[10];
    char log_level[16];
    char log_file[256];
} config_t;

typedef struct {
    char name[32];
    char value[256];
} chat_argv;

typedef struct {
    int type;  // 消息类型
    int length; // 消息长度
    char target_id[10]; // 目标用户id
    char client_id[10]; // 用户id
    char online_id[10]; // 用户已登录的标识, 未登录为0
    int sockfd; // 额外增加的，不加入包头部
} Custom_header;

typedef enum cmd_code {
    SIGNUP = 0,
    SIGNIN,
	SEND,
    SELECT,
    NOT_SIGNIN,
    GET_FRIEND_LIST,
    ADD_FRIEND,
    RECV_FRIEND,
    SHOW_FRIENDS,
    NOTIFY_ONLINE,
} cmd_code;

#define LOCAL   static
#define GLOBAL
#endif