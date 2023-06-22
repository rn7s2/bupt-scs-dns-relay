//
// Created by rn7s2 on 2023/6/23.
//

#include "handler.h"
#include "util.h"

void init_handler()
{
    // TODO

    // 初始化线程池
    // 线程池中的最合理线程数为 CPU 核心数 + 1
    // 但是由于程序日志系统、DNS 规则文件轮询系统占用了两个线程
    // 所以此处线程池中分配为 CPU 核心数 - 1
    worker_pool = g_thread_pool_new(NULL, NULL, get_cpu_num() - 1, FALSE, NULL);
}

void free_handler()
{
    // TODO
}
