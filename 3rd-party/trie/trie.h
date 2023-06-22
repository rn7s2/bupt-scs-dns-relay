/**
    Code for https://journaldev.com article
    Purpose: A Trie Data Structure Implementation in C
    @author: Vijay Ramachandran
    @modified: rn7s2
*/

#ifndef BUPT_SCS_DNS_RELAY_TRIE_H
#define BUPT_SCS_DNS_RELAY_TRIE_H

// The number of children for each node
// We will construct a N-ary tree and make it
// a Trie
// Since we have 26 english letters, we need
// 26 children per node
#define N 26

typedef struct TrieNode TrieNode;

struct TrieNode {
    // The Trie Node Structure
    // Each node has N children, starting from the root
    // and a flag to check if it's a leaf node
    char data; // Storing for printing purposes only
    TrieNode *children[N];
    int is_leaf;
};

/**
 * 创建一个 TrieNode
 * @param data
 * @return
 */
TrieNode *make_trienode(char data);

/**
 * 释放一个 TrieNode 及其子节点
 * @param node
 */
void free_trienode(TrieNode *node);

/**
 * 向 Trie 中插入一个单词
 * @param root
 * @param word
 * @return
 */
TrieNode *insert_trie(TrieNode *root, char *word);

/**
 * 在 Trie 中搜索一个单词
 * @param root
 * @param word
 * @return
 */
int search_trie(TrieNode *root, char *word);

/**
 * 从 Trie 中删除一个单词
 * @param root
 * @param word
 * @return
 */
TrieNode *delete_trie(TrieNode *root, char *word);

/**
 * 打印 Trie 树的结构
 * @param root
 */
void print_trie(TrieNode *root);

/**
 * 在 Trie 中搜索一个单词并打印结果
 * @param root
 * @param word
 */
void print_search(TrieNode *root, char *word);

/**
 * 测试 Trie
 */
void test_trie();

#endif //BUPT_SCS_DNS_RELAY_TRIE_H
