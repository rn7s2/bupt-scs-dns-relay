//
// Created by rn7s2 on 2023/6/22.
//

#include "argsparser.h"

#include <stdio.h>
#include <stdlib.h>

int parse_args(int argc, char **argv, struct config *config)
{
    cag_option_context context;

    // 准备命令行选项解析器
    cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
    while (cag_option_fetch(&context)) {
        char identifier = cag_option_get(&context);
        switch (identifier) {
            case 'd':
                config->debug_level = config->debug_level > 1 ? config->debug_level : 1;
                break;
            case 'm':
                config->debug_level = 2;
                break;
            case 's':
                config->dns_server_ipaddr = cag_option_get_value(&context);
                break;
            case 'f':
                config->filename = cag_option_get_value(&context);
                break;
            case 'h':
                printf("用法: dns-relay [OPTION]\n"
                       "OPTION:\n"
                       "  -d, --debug               调试级别 1 (仅输出时间坐标，序号，查询的域名)\n"
                       "  -v, --verbose             调试级别 2 (输出冗长的调试信息)\n"
                       "  -h, --help                显示本帮助信息，然后退出\n"
                       "  -s, --server=VALUE        使用指定的 DNS 服务器\n"
                       "  -f, --filename=FILE       使用指定的配置文件 (默认为 dnsrelay.txt)\n");
                exit(0);
            default:
                printf("无法识别的选项: %c\n", identifier);
                break;
        }
    }

    return 0;
}

void dump_args(struct config *config)
{
    printf("debug_level: %d\n", config->debug_level);
    printf("dns_server_ipaddr: %s\n", config->dns_server_ipaddr);
    printf("filename: %s\n", config->filename);
}
