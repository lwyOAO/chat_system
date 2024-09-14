#include "chat_server_cmd.h"
#include "chat_global.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "chat_server_mysql.h"
#include "chat_server_mysql_friends.h"
#include "chat_server_socket.h"
#include "chat_server_utils.h"
#include "logger.h"

#define USER_ID_LEN 9

CMD_MAP_T g_server_cmdTable[] =
    {
        /* code -- handler  */
        {SIGNUP, handle_sign_up},
        {SIGNIN, handle_sign_in},
        {SEND, handle_send},
        {NOT_SIGNIN, handle_not_sign_in},
        {GET_FRIEND_LIST, handle_get_friend_list},
        {ADD_FRIEND, handle_add_friend},

        /* added item above this line */
        {-1, NULL},
};

int generate_id(int len, char* id)
{
    int i = 0;
    srand(time(NULL));

    // 随机生成 len 位数字
    for(i = 0; i < len; i++)
    {
        id[i] = '0' + rand() % 10;
    }

    id[len] = '\0';

    return EXIT_SUCCESS;
}

int generate_user_id(int len, char* id)
{
    generate_id(len, id);
    // 查询比较id是否重复
    return EXIT_SUCCESS;
}

int generate_online_id(int len, char* id)
{
    generate_id(len, id);
    // 查询比较id是否重复
    return EXIT_SUCCESS;
}

int handle_sign_up(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    char *token;
    mysql_users user;

    for (int i = 1; i < line_count; i++)
    {
        token = strtok(lines[i], ": ");
        if(strcmp(token, "username") == 0)
        {
            token = strtok(NULL, ": ");
            strcpy(user.user_name, token);
        } else if(strcmp(token, "password") == 0)
        {
            token = strtok(NULL, ": ");
            strcpy(user.password, token);
        }
    }

    // 生成随机user_id
    generate_id(USER_ID_LEN, user.user_id);

    // 插入数据库
    insert_user(&user);

    // 修改头部
    strcpy(old_header->client_id, user.user_id);
    strcpy(old_header->target_id, user.user_id);

    // 构建响应包头部
    build_header(old_header, buffer);
    sprintf(buffer + strlen(buffer), "code: 1\nsign up successfully");

    return SIGNUP;
}

int handle_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    LOG_DEBUG("ENTER handle_sign_in");
    char *token;
    mysql_users user;
    char user_id[10] = {0};
    char password[16] = {0};
    char online_id[10] = {0};

    for (int i = 1; i < line_count; i++)
    {
        token = strtok(lines[i], ": ");
        if(strcmp(token, "userid") == 0)
        {
            token = strtok(NULL, ": ");
            strcpy(user_id, token);
        } else if(strcmp(token, "password") == 0)
        {
            token = strtok(NULL, ": ");
            strcpy(password, token);
        }
    }

    strcpy(old_header->client_id, user_id);
    strcpy(old_header->target_id, user_id);

    // 构建响应包头部
    build_header(old_header, buffer);

    if(find_user_by_id(user_id, &user) == 0)
    {
        if(strcmp(password, user.password) == 0)
        {
            // 生成在线id
            generate_online_id(USER_ID_LEN, online_id);
            add_online_user(user_id, old_header->sockfd, online_id);
            sprintf(buffer + strlen(buffer), "%s\nonline_id: %s\nuser_id: %s", "code: 1", online_id, old_header->client_id);

            return SIGNIN;
        }
    }

    sprintf(buffer + strlen(buffer), "code: 0");

    LOG_DEBUG("Leave handle_sign_in");

    return SIGNIN;
}

int handle_send(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    strcpy(old_header->online_id, "0");
    build_header(old_header, buffer);

    for(int i = 1; i < line_count; i++)
    {
        sprintf(buffer + strlen(buffer), "%s\n", lines[i]);
    }
    
    return SEND;
}

int handle_not_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    strcpy(old_header->target_id, old_header->client_id);

    build_header(old_header, buffer);
    sprintf(buffer + strlen(buffer), "error: %s\n", "you have not sign in");

    return NOT_SIGNIN;
}

int handle_get_friend_list(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    LOG_DEBUG("ENTER handle_get_friend_list");
    mysql_friends* friend_list = NULL;
    mysql_friends* ptr = NULL;

    strcpy(old_header->target_id, old_header->client_id);
    build_header(old_header, buffer);

    friend_list = find_friends_by_id(old_header->client_id);
    if(friend_list != NULL)
    {
        sprintf(buffer + strlen(buffer), "code: 1\n");
    } else 
    {
        sprintf(buffer + strlen(buffer), "code: 0\n");
    }

    while(friend_list != NULL)
    {
        ptr = friend_list;
        sprintf(buffer + strlen(buffer), "%s %s %d %d %d %d\n", friend_list->user_id_1, friend_list->user_id_2,
                    friend_list->black_1, friend_list->black_2, friend_list->white_1, friend_list->white_2);
        friend_list = friend_list->next;
        free(ptr);
    }

    LOG_DEBUG("Leave handle_get_friend_list");

    return GET_FRIEND_LIST;
}

int handle_add_friend(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    mysql_friends friend;

    strcpy(friend.user_id_1, old_header->client_id);
    strcpy(friend.user_id_2, old_header->target_id);
    friend.black_1 = 0;
    friend.black_2 = 0;
    friend.next = NULL;
    friend.white_1 = 0;
    friend.white_2 = 0;

    insert_friend(&friend);
    
    strcpy(old_header->target_id, old_header->client_id);
    build_header(old_header, buffer);

    sprintf(buffer + strlen(buffer), "code: 1");

    return ADD_FRIEND;
}