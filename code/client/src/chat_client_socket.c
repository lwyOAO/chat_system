#include <unistd.h>
#include <log4c.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <pthread.h>
#include "chat_global.h"
#include "chat_client_cmd.h"

#define BUFF_SIZE 1024
#define NAME_LEN 64
WINDOW *display_win, *input_win;
int sockfd;
char who[NAME_LEN];

void *receive_message(void *arg)
{
    char buffer[BUFF_SIZE];
    while (1)
    {
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            wprintw(display_win, "Received: %s\n", buffer);
            wrefresh(display_win);
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
    pthread_t recv_thread;
    struct sockaddr_in server_addr;
    char buffer[BUFF_SIZE];
    char message[BUFF_SIZE];
    int ret = 0;

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
        close(sockfd);
        endwin();
        exit(EXIT_FAILURE);
    }

    mvwprintw(display_win, 1, 1, "Connected to server\n");
    wrefresh(display_win);

    pthread_create(&recv_thread, NULL, receive_message, NULL);

    strcpy(who, "who");
    while (1)
    {
        attron(COLOR_PAIR(1));
        mvwprintw(input_win, 1, 1, "To %s > ", who);
        attroff(COLOR_PAIR(1));
        wrefresh(input_win);
        wgetnstr(input_win, message, sizeof(message) -1);

        werase(input_win);
        printw(message);

        //box(input_win, 0, 0);

        ret = cs_client_order_entry(message, sockfd);
        memset(message, '\0', strlen(message));
    }

    pthread_join(recv_thread, NULL);

    close(sockfd);
    endwin();

    return EXIT_SUCCESS;
}