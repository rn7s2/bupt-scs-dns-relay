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
    // ASSUMPTION: The word only has lower case characters
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

static int check_divergence(struct TrieNode *root, const char *word)
{
    // Checks if there is branching at the last character of word
    // and returns the largest position in the word where branching occurs
    struct TrieNode *temp = root;
    int len = (int) strlen(word);
    if (len == 0)
        return 0;
    // We will return the largest index where branching occurs
    int last_index = 0;
    for (int i = 0; i < len; i++) {
        int position = word[i] - FIRST_CHAR;
        if (temp->children[position]) {
            // If a child exists at that position
            // we will check if there exists any other child
            // so that branching occurs
            for (int j = 0; j < N; j++) {
                if (j != position && temp->children[j]) {
                    // We've found another child! This is a branch.
                    // Update the branch position
                    last_index = i + 1;
                    break;
                }
            }
            // Go to the next child in the sequence
            temp = temp->children[position];
        }
    }
    return last_index;
}

char *find_longest_prefix(struct TrieNode *root, const char *word)
{
    // Finds the longest common prefix substring of word
    // in the Trie
    if (!word || word[0] == '\0')
        return NULL;
    // Length of the longest prefix
    int len = (int) strlen(word);

    // We initially set the longest prefix as the word itself,
    // and try to back-tracking from the deepst position to
    // a point of divergence, if it exists
    char *longest_prefix = (char *) calloc(len + 1, sizeof(char));
    for (int i = 0; word[i] != '\0'; i++)
        longest_prefix[i] = word[i];
    longest_prefix[len] = '\0';

    // If there is no branching from the root, this
    // means that we're matching the original string!
    // This is not what we want!
    int branch_idx = check_divergence(root, longest_prefix) - 1;
    if (branch_idx >= 0) {
        // There is branching, We must update the position
        // to the longest match and update the longest prefix
        // by the branch index length
        longest_prefix[branch_idx] = '\0';
        longest_prefix = (char *) realloc(longest_prefix, (branch_idx + 1) * sizeof(char));
    }

    return longest_prefix;
}

int is_leaf_node(struct TrieNode *root, const char *word)
{
    // Checks if the prefix match of word and root
    // is a leaf node
    struct TrieNode *temp = root;
    for (int i = 0; word[i]; i++) {
        int position = (int) word[i] - FIRST_CHAR;
        if (temp->children[position]) {
            temp = temp->children[position];
        }
    }
    return temp->is_leaf;
}

struct TrieNode *delete_trie(struct TrieNode *root, const char *word, int free_data)
{
    // Will try to delete the word sequence from the Trie only it
    // ends up in a leaf node
    if (!root)
        return NULL;
    if (!word || word[0] == '\0')
        return root;
    // If the node corresponding to the match is not a leaf node,
    // we stop
    if (!is_leaf_node(root, word)) {
        return root;
    }
    struct TrieNode *temp = root;
    // Find the longest prefix string that is not the current word
    char *longest_prefix = find_longest_prefix(root, word);
    //printf("Longest Prefix = %s\n", longest_prefix);
    if (longest_prefix[0] == '\0') {
        free(longest_prefix);
        return root;
    }
    // Keep track of position in the Trie
    int i;
    for (i = 0; longest_prefix[i] != '\0'; i++) {
        int position = (int) longest_prefix[i] - FIRST_CHAR;
        if (temp->children[position] != NULL) {
            // Keep moving to the deepest node in the common prefix
            temp = temp->children[position];
        } else {
            // There is no such node. Simply return.
            free(longest_prefix);
            return root;
        }
    }
    // Now, we have reached the deepest common node between
    // the two strings. We need to delete the sequence
    // corresponding to word
    int len = (int) strlen(word);
    for (; i < len; i++) {
        int position = (int) word[i] - FIRST_CHAR;
        if (temp->children[position]) {
            // Delete the remaining sequence
            struct TrieNode *rm_node = temp->children[position];
            temp->children[position] = NULL;
            free_trienode(rm_node, free_data);
        }
    }
    free(longest_prefix);
    return root;
}
