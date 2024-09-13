#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include "logger.h"
#include "chat_server_thread_pool.h"

#define BROADCAST_PORT 8890
#define BROADCAST_IP "255.255.255.255" // 广播地址
#define BUFFER_SIZE 1024
#define BROADCAST_INTERVAL (5 * 60)

void *scan_local_client(void *arg)
{
    int sockfd;
    struct sockaddr_in broadcast_addr, client_addr;
    char message[] = "DISCOVER_CLIENTS";
    char buffer[BUFFER_SIZE];

    // 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // 设置套接字为允许广播
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    // 初始化广播地址
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    socklen_t addr_len = sizeof(client_addr);

    // 定时广播
    while (1)
    {
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr,
                   sizeof(broadcast_addr)) < 0)
        {
            LOG_ERROR("sendto failed");
            perror("sendto failed");
        }
        else
        {
            LOG_INFO("Broadcast message sent");
        }

        while (1)
        {
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;

            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);

            task_t *task = malloc(sizeof(task_t));
            if (recv_len < 0)
            {
                // 超时退出
                LOG_DEBUG("time out");
                task->message_len = 0;
                task->type = UDP_refresh;
                add_task_to_pool(task);
                break;
            }

            buffer[recv_len] = '\0';
            task->sockfd = sockfd;
            strncpy(task->message, buffer, BUFF_SIZE);
            task->message_len = recv_len;
            task->type = UDP_online;

            // Add the task to the thread pool
            add_task_to_pool(task);
            LOG_DEBUG("Rrceive response from client %s: %s", inet_ntoa(client_addr.sin_addr), buffer);
        }

        sleep(BROADCAST_INTERVAL);
    }

    close(sockfd);
    return 0;
}