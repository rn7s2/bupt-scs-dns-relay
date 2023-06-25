//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_RULES_H
#define BUPT_SCS_DNS_RELAY_RULES_H

#define MAX_DOMAIN_LEN 256
#define MAX_IP_LEN 16

#include "trie.h"

#include <gmodule.h>

/**
 * 规则: 域名 -> IP
 */
struct Rule {
    char domain[MAX_DOMAIN_LEN];
    char ip[MAX_IP_LEN];
};

/**
 * 按照 文件硬规则, Cache/数据库的顺序查找域名
 * 返回匹配到的 IP. 其中 Cache/数据库 部分使用
 * LRU 算法进行 DNS 缓存
 * @param domain
 * @return
 */
char *match_domain(const char *domain);

#endif //BUPT_SCS_DNS_RELAY_RULES_H
