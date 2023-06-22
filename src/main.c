#include "argsparser.h"
#include "logger.h"
#include "filerules.h"
#include <signal.h>

/// 服务器的配置信息
struct Config server_config = {0};

/// 初始化服务器
static void init();

/// 优雅地释放资源、关闭服务器
static void graceful_shutdown(int sig);

int main(int argc, char **argv)
{
    // 从命令行参数中解析出配置信息
    parse_args(argc, argv, &server_config);
    if (server_config.debug_level) {
        dump_args(&server_config);
    }

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

    // 注册 Ctrl + C 信号处理函数
    signal(SIGINT, graceful_shutdown);
}

static void graceful_shutdown(int sig)
{
    // 停止各个模块
    free_filerules();

    // 请最后释放日志系统，因为日志系统运行在主线程
    free_logger();
}
