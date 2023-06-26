/**
    Code for https://journaldev.com article
    Purpose: A Trie Data Structure Implementation in C
    @author: Vijay Ramachandran
    @modified: rn7s2
*/

#include "trie.h"

#include <stdlib.h>
#include <string.h>

struct TrieNode *make_trienode(void *data)
{
    // Allocate memory for a TrieNode
    struct TrieNode *node = (struct TrieNode *) calloc(1, sizeof(struct TrieNode));
    for (int i = 0; i < N; i++)
        node->children[i] = NULL;
    node->is_leaf = 0;
    node->data = data;
    return node;
}

void free_trienode(struct TrieNode *node, int free_data)
{
    // Free the struct TrieNode sequence
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

struct TrieNode *insert_trie(struct TrieNode *root, const char *word, void *data)
{
    // Inserts the word onto the Trie
    // ASSUMPTION: The word only has upper case characters
    struct TrieNode *temp = root;

    for (int i = 0; word[i] != '\0'; i++) {
        // Get the relative position in the alphabet list
        int idx = (int) word[i] - FIRST_CHAR;
        if (temp->children[idx] == NULL) {
            // If the corresponding child doesn't exist,
            // simply create that child!
            temp->children[idx] = make_trienode(NULL);
        } else {
            // Do nothing. The node already exists
        }
        // Go down a level, to the child referenced by idx
        // since we have a prefix match
        temp = temp->children[idx];
    }
    // At the end of the word, mark this node as the leaf node
    // 并且保存对应的 Cache 块给 cell
    temp->is_leaf = 1;
    temp->data = data;
    return root;
}

void *search_trie(struct TrieNode *root, const char *word)
{
    // Searches for word in the Trie
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
