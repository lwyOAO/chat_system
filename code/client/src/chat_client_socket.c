#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <pthread.h>
#include "chat_global.h"
#include "chat_client_cmd.h"
#include "chat_config.h"
#include "chat_client_socket.h"
#include "chat_client_scan.h"
#include "logger.h"

#define BUFF_SIZE 1024
#define NAME_LEN 64

WINDOW *display_win, *input_win;
int sockfd;
char who[NAME_LEN];
extern RECV_MAP_T g_recv_cmdTable[];
char target_id[10];
Client_g client_info;

void build_header(int type, char* target_id, char* buffer)
{
    sprintf(buffer, "%d ", type);
    sprintf(buffer+strlen(buffer), "%d ", HEADER_SIZE);
    sprintf(buffer+strlen(buffer), "%s ", target_id);
    sprintf(buffer+strlen(buffer), "%s ", client_info.client_id);
    sprintf(buffer+strlen(buffer), "%s\n", client_info.online_id);
}

void *receive_message(void *arg)
{
    char buffer[BUFF_SIZE];
    while (1)
    {
        Custom_header header = {0, 0, "0", "0", "0", -1};
        char *lines[10];
        int line_count = 0;
        memset(buffer, 0, sizeof(buffer));

        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            // wprintw(display_win, "Received: %s\n", buffer);
            // wrefresh(display_win);

            parse_cmd(buffer, &header, lines, &line_count);

            for (int i = 0; g_recv_cmdTable[i].cmd_code != -1; i++)
            {
                if (g_recv_cmdTable[i].cmd_code == header.type)
                {
                    g_recv_cmdTable[i].func(line_count, &header, buffer, lines);
                }
            }
        } else if(bytes_received <= 0)
        {
            mvwprintw(display_win, 1, 1, "Server disconnected or error occurred\n");
            wrefresh(display_win);
            endwin();
            close(sockfd);
            exit(EXIT_SUCCESS);
        }
    }
    return NULL;
}

int chat_client_start(char *ip, int port)
{
    pthread_t recv_thread, scan_thread;
    struct sockaddr_in server_addr;
    char message[BUFF_SIZE];
    int ret = 0;

    // 初始化客户信息结构体
    strcpy(client_info.client_id, "0");
    strcpy(client_info.online_id, "0");
    strcpy(client_info.username, "0");
    strcpy(target_id, "0");

    // 初始化窗口
    initscr();
    raw();
    keypad(stdscr, TRUE);
    
    //noecho();

    // 设置颜色
    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    display_win = newwin(LINES-3, COLS, 0, 0);
    scrollok(display_win, TRUE);
    //box(display_win, 0, 0);
    wrefresh(display_win);

    input_win = newwin(3, COLS, LINES-3, 0);
    //box(input_win, 0, 0);
    wrefresh(input_win);

    printw("Connecting to server....\n");
    refresh();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG_ERROR("Socket create failed");
        perror("Socket create failed");
        endwin();
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        LOG_ERROR("建立连接失败, IP: %s, port: %d", ip, port);
        close(sockfd);
        endwin();
        exit(EXIT_FAILURE);
    }

    mvwprintw(display_win, 1, 1, "Connected to server\n");
    wrefresh(display_win);

    pthread_create(&recv_thread, NULL, receive_message, NULL);
    pthread_create(&scan_thread, NULL, handle_recv_scan, NULL);

    strcpy(who, "who");
    while (1)
    {
        attron(COLOR_PAIR(1));
        mvwprintw(input_win, 1, 1, "To %s > ", who);
        attroff(COLOR_PAIR(1));
        wrefresh(input_win);
        wgetnstr(input_win, message, sizeof(message) -1);

        werase(input_win);

        if(strlen(message) == 0)
        {
            continue;
        }
        //box(input_win, 0, 0);

        ret = cs_client_order_entry(message, sockfd);
        memset(message, '\0', strlen(message));
    }

    pthread_join(recv_thread, NULL);

    close(sockfd);
    endwin();

    return EXIT_SUCCESS;
}