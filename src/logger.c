//
// Created by rn7s2 on 2023/6/22.
//

#include "logger.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <glib.h>

struct LogMsg {
    enum LogLevel level;
    char msg[MAX_LOG_BUF];
};

/// 日志异步工作队列
static GAsyncQueue *log_queue = NULL;

void init_logger()
{
    // 创建工作队列
    if (log_queue != NULL) {
        g_async_queue_unref(log_queue);
    }
    log_queue = g_async_queue_new();
}

void run_logger()
{
    while (1) {
        struct LogMsg *msg = g_async_queue_pop(log_queue);
        printf("%s\n", msg->msg);
        if (msg->level == KILL) { // 收到停止消息，释放资源并退出
            free(msg);
            g_async_queue_unref(log_queue);
            log_queue = NULL;
            break;
        }
        free(msg);
    }
}

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/**
 * 格式化消息，并向日志工作队列放入一条日志
 * @param level
 * @param format
 * @param ...
 */
static void commit_log(enum LogLevel level, const char *format, ...)
{
    struct LogMsg *log_msg = malloc(sizeof(struct LogMsg));

    log_msg->level = level;
    char localtime[20];
    format_time(localtime);

    va_list args;
    va_start(args, format);
    char buf[MAX_LOG_BUF];
    vsprintf(buf, format, args);
    va_end(args);

    switch (level) {
        case DEBUG:
            sprintf(log_msg->msg, ANSI_COLOR_CYAN "[DEBUG, %s]" ANSI_COLOR_RESET " %s", localtime, buf);
            break;
        case INFO:
            sprintf(log_msg->msg, ANSI_COLOR_GREEN "[INFO, %s]" ANSI_COLOR_RESET " %s", localtime, buf);
            break;
        case WARN:
            sprintf(log_msg->msg, ANSI_COLOR_YELLOW "[WARN, %s]" ANSI_COLOR_RESET " %s", localtime, buf);
            break;
        case ERROR:
            sprintf(log_msg->msg, ANSI_COLOR_MAGENTA "[ERROR, %s]" ANSI_COLOR_RESET " %s", localtime, buf);
            break;
        case FATAL:
            sprintf(log_msg->msg, ANSI_COLOR_RED "[FATAL, %s]" ANSI_COLOR_RESET " %s", localtime, buf);
            break;
        case KILL:
            sprintf(log_msg->msg, ANSI_COLOR_BLUE "[STOP, %s]" ANSI_COLOR_RESET " %s", localtime, buf);
            break;
    }

    g_async_queue_push(log_queue, log_msg);
}

void debug(const char *format, ...)
{
    commit_log(DEBUG, format);
}

void info(const char *format, ...)
{
    commit_log(INFO, format);
}

void warning(const char *format, ...)
{
    commit_log(WARN, format);
}

void error(const char *format, ...)
{
    commit_log(ERROR, format);
}

void fatal(const char *format, ...)
{
    commit_log(FATAL, format);
}

void stop(const char *format, ...)
{
    commit_log(KILL, format);
}
