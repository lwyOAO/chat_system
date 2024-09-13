#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "chat_client_socket.h"
#include "logger.h"

#define SERVER_PORT 8890
#define BUFFER_SIZE 1024

extern Client_g client_info;

void* handle_recv_scan(void* arg) {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];

    // 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // 初始化客户端地址（用于监听）
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定套接字到客户端端口
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        LOG_ERROR("bind failed");
        perror("bind");
        close(sockfd);
        exit(1);
    }

    while(1)
    {
        addr_len = sizeof(server_addr);
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&server_addr, &addr_len);

        if(recv_len > 0)
        {
            buffer[recv_len] = '\0';
            LOG_INFO("Recv broadcast message: %s", buffer);

            if(sendto(sockfd, client_info.client_id, strlen(client_info.client_id), 0, 
                            (struct sockaddr*)&server_addr, addr_len) < 0)
            {
                LOG_INFO("sendto failed");
                perror("sendto failed");
            } else 
            {
                LOG_INFO("sendto success");
            }
        }
    }

    close(sockfd);
    return 0;
}