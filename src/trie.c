#include "trie.h"

#include <stdlib.h>
#include <string.h>

struct TrieNode *make_trienode(void *data)
{
    struct TrieNode *node = (struct TrieNode *) calloc(1, sizeof(struct TrieNode));
    for (int i = 0; i < N; i++)
        node->children[i] = NULL;
    node->is_leaf = 0;
    node->data = data;
    return node;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
void free_trienode(struct TrieNode *node, int free_data)
{
    for (int i = 0; i < N; i++) {
        if (node->children[i] != NULL) {
            free_trienode(node->children[i], free_data);
        } else {
            continue;
        }
    }
    if (free_data) {
        free(node->data);
    }
    free(node);
}
#pragma clang diagnostic pop

struct TrieNode *insert_trie(struct TrieNode *root, const char *word, void *data)
{
    // 假定：word 只包含大写字母
    struct TrieNode *temp = root;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = (int) word[i] - FIRST_CHAR;
        if (temp->children[idx] == NULL) {
            // 如果不存在，那么创建子节点
            temp->children[idx] = make_trienode(NULL);
        } else {
            // 如果已经存在，那么不需要再创建
        }
        // 向下一层移动
        temp = temp->children[idx];
    }
    // 到达单词结尾，设置 is_leaf 为 1
    // 并且保存对应的 data 给 cell
    temp->is_leaf = 1;
    temp->data = data;
    return root;
}

void *search_trie(struct TrieNode *root, const char *word)
{
    struct TrieNode *temp = root;

    for (int i = 0; word[i] != '\0'; i++) {
        int position = word[i] - FIRST_CHAR;
        if (temp->children[position] == NULL)
            return NULL;
        temp = temp->children[position];
    }
    if (temp != NULL && temp->is_leaf == 1)
        return temp->data;
    return NULL;
}
