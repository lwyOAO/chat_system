#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "chat_server_utils.h"

Online_user* online_user_list_head = NULL;
Online_user* online_user_list_tail = NULL;

char online_user_list[16][10] = {0};

int online_id_exist(char* id)
{
    Online_user* node = online_user_list_head->next;
    while(node != NULL)
    {
        if(strcmp(node->online_id, id) == 0)
        {
            return 1;
        }
        node = node->next;
    }

    return 0;
}

int find_sockfd_by_id(char* id)
{
    Online_user* node = online_user_list_head->next;
    while(node != NULL)
    {
        if(strcmp(node->user_id, id) == 0)
        {
            return node->sockfd;
            break;
        }
        node = node->next;
    }

    return -1;
}

Online_user* find_online_user_by_id(char* id)
{
    Online_user* node = online_user_list_head->next;
    while(node != NULL)
    {
        if(strcmp(node->user_id, id) == 0)
        {
            return node;
            break;
        }
        node = node->next;
    }

    return NULL;
}

Online_user* del_online_user_by_id(char* id)
{
    Online_user* pre_node = online_user_list_head;
    Online_user* next_node = online_user_list_head->next;
    while(next_node != NULL)
    {
        if(strcmp(next_node->user_id, id) == 0)
        {
            pre_node->next = next_node->next;
            free(next_node);
            return pre_node;
        }
        next_node = next_node->next;
    }

    return NULL;
}

int add_online_user(char* user_id, int sockfd, char* online_id)
{
    Online_user* new_online_user = (Online_user*)malloc(sizeof(Online_user));
    strcpy(new_online_user->user_id, user_id);
    new_online_user->sockfd = sockfd;
    strcpy(new_online_user->online_id, online_id);
    new_online_user->first_scan_online = 1;
    new_online_user->next = NULL;

    if(online_user_list_head == NULL)
    {
        online_user_list_head = (Online_user*)malloc(sizeof(Online_user));
        online_user_list_head->next = new_online_user;
        online_user_list_tail = new_online_user;
    } else
    {
        online_user_list_tail->next = new_online_user;
        online_user_list_tail = new_online_user;
    }

    return EXIT_SUCCESS;
}