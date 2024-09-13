#include "chat_config.h"
#include <string.h>
#include "logger.h"

#define CUSTOM_CONFIG_PATH   "config/chat_config.yaml"

GLOBAL bool cs_parse_config(char *config_path, config_t* config)
{
    char current_key[128] = {0};
    yaml_parser_t parser;
    yaml_token_t token;
    int state = 0;
    char* datap;
    char* tk;

    FILE *fh = fopen(config_path, "r");
    if (!fh)
    {
        perror("Failed to open file");
        LOG_ERROR("Failed to open file");
        return false;
    }

    // 创建YAML解析器
    if (!yaml_parser_initialize(&parser))
    {
        fputs("Failed to initialize parser!\n", stderr);
        LOG_ERROR("Failed to initialize parser!");
        return false;
    }

    // 将文件设置为解析器的输入
    yaml_parser_set_input_file(&parser, fh);

    memset(config, 0, sizeof(config_t));

    // 逐个获取 token
    do
    {
        yaml_parser_scan(&parser, &token);
        switch(token.type)
        {
            case YAML_KEY_TOKEN:     state = 0; break;
            case YAML_VALUE_TOKEN:   state = 1; break;
            case YAML_SCALAR_TOKEN:
                tk = token.data.scalar.value;
                if (state == 0) {
                    /* It's safe to not use strncmp as one string is a literal */
                    if (!strcmp(tk, "port")) {
                        datap = config->server_port;
                    } else if (!strcmp(tk, "host")) {
                        datap = config->server_host;
                    } else if (!strcmp(tk, "level")) {
                        datap = config->log_level;
                    } else if (!strcmp(tk, "file")) {
                        datap = config->log_file;
                    } else {
                        printf("key: %s\n", tk);
                    }
                } else {
                    strcpy(datap, tk);
                    //*datap = strdup(tk);
                }
                break;
            default: break;
        }

        if(YAML_STREAM_END_TOKEN != token.type)
        {
            yaml_token_delete(&token);
        }
    }while (YAML_STREAM_END_TOKEN != token.type);
    
    // 清理
    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(fh);

    return true;
}

int parse_main_argv(chat_argv* argv_list, int argc, char* argv[])
{
    int argv_count = 0;

    if (argc <= 1) 
    {
        return 0;
    }

    for(int i = 1; i < argc; i++)
    {
        if(strncmp(argv[i], "-", 1) == 0)
        {
            strncpy(argv_list[argv_count].name, argv[i], sizeof(argv_list[argv_count].name));
        }
        else 
        {
            LOG_ERROR("argv format error");
            fprintf(stderr, "argv format error");
            exit(-1);
        }

        if(strncmp(argv[i+1], "-", 1) != 0)
        {
            strncpy(argv_list[argv_count].value, argv[i+1], sizeof(argv_list[argv_count].value));
            i++;
        }
        argv_count++;
    }

    return argv_count;
}

bool get_argv_value(chat_argv* argv_list, char* name, char* ret_val)
{
    for(int i = 0; strlen(argv_list[i].name) > 0; i++)
    {
        if(strcmp(argv_list[i].name, name) == 0)
        {
            ret_val = argv_list[i].value;
            return true;
        }
    }
    return false;
}

int init_config(int argc, char * argv[], chat_argv* argv_list, config_t* config)
{
    printf("================开始配置系统===================\n");
    int real_argc = 0;
    char* config_path = NULL;
    bool success = false;

    // 初始化日志系统
    Logger* logger = NULL;
    logger = init_default_logger(DEBUG);
    if( logger->if_init == 0)
    {
        fprintf(stderr, "logger init failed!");
        return EXIT_FAILURE;
    }
    LOG_DEBUG("日志系统初始化成功");

    // 解析程序参数
    real_argc = parse_main_argv(argv_list, argc, argv);

    // 尝试获取配置文件路径
    success = get_argv_value(argv_list, "-p", config_path);
    if(!success)
    {
        config_path = CUSTOM_CONFIG_PATH;
    }
    
    // 获取YAML配置文件各项配置
    success = cs_parse_config(config_path, config);
    if(!success)
    {
        LOG_ERROR("get configure failed!");
        fprintf(stderr, "get configure failed!");
        return EXIT_FAILURE;
    }
    LOG_DEBUG("配置解析完毕");
    LOG_INFO("服务器ip: %s, port: %s", config->server_host, config->server_port);

    printf("================配置系统完毕===================\n");

    return EXIT_SUCCESS;
}

int parse_cmd(char *message, Custom_header *header, char **lines, int *line_count)
{
    char *line;
    char *token;
    char *tokens[5];
    int token_count = 0;

    // 分割每行
    line = strtok(message, "\n");
    while (line != NULL)
    {
        lines[(*line_count)++] = line;
        line = strtok(NULL, "\n");
    }

    // 第一行为头部, 解析头部
    token = strtok(lines[0], " ");
    while (token != NULL)
    {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }

    header->type = atoi(tokens[0]);
    header->length = atoi(tokens[1]);
    strcpy(header->target_id, tokens[2]);
    strcpy(header->client_id, tokens[3]);
    strcpy(header->online_id, tokens[4]);

    return EXIT_SUCCESS;
}