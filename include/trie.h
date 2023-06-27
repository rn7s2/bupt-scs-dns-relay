#ifndef BUPT_SCS_DNS_RELAY_TRIE_H
#define BUPT_SCS_DNS_RELAY_TRIE_H

/// 每个节点的子节点数
///
/// 由于域名不区分大小写，并且可能包含 0-9, '.' 和 '-'
/// 所以从 ASCII 45 '-' 开始，到 ASCII 90 'Z' 结束
/// 一共 46 个字符，所以 N = 46
#define N 46

/// 子节点字符集的第一个字符（零元）
#define FIRST_CHAR '-'

struct TrieNode {
    void *data;                   // 字符串对应的数据
    struct TrieNode *children[N]; // 子节点
    int is_leaf;                  // 是否是叶子节点
};

/**
 * 创建一个 TrieNode
 * @param data 节点对应的数据
 * @return 创建的节点
 */
struct TrieNode *make_trienode(void *data);

/**
 * 释放一个 TrieNode 及其子节点
 * @param node 要释放的节点
 * @param free_data 是否释放 data
 */
void free_trienode(struct TrieNode *node, int free_data);

/**
 * 向 Trie 中插入一个单词
 * @param root 根节点
 * @param word 要插入的单词
 * @return 插入后的根节点
 */
struct TrieNode *insert_trie(struct TrieNode *root, const char *word, void *data);

/**
 * 在 Trie 中搜索一个单词
 * @param root 根节点
 * @param word 要搜索的单词
 * @return 搜索到的节点数据，如果没有找到则返回 NULL
 */
void *search_trie(struct TrieNode *root, const char *word);

#endif //BUPT_SCS_DNS_RELAY_TRIE_H
