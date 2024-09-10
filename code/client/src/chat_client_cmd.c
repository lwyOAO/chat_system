#include "chat_client_cmd.h"
#include <string.h>
#include <sys/socket.h>
#include "chat_client_send.h"
#include "chat_client_cmd.h"

#define MINI_CODE_ERROR 1

extern Client_g client_info;
int target_id = 0;

CMD_MAP_T g_client_cmdTable[] =
{
	/* name -- handler -- code -- cmmand_info -- help*/
    {"signup", handle_sign_up, SIGNUP, "sign up your information", NULL},
	// { "client", NULL, NULL, g_client_sub_client_cmdTable, 0, NULL, NULL },
	
	// { "server", NULL, NULL, g_client_sub_server_cmdTable, 0, NULL, NULL },

	/* added item above this line */
	{ NULL, NULL, 0, NULL, NULL},
};

int handle_sign_up(int num, int code, char* buffer, char** tokens)
{
    serialize_header(SIGNUP, target_id, buffer);
    // 填充数据
    strcpy(buffer + strlen(buffer), "\n");
    strcpy(buffer + strlen(buffer), "username: ");
    strcpy(buffer + strlen(buffer), tokens[1]);
    strcpy(buffer + strlen(buffer), "\n");
    strcpy(buffer + strlen(buffer), "passwd: ");
    strcpy(buffer + strlen(buffer), tokens[2]);
    return 0;
}

int handle_send(int num, int code, char* buffer, char* tokens[10])
{

}

int cs_client_order_entry(char* order, int sockfd)
{
    char* tokens[10];
    int count = 0;
    char* token;
    char buffer[HEADER_SIZE+strlen(order)+50];
    int has_handle = 0;

    // 分割命令行
    token = strtok(order, " ");
    while (token != NULL)
    {
        tokens[count++] = token;
        token = strtok(NULL, " ");
    }
    
    for(int i = 0; g_client_cmdTable[i].pCmdName != NULL; i++)
    {
        if(strcmp(tokens[0], g_client_cmdTable[i].pCmdName) == 0)
        {
            has_handle = 1;
            g_client_cmdTable[i].func(count, g_client_cmdTable[i].cmd_code, buffer, tokens);
        }
    }

    // 直接发送消息
    if(!has_handle)
    {
        send(sockfd, order, strlen(order), 0);
    } else if(send(sockfd, buffer, strlen(buffer), 0) < 0)
    {
        printf("send to server failed");
    }

    return 0;
}