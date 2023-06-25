//
// Created by 雷瑞祺 on 2023/6/22.
//

#include <malloc.h>
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

int match_cacherules(struct DnsQuestion *question, struct DnsResource *resource)
{
    pthread_mutex_lock(&cache_mutex);
    // 从缓存中查找，如果找到且未超时，则返回成功
    struct DnsResource *cached_resource = search_trie(cache.root, question->qname);
    if (cached_resource != NULL) {
        // 如果缓存未超时，则返回成功
        if (cached_resource->recorded_time + cached_resource->ttl <= time(NULL)) {
            *resource = *cached_resource;

            // 将这条缓存移动到链表的最前面 (最前为 MRU, 最后为 LRU)
            GList *cached_resource_link = g_list_find(cache.cache_mru_first, cached_resource);
            cache.cache_mru_first = g_list_remove_link(cache.cache_mru_first, cached_resource_link);
            cache.cache_mru_first = g_list_prepend(cache.cache_mru_first, cached_resource_link);

            pthread_mutex_unlock(&cache_mutex);
            return 1;
        } else { // 如果超过 TTL, 先移出 Cache
            cache.root = delete_trie(cache.root, question->qname, 0);
            cache.cache_mru_first = g_list_remove(cache.cache_mru_first, cached_resource);
        }
    }
    // 缓存未命中，返回失败
    pthread_mutex_unlock(&cache_mutex);
    return 0;
}

void insert_cache(struct DnsQuestion *question, struct DnsResource *resource)
{
    pthread_mutex_lock(&cache_mutex);
    // 如果 Cache 未满，直接插入
    if (cache.cached < server_config.cache_size) {
        // 将缓存插入链表头部
        cache.cache_mru_first = g_list_insert_before(cache.cache_mru_first, cache.cache_mru_first, resource);
        // 将缓存插入 Trie
        cache.root = insert_trie(cache.root, question->qname, cache.cache_mru_first);
    } else { // 如果缓存已满，使用 LRU 算法替换缓存
        // 从链表尾部取出最久未使用的缓存
        GList *lru_resource_link = g_list_last(cache.cache_mru_first);
        // 从 Trie 中删除该缓存
        cache.root = delete_trie(cache.root, ((struct DnsResource *) lru_resource_link->data)->name, 1);
        // 从链表中删除该缓存
        cache.cache_mru_first = g_list_remove_link(cache.cache_mru_first, lru_resource_link);
        // 将新缓存插入链表头部
        cache.cache_mru_first = g_list_insert_before(cache.cache_mru_first, cache.cache_mru_first, resource);
        // 将新缓存插入 Trie
        cache.root = insert_trie(cache.root, question->qname, cache.cache_mru_first);
    }
    pthread_mutex_unlock(&cache_mutex);
}
