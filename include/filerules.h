//
// Created by 雷瑞祺 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_FILERULES_H
#define BUPT_SCS_DNS_RELAY_FILERULES_H

#include "rules.h"
#include "dns.h"

/// 来自文件的规则列表
typedef struct TrieNode *FileRules;

/// 初始化来自文件的 DNS 规则模块，该模块轮询热更新 DNS 规则，无需重启服务器
void init_filerules();

/// 释放 filerules 模块的资源
void free_filerules();

/**
 * 从文件中轮询规则，并通过 cache 和 back_cache 的替换来进行双 buffer 无缝更新
 */
_Noreturn void poll_filerules();

/**
 * 在文件规则中匹配 DNS 问题
 * @param question DNS 问题
 * @param resource 要保存查找到的 DNS 资源的指针
 * @return 是否成功，0 为失败，1 为成功
 */
int match_filerules(struct DnsQuestion *question, struct DnsResource *resource);

/**
 * 读取文件规则到 Trie 树中，会判断 IP 是否有效
 * @param rules 指向 Trie 树根节点的指针
 * @return 如果正确读取，返回 1，否则返回 0
 */
int read_rules_to_trie(struct TrieNode **rules);

#endif //BUPT_SCS_DNS_RELAY_FILERULES_H
