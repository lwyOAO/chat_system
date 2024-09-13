#ifndef __CHAT_CLIENT_CMD_H__
#define __CHAT_CLIENT_CMD_H__

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "chat_global.h"

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
	int (*func)(int num, int code, char *buffer, char **tokens);			                /* handler */
	int			cmd_code;									/* code */
	char *		formatInfo;									/* command format info */
	char *       helpInfo;									/* help information */
}CMD_MAP_T;

typedef struct __RECV_MAP_T
{
    int	 cmd_code;	/* code */
	int  (*func)(int line_count, Custom_header* header, char* buffer, char** lines); /* handler */	
} RECV_MAP_T;

int cs_client_order_entry(char* order, int sockfd);
int handle_sign_up(int num, int code, char* buffer, char** tokens);
int handle_sign_in(int num, int code, char* buffer, char** tokens);
int handle_select(int num, int code, char* buffer, char** tokens);
int handle_default_send(int num, int code, char *buffer, char *msg);

int recv_sign_up(int line_count, Custom_header *old_header, char *buffer, char **lines);
int recv_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines);
int recv_msg(int line_count, Custom_header *old_header, char *buffer, char **lines);
int recv_not_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines);

#endif