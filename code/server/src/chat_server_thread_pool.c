#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat_server_thread_pool.h"
#include "chat_server_cmd.h"
#include "chat_global.h"
#include "chat_server_utils.h"
#include "chat_config.h"
#include "logger.h"
#include "chat_server_mysql_friends.h"

thread_pool_t pool;
extern int client_count;
extern client_t clients[MAX_CLIENTS];
extern CMD_MAP_T g_server_cmdTable[];
extern char online_user_list[16][10];
extern Online_user* online_user_list_head;
#define MAX_PACKET_SIZE 1024

// 标记客户端是否在线
void handle_UDP_online(task_t * task)
{
    for(int i = 0; i < 16; i++)
    {
        if(online_user_list[i][0] == '\0')
        {
            strcpy(online_user_list[i], task->message);
        }
    }
}

// 刷新在线用户列表
void handle_UDP_refresh(task_t * task)
{
    Online_user* node = online_user_list_head;
    int if_find = 0;
    while (node != NULL)
    {
        if_find = 0;
        for(int i = 0; i < 16; i++)
        {
            if(strcmp(online_user_list[i], node->user_id) == 0)
            {
                if_find = 1;
                node->first_scan_online = 1;
                break;
            }
        }

        // 没有找到，代表客户端可能已经离线
        if(if_find == 0)
        {
            if(node->first_scan_online == 0)
            {
                // 第二次扫描依旧离线，可认为确实离线
                node = del_online_user_by_id(node->user_id);

            } else 
            {
                node->first_scan_online = 0;
            }
        }

        node = node->next;
    }

    for(int i = 0; i < 16; i++)
    {
        online_user_list[i][0] = '\0';
    }
    LOG_DEBUG("refresh online_user_list");
}

void handle_TCP(task_t *task)
{
    Custom_header header;
    char *lines[10];
    int line_count = 0;
    char buffer[MAX_PACKET_SIZE];
    int target_sockfd;
    cmd_code ret;

    // Process the client's message
    LOG_DEBUG("Thread handling message from client %d: %s\n", task->sockfd, task->message);

    header.sockfd = task->sockfd;

    // 解析报文
    parse_cmd(task->message, &header, lines, &line_count);

    if (strcmp(header.online_id, "0") == 0 && header.type != SIGNIN) // 未登录
    {
        // 不转发信息，告知用户未登录
        header.type = NOT_SIGNIN;
    }
    else if(header.type != SIGNIN && header.type != SIGNUP && !online_id_exist(header.online_id))
    {
        header.type = NOT_SIGNIN;
    }

    for (int i = 0; g_server_cmdTable[i].cmd_code != -1; i++)
    {
        if (g_server_cmdTable[i].cmd_code == header.type)
        {
            ret = g_server_cmdTable[i].func(line_count, &header, buffer, lines);
        }
    }

    // 发送id和目标id都一样，代表是要发回给用户的
    if (strcmp(header.client_id, header.target_id) == 0)
    {
        LOG_DEBUG("prepare send: %s", buffer);
        send(task->sockfd, buffer, strlen(buffer), 0);

        if(SIGNIN == ret)
        {
            // 通知好友
            char** friend_list = find_common_friends_by_id(header.client_id);
            int i = 0;
            while (friend_list[i] != NULL)
            {
                // 过滤离线朋友
                target_sockfd = find_sockfd_by_id(friend_list[i]);
                if(target_sockfd > 0)
                {
                    memset(buffer, 0, sizeof(buffer));
                    strcpy(header.target_id, friend_list[i]);
                    header.type = NOTIFY_ONLINE;
                    notify_my_online(&header, buffer);
                    send(target_sockfd, buffer, strlen(buffer), 0);
                }
                free(friend_list[i]);
                i++;
            }
            free(friend_list);
        }
    }
    else
    {
        target_sockfd = find_sockfd_by_id(header.target_id);

        LOG_DEBUG("forward: %s", buffer);
        if (target_sockfd > 0)
        {
            send(target_sockfd, buffer, strlen(buffer), 0);
        }
        else
        {
            LOG_DEBUG("target not found!");
        }
    }

    free(task);
}

void *worker_thread(void *arg)
{
    while (1)
    {
        task_t *task;

        // Lock the mutex to access the task queue
        pthread_mutex_lock(&pool.mutex);

        // Wait for tasks if queue is empty
        while (pool.task_count == 0)
        {
            pthread_cond_wait(&pool.cond, &pool.mutex);
        }

        // Retrieve task from the queue
        task = pool.tasks[--pool.task_count];

        // Unlock the mutex after retrieving the task
        pthread_mutex_unlock(&pool.mutex);

        if(task->type == TCP)
        {
            handle_TCP(task);
        }
        else if(task->type == UDP_online)
        {
            handle_UDP_online(task);
        }

        else if(task->type == UDP_refresh) 
        {
            handle_UDP_refresh(task);
        }
    }

    return NULL;
}

void init_thread_pool()
{
    pool.task_count = 0;
    pthread_mutex_init(&pool.mutex, NULL);
    pthread_cond_init(&pool.cond, NULL);

    // Create threads in the pool
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, worker_thread, NULL);
        pthread_detach(thread_id); // Detach thread to avoid join
    }
}

void add_task_to_pool(task_t *task)
{
    // Lock the mutex to add a task
    pthread_mutex_lock(&pool.mutex);

    // Add task to the queue
    pool.tasks[pool.task_count++] = task;

    // Signal the condition variable to notify a worker
    pthread_cond_signal(&pool.cond);

    // Unlock the mutex
    pthread_mutex_unlock(&pool.mutex);
}