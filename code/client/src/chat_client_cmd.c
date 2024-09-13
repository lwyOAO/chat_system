#include "chat_client_cmd.h"
#include <string.h>
#include <sys/socket.h>
#include <ncurses.h>
#include "chat_client_socket.h"
#include "chat_client_win.h"
#include "logger.h"

#define MINI_CODE_ERROR 1

extern Client_g client_info;
extern WINDOW *display_win;
extern char who[64];
extern char target_id[10];

CMD_MAP_T g_client_cmdTable[] =
    {
        /* name -- handler -- code -- cmmand_info -- help*/
        {"signup", handle_sign_up, SIGNUP, "sign up your information", NULL},
        {"signin", handle_sign_in, SIGNIN, "sign in", NULL},
        {"select", handle_select, SELECT, "select target user", NULL},

        // { "server", NULL, NULL, g_client_sub_server_cmdTable, 0, NULL, NULL },

        /* added item above this line */
        {NULL, NULL, 0, NULL, NULL},
};

RECV_MAP_T g_recv_cmdTable[] =
    {
        /* code -- handler  */
        {SIGNUP, recv_sign_up},
        {SIGNIN, recv_sign_in},
        {SEND, recv_msg},
        {NOT_SIGNIN, recv_not_sign_in},

        /* added item above this line */
        {-1, NULL},
};

//**********************************
// 处理命令的函数                  **
//**********************************
int handle_sign_up(int num, int code, char *buffer, char **tokens)
{
    strcpy(client_info.client_id, "0");
    strcpy(client_info.online_id, "0");
    strcpy(target_id, "0");

    build_header(SIGNUP, target_id, buffer);
    // 填充数据
    strcpy(buffer + strlen(buffer), "username: ");
    strcpy(buffer + strlen(buffer), tokens[1]);
    strcpy(buffer + strlen(buffer), "\n");
    strcpy(buffer + strlen(buffer), "password: ");
    strcpy(buffer + strlen(buffer), tokens[2]);
    return 0;
}

int handle_sign_in(int num, int code, char *buffer, char **tokens)
{
    strcpy(client_info.client_id, "0");
    strcpy(client_info.online_id, "0");
    strcpy(target_id, "0");

    build_header(SIGNIN, target_id, buffer);
    // 填充数据
    strcpy(buffer + strlen(buffer), "userid: ");
    strcpy(buffer + strlen(buffer), tokens[1]);
    strcpy(buffer + strlen(buffer), "\n");
    strcpy(buffer + strlen(buffer), "password: ");
    strcpy(buffer + strlen(buffer), tokens[2]);
    return 0;
}

int handle_select(int num, int code, char *buffer, char **tokens)
{
    strcpy(target_id, tokens[1]);
    // 更新 who 变量
    strcpy(who, target_id);
    wprintw(display_win, "system: switch to %s\n", target_id);
    wrefresh(display_win);

    return SELECT;
}

int handle_default_send(int num, int code, char *buffer, char *msg)
{
    build_header(code, target_id, buffer);

    strcpy(buffer + strlen(buffer), "msg: ");

    strcpy(buffer + strlen(buffer), msg);

    return SEND;
}

//**********************************
// 处理接收到的消息的函数           **
//**********************************
int recv_sign_up(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    char *token;

    for (int i = 1; i < line_count; i++)
    {
        token = strtok(lines[i], ": ");
        if (strcmp(token, "code") == 0)
        {
            token = strtok(NULL, ": ");
            if (strcmp(token, "1") == 0)
            {
                show_msg("system", "sign up successfully!");
            }
        }
        else
        {
            wprintw(display_win, "sign up failed!\n");
            wrefresh(display_win);
        }
    }

    return SIGNUP;
}

int recv_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    char *token;

    for (int i = 1; i < line_count; i++)
    {
        token = strtok(lines[i], ": ");
        if (strcmp(token, "code") == 0)
        {
            token = strtok(NULL, ": ");
            if (strcmp(token, "1") == 0)
            {
                show_msg("system", "sign in successfully!");
            }
        }
        else if (strcmp(token, "online_id") == 0)
        {
            token = strtok(NULL, ": ");
            strcpy(client_info.online_id, token);
        }
        else if (strcmp(token, "user_id") == 0)
        {
            token = strtok(NULL, ": ");
            strcpy(client_info.client_id, token);
        }
    }

    return SIGNIN;
}

int recv_msg(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    char *token;

    // if (strcmp(old_header->target_id, client_info.client_id) != 0)
    // {
    //     return SEND;
    // }

    for (int i = 1; i < line_count; i++)
    {
        token = strtok(lines[i], ": ");
        if (strcmp(token, "msg") == 0)
        {
            token = strtok(NULL, "");

            show_msg(old_header->client_id, token);
        }
    }

    return SEND;
}

int recv_not_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines)
{
    char *token;

    for (int i = 1; i < line_count; i++)
    {
        token = strtok(lines[i], ": ");
        if (strcmp(token, "error") == 0)
        {
            token = strtok(NULL, "");

            show_msg("system", token);
        }
    }

    return NOT_SIGNIN;
}

int cs_client_order_entry(char *order, int sockfd)
{
    char *tokens[10];
    int count = 0;
    char *first_part;
    char *second_part;
    char *token;
    char buffer[HEADER_SIZE + strlen(order) + 50];
    int has_handle = 0;
    int ret;
    int (*handlefunc)(int num, int code, char *buffer, char **tokens);
    int i = 0;

    // 分割出第一个词，查看是否有对应命令
    first_part = strtok(order, " ");
    token = first_part;

    for (i = 0; g_client_cmdTable[i].pCmdName != NULL; i++)
    {
        if (strcmp(first_part, g_client_cmdTable[i].pCmdName) == 0)
        {
            handlefunc = g_client_cmdTable[i].func;
            has_handle = 1;
            break;
        }
    }

    if (has_handle)
    {
        while (token != NULL)
        {
            tokens[count++] = token;
            token = strtok(NULL, " ");
        }
        ret = handlefunc(count, g_client_cmdTable[i].cmd_code, buffer, tokens);
        has_handle = 1;
    } else // 没有对应命令, 将分割的两部分合并
    {
        second_part = strtok(NULL, "");
        if(second_part != NULL)
        {
            *(second_part-1) = ' ';
        }
    }

    // 直接发送消息
    if (!has_handle)
    {
        ret = handle_default_send(count, SEND, buffer, order);
        if (send(sockfd, buffer, strlen(buffer), 0) < 0)
        {
            show_msg("system", "send to server failed!");
        }
        else
        {   
            LOG_DEBUG("send packet: %s", buffer);
            show_msg("Me", order);
        }
    }
    else if (SELECT == ret)
    {
        // 不做处理
    }
    else
    {
        if (send(sockfd, buffer, strlen(buffer), 0) < 0)
        {
            show_msg("system", "send to server failed!");
        } else 
        {
            LOG_DEBUG("send packet: %s", buffer);
        }
    }

    return 0;
}