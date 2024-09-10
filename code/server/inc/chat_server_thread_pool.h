#ifndef __CHAT_SERVER_THREAD_POOL__
#define __CHAT_SERVER_THREAD_POOL__

#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define THREAD_POOL_SIZE 4
#define MAX_CLIENTS 10
#define BUFF_SIZE 1024

typedef struct
{
    int sockfd;
    char message[BUFF_SIZE];
    int message_len;
} task_t;

typedef struct
{
    task_t *tasks[MAX_CLIENTS];
    int task_count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} thread_pool_t;

typedef struct
{
    struct sockaddr_in client_addr;
    int sockfd;
    char username[64];
    char password[16];
    char userid[8];
} client_t;

void add_task_to_pool(task_t *task);
void init_thread_pool();

#endif