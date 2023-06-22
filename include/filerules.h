//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_FILERULES_H
#define BUPT_SCS_DNS_RELAY_FILERULES_H

#include "rules.h"

/// 来自文件的规则列表
typedef struct TrieNode *FileRules;

/// 初始化来自文件的 DNS 规则模块，该模块轮询热更新 DNS 规则，无需重启服务器
pthread_t init_filerules();

/// 释放 filerules 模块的资源
void free_filerules();

/**
 * 从文件中轮询规则，并通过 cache 和 back_cache 的替换来进行双 buffer 无缝更新
 */
_Noreturn void poll_filerules();

#endif //BUPT_SCS_DNS_RELAY_FILERULES_H
