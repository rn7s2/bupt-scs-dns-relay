//
// Created by rn7s2 on 2023/6/23.
//

#ifndef BUPT_SCS_DNS_RELAY_HANDLER_H
#define BUPT_SCS_DNS_RELAY_HANDLER_H

#include <gmodule.h>

GThreadPool *worker_pool;

/**
 * 初始化 handler 模块
 */
void init_handler();

/**
 * 释放 handler 模块的资源
 */
void free_handler();

#endif //BUPT_SCS_DNS_RELAY_HANDLER_H
