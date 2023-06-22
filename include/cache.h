//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_CACHE_H
#define BUPT_SCS_DNS_RELAY_CACHE_H

#include <gmodule.h>

struct Cache {
    struct TrieNode *root;
    GList *cache_mru_first; // 最近使用的缓存在头，最长时间未使用的缓存在尾
};

#endif //BUPT_SCS_DNS_RELAY_CACHE_H
