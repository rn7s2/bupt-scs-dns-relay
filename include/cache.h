//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_CACHE_H
#define BUPT_SCS_DNS_RELAY_CACHE_H

#include <gmodule.h>

#include "dns.h"

/**
 * 缓存的数据结构
 */
struct Cache {
    int cached;
    struct TrieNode *root;
    GList *cache_mru_first; // 最近使用的缓存在头，最长时间未使用的缓存在尾
};

/// 初始化缓存模块
void init_cache();

/// 释放缓存模块
void free_cache();

/**
 * 在 Cache 中匹配 DNS 问题
 * @param question DNS 问题
 * @return 如果匹配成功，则返回对应的 DNS 资源记录，否则返回 NULL
 */
struct DnsAnswer *match_cacherules(struct DnsQuestion *question);

/**
 * 将 DNS 问题对应的 DNS 资源记录插入 Cache，必要时使用 LRU 算法淘汰
 * @param question DNS 问题
 * @param record 对应的 DNS 资源记录
 */
void insert_cache(struct DnsQuestion *question, struct DnsAnswer *record);

#endif //BUPT_SCS_DNS_RELAY_CACHE_H
