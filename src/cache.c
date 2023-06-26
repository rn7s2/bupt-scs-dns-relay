//
// Created by 雷瑞祺 on 2023/6/22.
//

#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "trie.h"
#include "args.h"

extern struct Config server_config;

static struct Cache cache;

/// 用于保护 cache 的互斥锁
static pthread_mutex_t cache_mutex;

void init_cache()
{
    pthread_mutex_init(&cache_mutex, NULL);

    cache.cached = 0;
    cache.root = make_trienode(NULL);
    cache.cache_mru_first = NULL;
}

void free_cache()
{
    pthread_mutex_lock(&cache_mutex);
    free_trienode(cache.root, 0);
    g_list_free_full(cache.cache_mru_first, free);
    pthread_mutex_unlock(&cache_mutex);

    pthread_mutex_destroy(&cache_mutex);
}

struct DnsAnswer *match_cacherules(struct DnsQuestion *question)
{
    pthread_mutex_lock(&cache_mutex);
    // 从缓存中查找，如果找到且未超时，则返回成功
    GList *cached_record_cell = search_trie(cache.root, question->qname);
    if (cached_record_cell != NULL) {
        // 判断是否存在记录超时
        struct DnsAnswer *cached_record = cached_record_cell->data;
        int timeout = (cached_record->cached_time + cached_record->ttl <= time(NULL));
        if (!timeout) { // 如果缓存未超时，则返回成功
            // 将这条缓存移动到链表的最前面 (最前为 MRU, 最后为 LRU)
            cache.cache_mru_first = g_list_remove_link(cache.cache_mru_first, cached_record_cell);
            cache.cache_mru_first = g_list_concat(cached_record_cell, cache.cache_mru_first);

            // 返回缓存的副本以避免多线程冲突
            struct DnsAnswer *ret_copy = malloc(sizeof(struct DnsAnswer));
            memcpy(ret_copy, cached_record, sizeof(struct DnsAnswer));
            pthread_mutex_unlock(&cache_mutex);
            return ret_copy;
        } else { // 如果超过 TTL, 先移出 Cache
            --cache.cached;
            // 将 Trie 树中对应节点改为 NULL
            cache.root = insert_trie(cache.root, question->qname, NULL);
            free(cached_record);
            cache.cache_mru_first = g_list_delete_link(cache.cache_mru_first, cached_record_cell);
        }
    }
    // 缓存未命中，返回失败
    pthread_mutex_unlock(&cache_mutex);
    return NULL;
}

void insert_cache(struct DnsQuestion *question, struct DnsAnswer *record)
{
    pthread_mutex_lock(&cache_mutex);
    // 如果 Cache 未满，直接插入
    if (cache.cached < server_config.cache_size) {
        GList *trie_record = search_trie(cache.root, question->qname);
        if (trie_record != NULL) { // 如果已经存在该记录，先从 Cache 删除
            --cache.cached;
            // 将 Trie 树中对应节点改为 NULL
            cache.root = insert_trie(cache.root, question->qname, NULL);
            free(trie_record->data);
            cache.cache_mru_first = g_list_delete_link(cache.cache_mru_first, trie_record);
        }
        ++cache.cached;
        // 将缓存插入链表头部
        cache.cache_mru_first = g_list_prepend(cache.cache_mru_first, record);
        // 将缓存插入 Trie
        cache.root = insert_trie(cache.root, question->qname, cache.cache_mru_first);
    } else { // 如果缓存已满，使用 LRU 算法替换缓存
        // 从链表尾部取出最久未使用的缓存
        GList *lru_resource_link = g_list_last(cache.cache_mru_first);
        // 从 Trie 中删除该缓存
        cache.root = insert_trie(cache.root, ((struct DnsAnswer *) lru_resource_link->data)->qname, NULL);
        // 从链表中删除该缓存
        free(lru_resource_link->data);
        cache.cache_mru_first = g_list_delete_link(cache.cache_mru_first, lru_resource_link);
        // 将新缓存插入链表头部
        cache.cache_mru_first = g_list_prepend(cache.cache_mru_first, record);
        // 将新缓存插入 Trie
        cache.root = insert_trie(cache.root, question->qname, cache.cache_mru_first);
    }
    pthread_mutex_unlock(&cache_mutex);
}
