//
// Created by 雷瑞祺 on 2023/6/22.
//

#include <malloc.h>
#include <string.h>
#include "cache.h"
#include "trie.h"
#include "args.h"

extern struct Config server_config;

struct Cache cache;

/// 用于保护 cache 的互斥锁
pthread_mutex_t cache_mutex;

void init_cache()
{
    pthread_mutex_init(&cache_mutex, NULL);

    cache.cached = 0;
    cache.root = NULL;
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
    struct DnsAnswer *cached_record = search_trie(cache.root, question->qname);
    if (cached_record != NULL) {
        // 判断是否存在记录超时
        int timeout = 0;
        time_t now = time(NULL);
        for (int i = 0; i < cached_record->size; i++) {
            if (cached_record->resources[i].ttl + cached_record->cached_time > now) {
                timeout = 1;
                break;
            }
        }
        if (!timeout) { // 如果缓存未超时，则返回成功
            // 将这条缓存移动到链表的最前面 (最前为 MRU, 最后为 LRU)
            GList *cached_resource_link = g_list_find(cache.cache_mru_first, cached_record);
            cache.cache_mru_first = g_list_remove_link(cache.cache_mru_first, cached_resource_link);
            cache.cache_mru_first = g_list_prepend(cache.cache_mru_first, cached_resource_link);

            // 返回缓存的副本以避免多线程冲突
            size_t len = sizeof(struct DnsAnswer) + cached_record->size * sizeof(struct DnsResource);
            struct DnsAnswer *ret_copy = malloc(len);
            memcpy(ret_copy, cached_record, len);
            pthread_mutex_unlock(&cache_mutex);
            return ret_copy;
        } else { // 如果超过 TTL, 先移出 Cache
            cache.root = delete_trie(cache.root, question->qname, 0);
            cache.cache_mru_first = g_list_remove(cache.cache_mru_first, cached_record);
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
        // 将缓存插入链表头部
        cache.cache_mru_first = g_list_insert_before(cache.cache_mru_first, cache.cache_mru_first, record);
        // 将缓存插入 Trie
        cache.root = insert_trie(cache.root, question->qname, cache.cache_mru_first);
    } else { // 如果缓存已满，使用 LRU 算法替换缓存
        // 从链表尾部取出最久未使用的缓存
        GList *lru_resource_link = g_list_last(cache.cache_mru_first);
        // 从 Trie 中删除该缓存
        cache.root = delete_trie(cache.root, ((struct DnsAnswer *) lru_resource_link->data)->qname, 1);
        // 从链表中删除该缓存
        cache.cache_mru_first = g_list_remove_link(cache.cache_mru_first, lru_resource_link);
        // 将新缓存插入链表头部
        cache.cache_mru_first = g_list_insert_before(cache.cache_mru_first, cache.cache_mru_first, record);
        // 将新缓存插入 Trie
        cache.root = insert_trie(cache.root, question->qname, cache.cache_mru_first);
    }
    pthread_mutex_unlock(&cache_mutex);
}
