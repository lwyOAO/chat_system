#ifndef __CS_CONFIG_H__
#define __CS_CONFIG_H__

#include <stdbool.h>
#include <yaml.h>
#include <log4c.h>
#include "chat_global.h"

// 解析YAML配置文件
bool cs_parse_config(char *config_path, config_t* config);

// 解析程序命令行参数
int parse_main_argv(chat_argv* argv_list, int argc, char* argv[]);

// 获取参数具体值
bool get_argv_value(chat_argv* argv_list, char* name, char* ret_val);

// 初始化日志系统
bool init_log_sys();

// 初始化系统全部配置
int init_config(int argc, char * argv[], chat_argv* argv_list, config_t* client_config);

// 解析头部信息
int parse_cmd(char *message, Custom_header *header, char **lines, int *line_count);
#endif