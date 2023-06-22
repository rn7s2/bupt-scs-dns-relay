//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_CACHE_H
#define BUPT_SCS_DNS_RELAY_CACHE_H

#include <gmodule.h>

struct Cache {
    struct TrieNode *root;
    GList *cache_most_recently_used_first;
};

#endif //BUPT_SCS_DNS_RELAY_CACHE_H
