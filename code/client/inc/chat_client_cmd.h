#ifndef __CHAT_CLIENT_CMD_H__
#define __CHAT_CLIENT_CMD_H__

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#define CS_PRINT(args...)   printf(args)

#define CS_PRINTF(format, args...)  do \
	{ \
		CS_PRINT(format, ##args); \
	}while(0)

#define CLRnorm             COLOR("\e[0m")          /* Normal    color        */
#define CLRy                COLOR("\e[0;33m")       /* yellow           */
#define CLRr                COLOR("\e[0;31m")       /* red              */
#define CLRm                COLOR("\e[0;35m")       /* magenta          */
#define CLRgu                COLOR("\e[4;32m")        /* Green          */

typedef struct __CMD_MAP_T
{
	char *		pCmdName;									/* command name */
	int (*func)(int num, int code, char* buffer, char* tokens[10]);			                /* handler */
	int			cmd_code;									/* code */
	char *		formatInfo;									/* command format info */
	char *       helpInfo;									/* help information */
}CMD_MAP_T;

int cs_client_order_entry(char* order, int sockfd);
int handle_sign_up(int num, int code, char* buffer, char* tokens[10]);
#endif