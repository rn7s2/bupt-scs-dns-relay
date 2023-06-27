//
// Created by rn7s2 on 2023/6/22.
//

#include "args.h"

#include <stdio.h>
#include <stdlib.h>

int parse_args(int argc, char **argv, struct Config *config)
{
    config->port = 53;
    config->cache_size = 2048;
    config->rto = 7500;

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
            case 't':
                config->rto = atoi(cag_option_get_value(&context));
            case 'p':
                config->port = atoi(cag_option_get_value(&context));
                break;
            case 'c':
                config->cache_size = atoi(cag_option_get_value(&context));
                break;
            case 'h':
                printf("用法: dns-relay [OPTION]\n"
                       "OPTION:\n"
                       "  -d, --debug               调试级别 1 (仅输出时间坐标、序号和查询的域名)\n"
                       "  -v, --verbose             调试级别 2 (输出冗长的调试信息)\n"
                       "  -h, --help                显示本帮助信息，然后退出\n"
                       "  -s, --server=VALUE        使用指定的 DNS 服务器 (默认为阿里 DNS)\n"
                       "  -p, --port=VALUE          使用指定的端口号 (默认为 53)\n"
                       "  -c, --cache=VALUE         指定 Cache 最大数量 (默认为 2048)\n"
                       "  -f, --filename=FILE       使用指定的配置文件 (默认为 dnsrelay.txt)\n");
                exit(0);
            default:
                printf("无法识别的选项: %c\n", identifier);
                break;
        }
    }

    // 如果没有指定 DNS 服务器，则使用默认的 DNS 服务器
    if (config->dns_server_ipaddr == NULL) {
        config->dns_server_ipaddr = "223.5.5.5"; // 阿里 DNS
    }

    // 如果没有指定配置文件，则使用默认的配置文件
    if (config->filename == NULL) {
        config->filename = "dnsrelay.txt";
    }

    if (config->debug_level >= 2) {
        dump_args(config);
    }

    return 0;
}

void dump_args(struct Config *config)
{
    printf("debug_level: %d\n", config->debug_level);
    printf("dns_server_ipaddr: %s\n", config->dns_server_ipaddr);
    printf("filename: %s\n", config->filename);
    printf("port: %d\n", config->port);
    printf("cache_size: %d\n", config->cache_size);
    printf("rto: %d\n", config->rto);
}
