//
// Created by rn7s2 on 2023/6/23.
//

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "handler.h"
#include "args.h"
#include "util.h"
#include "logger.h"
#include "dns.h"

extern struct Config server_config;

/// 用于接受 UDP 数据包的线程
static pthread_t handler_thread;

/// 用于处理 DNS 请求的线程池
static GThreadPool *worker_pool;

void init_handler()
{
    // 初始化线程池，线程池中的线程用于处理 DNS 请求
    // 线程池中的最合理线程数为 CPU 核心数 + 1
    // 但是由于程序日志系统、DNS 规则文件轮询系统、RTT 检测系统占用了 3 个线程
    // 所以此处线程池中分配为 CPU 核心数 - 2
    worker_pool = g_thread_pool_new((GFunc) handle_dns_request, NULL,
                                    get_cpu_num() - 2, TRUE, NULL);
    pthread_create(&handler_thread, NULL, (void *(*)(void *)) run_handler, NULL);
}

void run_handler()
{
    // 分别创建 IPv4 和 IPv6 的 UDP socket
    int sockfd, sockfd6;
    struct sockaddr_in server_addr, client_addr;
    struct sockaddr_in6 server_addr6, client_addr6;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fatal("IPv4 socket 创建失败");
        exit(1);
    }
    if ((sockfd6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        fatal("IPv6 socket 创建失败");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_config.port);

    memset(&server_addr6, 0, sizeof(server_addr6));
    server_addr6.sin6_family = AF_INET6;
    server_addr6.sin6_addr = in6addr_any;
    server_addr6.sin6_port = htons(server_config.port);

    // 分别绑定 IPv4 和 IPv6 的 UDP socket 到命令行参数指定的端口
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fatal("IPv4 socket bind 失败");
        exit(1);
    }
    if (bind(sockfd6, (struct sockaddr *) &server_addr6, sizeof(server_addr6)) < 0) {
        fatal("IPv6 socket bind 失败");
        exit(1);
    }

    int n;
    char buf[MAX_DNSBUF_LEN];
    int maxfdp;
    fd_set rset;

    FD_ZERO(&rset);
    while (1) {
        // 使用 select 进行 I/O 复用，当 IPv4 或 IPv6 socket 接到请求时，将其读取
        FD_SET(sockfd, &rset);
        FD_SET(sockfd6, &rset);
        maxfdp = sockfd > sockfd6 ? sockfd : sockfd6 + 1;
        select(maxfdp, &rset, NULL, NULL, NULL);

        // IPv4 socket 有数据到达
        if (FD_ISSET(sockfd, &rset)) {
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_addr_len = sizeof(client_addr);
            n = (int) recvfrom(sockfd, buf, sizeof(buf), 0,
                               (struct sockaddr *) &client_addr,
                               &client_addr_len);
            if (n < 0) {
                fatal("IPv4 socket recvfrom 失败");
                exit(1);
            }

            if (server_config.debug_level >= 2) {
                debug("RECVREQ from: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            }

            struct RequestArgs *args = malloc(sizeof(struct RequestArgs));
            args->sockfd = sockfd;
            memcpy(&args->client_addr, &client_addr, client_addr_len);
            args->buf = (char *) malloc(sizeof(char) * n);
            memcpy(args->buf, buf, n);
            args->n = n;

            // 将请求交给线程池处理
            g_thread_pool_push(worker_pool, args, NULL);
        }

        // IPv6 socket 有数据到达
        if (FD_ISSET(sockfd6, &rset)) {
            memset(&client_addr6, 0, sizeof(client_addr6));
            socklen_t client_addr6_len = sizeof(client_addr6);
            n = (int) recvfrom(sockfd6, buf, sizeof(buf), 0,
                               (struct sockaddr *) &client_addr6,
                               &client_addr6_len);
            if (n < 0) {
                fatal("IPv6 socket recvfrom 失败");
                exit(1);
            }

            if (server_config.debug_level >= 2) {
                uint16_t *v6_addr = (uint16_t *) &client_addr6.sin6_addr;
                debug("RECVREQ from: [%x:%x:%x:%x]:%d", v6_addr[0], v6_addr[1], v6_addr[2], v6_addr[3], ntohs(client_addr6.sin6_port));
            }

            struct RequestArgs *args = malloc(sizeof(struct RequestArgs));
            args->sockfd = sockfd6;
            memcpy(&args->client_addr, &client_addr6, client_addr6_len);
            args->buf = (char *) malloc(sizeof(char) * n);
            memcpy(args->buf, buf, n);
            args->n = n;

            // 将请求交给线程池处理
            g_thread_pool_push(worker_pool, args, NULL);
        }
    }
}

void free_handler()
{
    // 首先释放负责接受 DNS 请求的线程 handler_thread
    pthread_cancel(handler_thread);
    pthread_join(handler_thread, NULL);

    // 然后释放线程池 worker_pool
    g_thread_pool_free(worker_pool, FALSE, TRUE);
}
