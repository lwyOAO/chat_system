#include <stdbool.h>
#include <stdlib.h>
#include "chat_global.h"
#include "chat_client_socket.h"
#include "chat_config.h"

#define MAX_MAIN_ARGC 8 /*不算程序名*/

GLOBAL config_t client_config;  // 保存配置文件里的键值对
GLOBAL log4c_category_t* logger = NULL;  // 日志器
GLOBAL chat_argv argv_list[MAX_MAIN_ARGC]; // 保存运行程序时输入的命令行参数

int main(int argc, char * argv[])
{
    // 初始化配置
    int ret = init_config(argc, argv, argv_list, &client_config);
    if (ret != 0)
    {
        // TODO
        // 处理各种错误
    }
    
    // 启动客户端
    chat_client_start(client_config.server_host, atoi(client_config.server_port));

    return EXIT_SUCCESS;
}