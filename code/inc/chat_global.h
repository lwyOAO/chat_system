#ifndef __CHAT_GLOBAL_H__
#define __CHAT_GLOBAL_H__

#include <log4c.h>
#include <stdio.h>

extern log4c_category_t* logger;

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
    int target_id; // 目标用户id
    int client_id; // 用户id
    int signup_id; // 用户已登录的标识, 未登录为0
} Custom_header;

enum cmd_code {
    SIGNUP = 0,
    SIGNIN,
	SEND,
};

#define LOCAL   static
#define GLOBAL

#define LOG_DEBUG(format, args...)    log4c_category_debug(logger, format, ##args);
#define LOG_ERR(format, args...)      log4c_category_error(logger, format, ##args);
#endif