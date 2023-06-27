#include "args.h"
#include "logger.h"
#include "filerules.h"
#include "handler.h"
#include "cache.h"
#include <signal.h>

/// 服务器的配置信息
struct Config server_config = {0};

/// 初始化服务器
static void init();

/// 优雅地释放资源、关闭服务器
static void graceful_shutdown(__attribute__((unused)) int sig);

int main(int argc, char **argv)
{
    // 从命令行参数中解析出配置信息
    parse_args(argc, argv, &server_config);

    // 初始化服务器的各个模块
    init();

    // 主线程运行日志系统
    run_logger();
    return 0;
}

static void init()
{
    // 初始化各个模块
    init_logger();
    init_filerules();
    init_cache();
    init_id();
    init_handler();

    // 注册 Ctrl + C 信号处理函数
    signal(SIGINT, graceful_shutdown);

    if (server_config.debug_level >= 1) {
        info("服务器启动成功");
    }
}

static void graceful_shutdown(__attribute__((unused)) int sig)
{
    // 停止各个模块
    free_handler();
    free_id();
    free_cache();
    free_filerules();

    // 请最后释放日志系统，因为日志系统运行在主线程
    free_logger();
}
