#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR
} LogLevel;

typedef struct {
    FILE *file;            // 当前的日志文件
    LogLevel level;        // 日志级别
    pthread_mutex_t lock;  // 互斥锁保证线程安全
    char filename[256];    // 日志文件名
    int if_init;
} Logger;

int logger_init(Logger *logger, const char *filename, LogLevel level);
void logger_close(Logger *logger);
void log_message(Logger *logger, LogLevel level, char* file_name, int line_num, const char *format, ...);
Logger* init_default_logger(LogLevel level);
void close_default_logger();

extern Logger default_logger;

#define LOG_DEBUG(format, args...) log_message(&default_logger, DEBUG, __FILE__, __LINE__, format, ##args)
#define LOG_INFO(format, args...) log_message(&default_logger, INFO, __FILE__, __LINE__, format, ##args)
#define LOG_WARNING(format, args...) log_message(&default_logger, WARNING, __FILE__, __LINE__, format, ##args)
#define LOG_ERROR(format, args...) log_message(&default_logger, ERROR, __FILE__, __LINE__, format, ##args)

#endif