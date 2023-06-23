//
// Created by rn7s2 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_ARGS_H
#define BUPT_SCS_DNS_RELAY_ARGS_H

#include "cargs.h"

/**
 * dns-relay 所有可用的命令行选项
 */
static struct cag_option options[] = {
        {.identifier = 'd',
                .access_letters = "d",
                .access_name = "debug",
                .value_name = NULL,
                .description = "调试级别 1 (仅输出时间坐标，序号，查询的域名)"},

        {.identifier = 'm',
                .access_letters = "v",
                .access_name = "verbose",
                .value_name = NULL,
                .description = "调试级别 2 (输出冗长的调试信息)"},

        {.identifier = 's',
                .access_letters = "s",
                .access_name = "server",
                .value_name = "dns-server-ipaddr",
                .description = "使用指定的 DNS 服务器"},

        {.identifier = 'f',
                .access_letters = "f",
                .access_name = "filename",
                .value_name = "filename",
                .description = "使用指定的配置文件 (默认为 dnsrelay.txt)"},

        {.identifier = 'p',
                .access_letters = "p",
                .access_name="port",
                .value_name="port",
                .description="使用指定的端口号 (默认为53)"},

        {
                .identifier = 'h',
                .access_letters = "h",
                .access_name = "help",
                .description = "显示本帮助信息，然后退出"}
};

/**
 * dns-relay 的命令行参数信息
 */
struct Config {
    int debug_level, port;
    const char *dns_server_ipaddr;
    const char *filename;
};

/**
 * 解析命令行参数
 * @param argc 命令行参数个数
 * @param argv 命令行参数
 * @param config 解析结果的指针
 * @return 解析成功或失败，成功为 0，失败为 -1
 */
int parse_args(int argc, char **argv, struct Config *config);

/**
 * 打印命令行参数
 * @param config 命令行参数信息
 */
void dump_args(struct Config *config);

#endif //BUPT_SCS_DNS_RELAY_ARGS_H
