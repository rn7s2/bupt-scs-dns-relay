//
// Created by rn7s2 on 2023/6/23.
//

#ifndef BUPT_SCS_DNS_RELAY_HANDLER_H
#define BUPT_SCS_DNS_RELAY_HANDLER_H

#include <netinet/in.h>
#include <gmodule.h>

struct RequestArgs {
    int sockfd;                            // 套接字
    struct sockaddr_storage client_addr;   // 客户端地址 (即支持 IPv4, 也支持 IPv6 地址)
    char *buf;                             // 接收到的数据
    int n;                                 // 接收到的数据长度
};

/**
 * 初始化 handler 模块
 */
void init_handler();

/**
 * 运行 handler 模块
 */
void run_handler();

/**
 * 释放 handler 模块的资源
 */
void free_handler();

#endif //BUPT_SCS_DNS_RELAY_HANDLER_H
