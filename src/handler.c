//
// Created by rn7s2 on 2023/6/23.
//

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handler.h"
#include "args.h"
#include "util.h"
#include "logger.h"

extern struct Config server_config;

pthread_t handler_thread;

GThreadPool *worker_pool;

void init_handler()
{
    // TODO

    // 初始化线程池
    // 线程池中的最合理线程数为 CPU 核心数 + 1
    // 但是由于程序日志系统、DNS 规则文件轮询系统占用了两个线程
    // 所以此处线程池中分配为 CPU 核心数 - 1
    worker_pool = g_thread_pool_new(NULL, NULL, get_cpu_num() - 1, FALSE, NULL);

    pthread_create(&handler_thread, NULL, (void *(*)(void *)) run_handler, NULL);
}

void run_handler()
{
    // TODO 监听端口，接收请求，将请求交给线程池处理
    int sockfd;
    char buf[4096];

    struct sockaddr_in server_addr, client_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fatal("socket 创建失败");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_config.port);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fatal("socket bind 失败");
        exit(1);
    }

    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    while (1) {
        int n = (int) recvfrom(sockfd, buf, sizeof(buf), MSG_WAITALL, (struct sockaddr *) &client_addr,
                               &client_addr_len);
        if (n < 0) {
            fatal("socket recvfrom 失败");
            exit(1);
        }
        buf[n] = '\0';

        // TODO 将请求交给线程池处理
        // buf 中即为请求报文
        g_thread_pool_push(worker_pool, buf, NULL);
    }
}

void free_handler()
{
    // TODO
}
