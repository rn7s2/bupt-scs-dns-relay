//
// Created by 雷瑞祺 on 2023/6/22.
//

#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "filerules.h"
#include "args.h"
#include "trie.h"
#include "logger.h"
#include "util.h"

extern struct Config server_config;

/// 轮询文件规则的线程
static pthread_t file_rules_poller_thread;

/// 来自文件的 DNS 规则列表
///
/// 注意：读取时需要加锁
static FileRules filerules;

/// 用于保护 filerules 的互斥锁
static pthread_mutex_t filerules_mutex;

/// 来自文件的 DNS 规则列表的 back buffer，用于无缝更新
static FileRules filerules_back;

void init_filerules()
{
    pthread_mutex_init(&filerules_mutex, NULL);
    filerules = make_trienode(NULL);
    filerules_back = NULL;

    // 创建一个线程，用于轮询文件规则
    pthread_create(&file_rules_poller_thread, NULL, (void *(*)(void *)) poll_filerules, NULL);
}

void free_filerules()
{
    pthread_cancel(file_rules_poller_thread);
    pthread_join(file_rules_poller_thread, NULL);

    pthread_mutex_lock(&filerules_mutex);
    free_trienode(filerules, 1);
    pthread_mutex_unlock(&filerules_mutex);
    pthread_mutex_destroy(&filerules_mutex);
}

/// 轮询文件规则的时间间隔
#define POLL_INTERVAL 3

_Noreturn void poll_filerules()
{
    pthread_setcancelstate(PTHREAD_CANCEL_DEFERRED, NULL);

    time_t last_modify_timestamp = 0;

    while (1) {
        // 检查线程是否被取消
        pthread_testcancel();

        // 读取文件的最后修改时间
        struct stat file_stat;
        stat(server_config.filename, &file_stat);
        time_t current_modify_timestamp = file_stat.st_mtime;

        // 如果文件被修改过
        if (current_modify_timestamp != last_modify_timestamp) {
            // 读取文件规则至 filerules_back
            if (!read_rules_to_trie(&filerules_back)) {
                // 如果读取失败，清空 filerules_back
                free_trienode(filerules_back, 1);
                // 等待一段时间后重试
                sleep(POLL_INTERVAL / 2);
                continue;
            }

            // 更新最后修改时间
            last_modify_timestamp = current_modify_timestamp;

            // 加锁并交换 filerules 和 filerules_back
            pthread_mutex_lock(&filerules_mutex);
            swap_ptr(filerules, filerules_back);
            pthread_mutex_unlock(&filerules_mutex);

            // 清空 filerules_back
            free_trienode(filerules_back, 1);
        }

        sleep(POLL_INTERVAL);
    }
}

int match_filerules(struct DnsQuestion *question, struct DnsResource *resource)
{
    pthread_mutex_lock(&filerules_mutex);
    struct DnsResource *result = search_trie(filerules, question->qname);
    if (result == NULL) {
        pthread_mutex_unlock(&filerules_mutex);
        return 0;
    }
    *resource = *result;
    pthread_mutex_unlock(&filerules_mutex);
    return 1;
}

int read_rules_to_trie(struct TrieNode **rules)
{
    *rules = make_trienode(NULL);
    struct TrieNode *root = *rules;

    FILE *fp = fopen(server_config.filename, "r");
    if (fp == NULL) {
        return 0;
    }

    char ip[MAX_DOMAIN_LEN], domain[MAX_DOMAIN_LEN], tmp[MAX_DOMAIN_LEN];
    while ((fscanf(fp, "%s %s", ip, domain) == 2)) {
        if (inet_pton(AF_INET, ip, tmp) || inet_pton(AF_INET6, ip, tmp)) {
            root = insert_trie(root, domain, ip);
        } else {
            error("Invalid IP address: %s\n", ip);
            *rules = root;
            fclose(fp);
            return 0;
        }
    }

    *rules = root;
    fclose(fp);
    return 1;
}
