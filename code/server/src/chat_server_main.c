#include <stdbool.h>
#include <stdlib.h>
#include "chat_global.h"
#include "chat_server_socket.h"
#include "chat_config.h"
#include "chat_server_mysql.h"

#define CUSTOM_CONFIG_PATH   "chat_server_config.yaml"
#define MAX_MAIN_ARGC 8 /*不算程序名*/

GLOBAL config_t server_config;// 保存配置文件里的键值对
GLOBAL chat_argv argv_list[MAX_MAIN_ARGC]; // 保存运行程序时输入的命令行参数

int main(int argc, char * argv[])
{
    // 初始化配置
    int ret = init_config(argc, argv, argv_list, &server_config);
    if (ret != 0)
    {
        // TODO
        // 处理各种错误
    }
    
    // 初始化 MYSQL 数据库
    init_mysql();

    // 启动服务器
    chat_server_start(server_config.server_host, atoi(server_config.server_port));

    return EXIT_SUCCESS;
}