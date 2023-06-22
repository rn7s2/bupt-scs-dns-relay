//
// Created by rn7s2 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_LOGGER_H
#define BUPT_SCS_DNS_RELAY_LOGGER_H

#define MAX_LOG_BUF 512

enum LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    KILL
};

/**
 * 初始化日志系统
 */
void init_logger();

/**
 * 运行日志系统
 */
void run_logger();

/// 停止日志系统，并释放资源
void free_logger();

/// 打印一条调试信息
void debug(const char *format, ...);

/// 打印一条信息
void info(const char *format, ...);

/// 打印一条警告
void warning(const char *format, ...);

/// 打印一条错误
void error(const char *format, ...);

/// 打印一条致命错误
void fatal(const char *format, ...);

#endif //BUPT_SCS_DNS_RELAY_LOGGER_H
