//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_RULES_H
#define BUPT_SCS_DNS_RELAY_RULES_H

#define MAX_DOMAIN_LENGTH 256
#define MAX_IP_LENGTH 16

#include "trie.h"

#include <gmodule.h>

/**
 * 规则: 域名 -> IP
 */
struct Rule {
    char domain[MAX_DOMAIN_LENGTH];
    char ip[MAX_IP_LENGTH];
};

/// 来自文件的规则列表
typedef struct TrieNode *FileRules;

struct Cache {
    struct TrieNode *root;
    GList *cache_most_recently_used_first;
};

/// 初始化来自文件的 DNS 规则模块，该模块轮询热更新 DNS 规则，无需重启服务器
pthread_t init_file_rules();

/**
 * 从文件中轮询规则，并通过 cache 和 back_cache 的替换来进行双 buffer 无缝更新
 */
_Noreturn void poll_file_rules();

#endif //BUPT_SCS_DNS_RELAY_RULES_H
