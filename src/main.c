#include "argsparser.h"
#include "logger.h"
#include <signal.h>

struct config server_config = {0};

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

    init();

    run_logger();
    return 0;
}

static void init()
{
    // 初始化各个模块
    init_logger();

    // 注册 Ctrl + C 信号处理函数
    signal(SIGINT, graceful_shutdown);
}

static void graceful_shutdown()
{
    stop("正在停止服务器……");
}
