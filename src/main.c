#include "argsparser.h"
#include "logger.h"
#include "rules.h"
#include <signal.h>
#include <glib.h>

/// 服务器的线程信息
static struct {
    pthread_t file_rules_poller;
    GThreadPool *dns_worker_pool;
} threads = {0};

struct Config server_config = {0};

/// 初始化服务器
static void init();

/// 优雅地释放资源、关闭服务器
static void graceful_shutdown();

int main(int argc, char **argv)
{
    // 从命令行参数中解析出配置信息
    parse_args(argc, argv, &server_config);
    if (server_config.debug_level) {
        dump_args(&server_config);
    }

    // 初始化服务器的各个模块
    init();

    run_logger();
    return 0;
}

static void init()
{
    // 初始化各个模块
    init_logger();
    threads.file_rules_poller = init_file_rules();

    // 注册 Ctrl + C 信号处理函数
    signal(SIGINT, graceful_shutdown);
}

static void graceful_shutdown()
{
    // 停止各个模块
    pthread_cancel(threads.file_rules_poller);

    stop("正在停止服务器……");
}
