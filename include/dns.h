//
// Created by rn7s2 on 2023/6/23.
//

#ifndef BUPT_SCS_DNS_RELAY_DNS_H
#define BUPT_SCS_DNS_RELAY_DNS_H

#include "handler.h"

#include <stdint.h>

#define MAX_DNSBUF_LEN (512 * 2)
#define MAX_DOMAIN_LEN 256

/**
 * DNS 报文头部
 */
struct DnsHeader {
    unsigned id: 16;        /* query identification number */

    // 注意，这里的位域排列根据小端序机器的字节序进行了调整
    unsigned rd: 1;         /* recursion desired */
    unsigned tc: 1;         /* truncated message */
    unsigned aa: 1;         /* authoritive answer */
    unsigned opcode: 4;     /* purpose of message */
    unsigned qr: 1;         /* response flag */

    // 注意，这里的位域排列根据小端序机器的字节序进行了调整
    unsigned rcode: 4;      /* response code */
    unsigned cd: 1;         /* checking disabled by resolver */
    unsigned ad: 1;         /* authentic data from named */
    unsigned z: 1;          /* unused bits, must be ZERO */
    unsigned ra: 1;         /* recursion available */

    uint16_t qdcount;       /* number of question entries */
    uint16_t ancount;       /* number of answer entries */
    uint16_t nscount;       /* number of authority entries */
    uint16_t arcount;       /* number of resource entries */
};

/**
 * DNS 一个询问
 */
struct DnsQuestion {
    char qname[MAX_DOMAIN_LEN];
    uint16_t qtype;
    uint16_t qclass;
};

enum DnsType {
    A = 1,
    AAAA = 28,
    CNAME = 5,
    MX = 15,
};

/**
 * DNS 资源记录
 */
struct DnsResource {
    char name[MAX_DOMAIN_LEN];
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;

    union {
        struct {
            uint32_t addr;
        } A; // IPv4 地址

        struct {
            uint8_t addr[16];
        } AAAA; // IPv6 地址

        struct {
            char cname[MAX_DOMAIN_LEN];
        } CNAME; // 别名

        struct {
            uint16_t preference;
            char mxname[MAX_DOMAIN_LEN];
        } MX; // 邮件交换};
    } rdata;
};

/**
 * DNS 回答
 */
struct DnsAnswer {
    int ttl;
    int size; // 包含的资源记录数
    int packet_len; // buf 的长度
    time_t cached_time;
    char qname[MAX_DOMAIN_LEN];
    char buf[MAX_DNSBUF_LEN];
};

void init_id();

void free_id();

/**
 * DNS 请求处理函数，在线程池的各个线程中调用，并发处理 DNS 请求
 * @param args 请求参数，包含整个 DNS 报文
 * @param user_data 无意义，可忽略
 */
void handle_dns_request(struct RequestArgs *args, void *user_data);

/**
 * 将 DNS 报文头部的各个字段从网络字节序转换为主机字节序
 * @param header
 */
void dns_header_ntohs(struct DnsHeader *header);

/**
 * 将 DNS 报文头部的各个字段从主机字节序转换为网络字节序
 * @param header
 */
void dns_header_htons(struct DnsHeader *header);

void dns_header_dump(struct DnsHeader *header);

/**
 * 解析 DNS 报文中的问题部分
 * @param buf DNS 报文
 * @param questions 问题数组，解析的结果会被保存在这里
 * @return 解析完问题部分后相对 buf 的偏移量
 */
int dns_parse_questions(char *buf, struct DnsQuestion questions[]);

/**
 * 判断 DNS 报文中的名称是否压缩
 * @param count_char 名称的第一个计数字节
 * @return 1 为压缩的，0 为非压缩的
 */
int dns_qname_compressed(char count_char);

/**
 * 解析 DNS 报文的名称格式
 * @param buf DNS 报文
 * @param offset 偏移量
 * @param qname 解析结果
 * @return 解析完该名称后相对 buf 的偏移量
 */
int dns_parse_qname(const char *buf, int offset, char *qname);

void dns_question_dump(struct DnsQuestion *question);

/**
 * 根据 question 联系上级 DNS 服务器查询
 * @param question
 * @return 失败为 NULL
 */
struct DnsAnswer *dns_query(struct DnsQuestion *question);

/**
 * 将 question 转换构造为 DNS 报文
 * @param question
 * @param buf 保存报文的 buffer
 * @param packet_len 保存的报文长度
 * @return 报文的 ID
 */
uint16_t dns_query_buf_new(struct DnsQuestion *question, char *buf, int *packet_len);

/**
 * 将域名转换为 DNS 报文中的名称格式
 * @param name
 * @param buf 保存名称的 buffer
 * @return 转换后的名称长度
 */
int dns_to_qname(const char *name, char *buf);

/**
 * 按照文件规则、Cache、上级 DNS 服务器的顺序解析 question，返回 answer
 * @param question
 * @return NULL 代表解析失败
 */
struct DnsAnswer *dns_resolve(struct DnsQuestion *question);

// void dns_answer_dump(struct DnsAnswer *reply);

/**
 * 将一条 DNS 资源记录转换为 DNS 报文，写入 buf 中
 * @param resource
 * @param buf
 * @return 写入后相对 buf 的偏移量
 */
int dns_resource_to_buf(struct DnsResource* resource, char *buf);

#endif //BUPT_SCS_DNS_RELAY_DNS_H
