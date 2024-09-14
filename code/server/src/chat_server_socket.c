#include <unistd.h>
#include <log4c.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "chat_global.h"
#include "chat_server_thread_pool.h"
#include "chat_server_scan.h"
#include "logger.h"

#define MAX_EVENT 10

client_t clients[MAX_CLIENTS];
int client_count = 0;

void broadcast_message(char *message, int sender_sockfd, int len)
{
    // 向客户端发送数据
    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].sockfd != sender_sockfd)
        {
            send(clients[i].sockfd, message, strlen(message), 0);
        }
    }
}

void build_header(Custom_header *header, char *buffer)
{
    sprintf(buffer, "%d ", header->type);
    sprintf(buffer + strlen(buffer), "%d ", header->length);
    sprintf(buffer + strlen(buffer), "%s ", header->target_id);
    sprintf(buffer + strlen(buffer), "%s ", header->client_id);
    sprintf(buffer + strlen(buffer), "%s\n", header->online_id);
}

// void fill_data(char *buff, char *data)
// {
//     sprintf(buff + strlen(buff), "%s", data);
// }

int chat_server_start(char *ip, int port)
{
    int client_sockfd, epollfd;
    struct epoll_event ev, events[MAX_EVENT];
    int nfds, i;
    struct sockaddr_in client_addr;
    socklen_t clnt_addr_size = sizeof(client_addr);
    char buffer[BUFF_SIZE];
    pthread_t scan_thread;

    // 创建套接字
    /*
    int socket(int af, int type, int protocol);
    af:地址族 常用的有 AF_INET(IPv4 地址) 和 AF_INET6(IPv6)
    type:数据传输方式常用的有 SOCK_STREAM 和 SOCK_DGRAM
    protocol:传输协议，常用的有 IPPROTO_TCP(TCP 传输协议) 和 IPPTOTO_UDP(UDP 传输协议)
    返回值：非负描述符 – 成功，-1 - 出错
    */
    int server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sockfd < 0)
    {
        perror("Socket create failed!");
        exit(EXIT_FAILURE);
    }

    // 将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // 每个字节都用0填充

    serv_addr.sin_family = AF_INET;            // 使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr(ip); // 具体的IP地址
    serv_addr.sin_port = htons(port);          // 端口

    int ret = bind(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        perror("Bind failed!");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    // 进入监听状态，等待用户发起请求
    /*
    int listen(int sock, int backlog);
    sock:为需要进入监听状态的套接字
    backlog:为请求队列的最大长度
    */
    if (listen(server_sockfd, 20) < 0)
    {
        perror("Listen failed!");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1 failed");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = server_sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sockfd, &ev) == -1)
    {
        perror("epoll_ctl: server_sockfd");
        close(server_sockfd);
        close(epollfd);
        exit(EXIT_FAILURE);
    }

    // Initialize the thread pool
    init_thread_pool();
    // 创建局域网扫描线程
    pthread_create(&scan_thread, NULL, scan_local_client, NULL);

    while (1)
    {
        // wait for events
        nfds = epoll_wait(epollfd, events, MAX_EVENT, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            close(server_sockfd);
            close(epollfd);
            exit(EXIT_FAILURE);
        }

        // loop through the events
        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == server_sockfd)
            {
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_addr, &clnt_addr_size);
                if (client_sockfd < 0)
                {
                    perror("accept failed");
                    continue;
                }

                // add new client socket to epoll
                ev.events = EPOLLIN;
                ev.data.fd = client_sockfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1)
                {
                    perror("epoll_ctl: client_sockfd");
                    close(client_sockfd);
                    continue;
                }

                clients[client_count].sockfd = client_sockfd;
                clients[client_count++].client_addr = client_addr;
                printf("new client connect\n");
            }
            else
            {
                // data from an existing client
                int client_sockfd = events[i].data.fd;
                int bytes_read = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_read > 0)
                {
                    buffer[bytes_read] = '\0';

                    // Allocate memory for the task
                    task_t *task = malloc(sizeof(task_t));
                    task->sockfd = client_sockfd;
                    strncpy(task->message, buffer, BUFF_SIZE);
                    task->message_len = bytes_read;
                    task->type = TCP;

                    LOG_DEBUG("receive one message: %s\n", buffer);

                    // Add the task to the thread pool
                    add_task_to_pool(task);
                }
                else if (bytes_read == 0)
                {
                    printf("Client disconnected\n");
                    close(client_sockfd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, client_sockfd, NULL);
                    for (int j = 0; j < client_count; j++)
                    {
                        if (clients[j].sockfd == client_sockfd)
                        {
                            clients[j] = clients[--client_count];
                            break;
                        }
                    }
                }
                else
                {
                    perror("recv error");
                    close(client_sockfd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, client_sockfd, NULL);
                }
            }
        }
    }

    // 关闭套接字
    close(server_sockfd);
    close(epollfd);
    return 0;
}