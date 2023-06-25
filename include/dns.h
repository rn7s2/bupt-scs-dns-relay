//
// Created by rn7s2 on 2023/6/23.
//

#ifndef BUPT_SCS_DNS_RELAY_DNS_H
#define BUPT_SCS_DNS_RELAY_DNS_H

#include "handler.h"

#include <stdint.h>

#define MAX_DNSBUF_LEN 512
#define MAX_DOMAIN_LEN 256

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

struct DnsResource {
    char name[MAX_DOMAIN_LEN];
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    time_t recorded_time;

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

void handle_dns_request(struct RequestArgs *args, void *user_data);

void dns_header_ntohs(struct DnsHeader *header);

void dns_header_htons(struct DnsHeader *header);

// void dns_header_dump(struct DnsHeader *header);

int dns_parse_questions(char *buf, struct DnsQuestion questions[]);

int dns_parse_qname(const char *buf, int offset, char *qname);

// void dns_question_dump(struct DnsQuestion *question);

struct DnsResource dns_resolve(struct DnsQuestion *question);

// void dns_reply_dump(struct DnsResource *reply);

int dns_reply_to_resource_record(struct DnsResource *reply, char *buf);

#endif //BUPT_SCS_DNS_RELAY_DNS_H
