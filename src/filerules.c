//
// Created by 雷瑞祺 on 2023/6/22.
//

#include <sys/stat.h>
#include <unistd.h>

#include "filerules.h"
#include "argsparser.h"

extern struct Config server_config;

/// 来自文件的 DNS 规则列表
///
/// 注意：读取时需要加锁
FileRules filerules;

/// 来自文件的 DNS 规则列表的 back buffer，用于无缝更新
static FileRules filerules_back;

/// 用于保护 filerules 和 filerules_back 的互斥锁
pthread_mutex_t filerules_mutex;

pthread_t init_filerules()
{
    pthread_mutex_init(&filerules_mutex, NULL);
    filerules = make_trienode(NULL);
    filerules_back = NULL;

    // 创建一个线程，用于轮询文件规则
    pthread_t file_rules_poller;
    pthread_create(&file_rules_poller, NULL, (void *(*)(void *)) poll_filerules, NULL);
    return file_rules_poller;
}

void free_filerules()
{
    free_trienode(filerules, 1);
    pthread_mutex_destroy(&filerules_mutex);
}

/// 轮询文件规则的时间间隔
#define POLL_INTERVAL 3

_Noreturn void poll_filerules()
{
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
            // TODO: 读取文件规则至 filerules_back

            // 更新最后修改时间
            last_modify_timestamp = current_modify_timestamp;

            // 加锁并交换 filerules 和 filerules_back
            pthread_mutex_lock(&filerules_mutex);
            FileRules temp = filerules;
            filerules = filerules_back;
            filerules_back = temp;
            pthread_mutex_unlock(&filerules_mutex);

            // 清空 filerules_back
            free_trienode(filerules_back, 1);
        }

        sleep(POLL_INTERVAL);
    }
}
