#ifndef __CHAT_SERVER_CMD_H__
#define __CHAT_SERVER_CMD_H__

#include "chat_global.h"

typedef struct __CMD_MAP_T
{
    int	 cmd_code;	/* code */
	int  (*func)(int line_count, Custom_header* header, char* buffer, char** lines); /* handler */	
} CMD_MAP_T;

int handle_sign_up(int line_count, Custom_header *old_header, char *buffer, char **lines);
int parse_cmd(char *message, Custom_header *header, char **lines, int *line_count);
int handle_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines);
int handle_send(int line_count, Custom_header *old_header, char *buffer, char **lines);
int handle_not_sign_in(int line_count, Custom_header *old_header, char *buffer, char **lines);
int handle_get_friend_list(int line_count, Custom_header *old_header, char *buffer, char **lines);
int handle_add_friend(int line_count, Custom_header *old_header, char *buffer, char **lines);
#endif