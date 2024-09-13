#include "logger.h"
#include <stddef.h>

static char log_file_name_fmt[256] = {0};

int logger_init(Logger *logger, const char *filename, LogLevel level) {
    logger->level = level;
    pthread_mutex_init(&logger->lock, NULL);
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    
    // 根据日期生成日志文件名
    strftime(logger->filename, sizeof(logger->filename), filename, tm_info);
    strncpy(log_file_name_fmt, filename, 255);
    
    // 打开日志文件
    logger->file = fopen(logger->filename, "a");
    if (logger->file == NULL) {
        fprintf(stderr, "Error opening log file: %s\n", logger->filename);
        return EXIT_FAILURE;
    }

    logger->if_init = 1;

    return EXIT_SUCCESS;
}

void logger_close(Logger *logger) {
    if (logger->file) {
        fclose(logger->file);
    }
    pthread_mutex_destroy(&logger->lock);
}

// 根据日期自动创建新日志文件
void check_and_rotate_log_file(Logger *logger) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char new_filename[256];
    
    // 根据日期生成新的文件名
    strftime(new_filename, sizeof(new_filename), log_file_name_fmt, tm_info);

    if (strcmp(new_filename, logger->filename) != 0) {
        // 日期发生了变化，关闭旧的文件，创建新的文件
        pthread_mutex_lock(&logger->lock);
        
        fclose(logger->file);
        strcpy(logger->filename, new_filename);
        logger->file = fopen(logger->filename, "a");
        if (logger->file == NULL) {
            fprintf(stderr, "Error opening new log file: %s\n", new_filename);
            exit(1);
        }

        pthread_mutex_unlock(&logger->lock);
    }
}

void log_message(Logger *logger, LogLevel level, char* file_name, int line_num, const char *format, ...) {
    if (level < logger->level) {
        return; // 日志级别低于当前设置的日志级别，忽略
    }

    // 检查是否更换文件输出日志
    check_and_rotate_log_file(logger);

    pthread_mutex_lock(&logger->lock); // 锁定，确保线程安全

    // 获取当前时间
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    // 根据日志级别设置前缀
    const char *level_str;
    switch (level) {
        case DEBUG: level_str = "DEBUG"; break;
        case INFO: level_str = "INFO"; break;
        case WARNING: level_str = "WARNING"; break;
        case ERROR: level_str = "ERROR"; break;
        default: level_str = "UNKNOWN"; break;
    }

    // 输出日志时间、级别、文件名、行号
    fprintf(logger->file, "[%s] %s [%s:%d]:     ", time_str, level_str, file_name, line_num);

    // 处理自定义格式的可变参数
    va_list args;
    va_start(args, format);
    vfprintf(logger->file, format, args);
    va_end(args);

    fprintf(logger->file, "\n");
    fflush(logger->file); // 立即刷新

    pthread_mutex_unlock(&logger->lock); // 解锁
}

Logger default_logger;

Logger* init_default_logger(LogLevel level)
{
    if(default_logger.file != NULL ) return;
    logger_init(&default_logger, "log/%Y-%m-%d.log", level);
    return &default_logger;
}

void close_default_logger()
{
    logger_close(&default_logger);
}
