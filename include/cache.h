//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_CACHE_H
#define BUPT_SCS_DNS_RELAY_CACHE_H

#include <gmodule.h>

#include "dns.h"

struct Cache {
    int cached;
    struct TrieNode *root;
    GList *cache_mru_first; // 最近使用的缓存在头，最长时间未使用的缓存在尾
};

void init_cache();

void free_cache();

/**
 * 在 Cache 中匹配 DNS 问题
 * @param question DNS 问题
 * @param resource 要保存查找到的 DNS 资源的指针
 * @return 是否成功，0 为失败，1 为成功
 */
int match_cacherules(struct DnsQuestion *question, struct DnsResource *resource);

/**
 * 将 DNS 问题对应的 DNS 资源记录插入 Cache，必要时使用 LRU 算法淘汰
 * @param question DNS 问题
 * @param resource 对应的 DNS 资源记录
 */
void insert_cache(struct DnsQuestion *question, struct DnsResource *resource);

#endif //BUPT_SCS_DNS_RELAY_CACHE_H
