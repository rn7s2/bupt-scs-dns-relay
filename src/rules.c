//
// Created by 雷瑞祺 on 2023/6/22.
//

#include <sys/stat.h>
#include <unistd.h>

#include "rules.h"
#include "trie.h"
#include "argsparser.h"

extern struct Config server_config;

/// 来自文件的 DNS 规则列表
///
/// 注意：读取时需要加锁
FileRules file_rules;

/// 来自文件的 DNS 规则列表的 back buffer，用于无缝更新
static FileRules file_rules_back;

/// 用于保护 file_rules 和 file_rules_back 的互斥锁
pthread_mutex_t file_rules_mutex = PTHREAD_MUTEX_INITIALIZER;

/// 初始化来自文件的 DNS 规则模块，该模块轮询热更新 DNS 规则，无需重启服务器
pthread_t init_file_rules()
{
    file_rules = make_trienode(NULL);
    file_rules_back = NULL;

    // 创建一个线程，用于轮询文件规则
    pthread_t file_rules_poller;
    pthread_create(&file_rules_poller, NULL, (void *(*)(void *)) poll_file_rules, NULL);
    return file_rules_poller;
}

/// 轮询文件规则的时间间隔
#define POLL_INTERVAL 3

_Noreturn void poll_file_rules()
{
    time_t last_modify_timestamp = 0;

    while (1) {
        pthread_testcancel();
        // 从文件中读取规则

        // 读取文件的最后修改时间
        struct stat file_stat;
        stat(server_config.filename, &file_stat);
        time_t current_modify_timestamp = file_stat.st_mtime;

        // 如果文件被修改过
        if (current_modify_timestamp != last_modify_timestamp) {
            // TODO: 读取文件规则至 file_rules_back

            // 更新最后修改时间
            last_modify_timestamp = current_modify_timestamp;

            // 加锁并交换 file_rules 和 file_rules_back
            pthread_mutex_lock(&file_rules_mutex);
            FileRules temp = file_rules;
            file_rules = file_rules_back;
            file_rules_back = temp;
            pthread_mutex_unlock(&file_rules_mutex);

            // 清空 file_rules_back
            free_trienode(file_rules_back, 1);
        }

        sleep(POLL_INTERVAL);
    }
}
