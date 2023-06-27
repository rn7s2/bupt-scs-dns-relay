//
// Created by rn7s2 on 2023/6/23.
//

#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include "args.h"
#include "dns.h"
#include "filerules.h"
#include "cache.h"
#include "logger.h"
#include "util.h"

extern struct Config server_config;

static uint16_t id = 0;

static pthread_mutex_t id_mutex;

void init_id()
{
    pthread_mutex_init(&id_mutex, NULL);
}

void free_id()
{
    pthread_mutex_destroy(&id_mutex);
}

void handle_dns_request(struct RequestArgs *args, void *user_data)
{
    // 请求的数据存放在 buf 缓冲区
    struct DnsHeader *header = (struct DnsHeader *) args->buf;
    // 网络字节序转为主机字节序
    dns_header_ntohs(header);
    dns_header_dump(header);

    if (header->qr == 0) {
        // 如果是查询请求
        struct DnsQuestion *questions = (struct DnsQuestion *) malloc(sizeof(struct DnsQuestion) * header->qdcount);
        // 解析里面的问题，并返回偏移量
        int offset = dns_parse_questions(args->buf, questions);
        for (int i = 0; i < header->qdcount; i++) {
            str_toupper(questions[i].qname);
            dns_question_dump(header, questions + i);
        }

        // 分配DNS响应缓冲区,并复制原始请求
        char reply_buf[MAX_DNSBUF_LEN] = {0};
        memcpy(reply_buf, args->buf, offset);

        struct DnsHeader *reply_header = (struct DnsHeader *) reply_buf;
        reply_header->qr = 1; // 将回答标记置为 1

        // 对每个问题记录进行域名解析,获取响应
        GList *answers = NULL;
        for (int i = 0; i < reply_header->qdcount; i++) {
            struct DnsAnswer *reply = dns_resolve(&questions[i]);

            if (reply == NULL) { // 处理解析失败的情况
                reply_header->rcode = 3;
                dns_header_htons(reply_header);
                socklen_t len;
                if (args->client_addr.ss_family == AF_INET) {
                    len = sizeof(struct sockaddr_in);
                } else {
                    len = sizeof(struct sockaddr_in6);
                }
                sendto(args->sockfd, reply_buf, offset, 0, (struct sockaddr *) &args->client_addr, len);
                free(questions);
                free(args->buf);
                free(args);
                return;
            }

            answers = g_list_append(answers, reply);
            reply_header->ancount += reply->answer_rr;
            reply_header->nscount += reply->authority_rr;
        }

        // 填写 Answer RR 部分
        GList *head = answers;
        while (head != NULL) {
            struct DnsAnswer *reply = head->data;
            if (reply->answer_rr > 0) {
                memcpy(reply_buf + offset, reply->answer_buf, reply->answer_buf_len);
                offset += reply->answer_buf_len;
            }
            if (offset > 512) {
                g_list_free_full(answers, free);
                free(args->buf);
                free(args);
                return;
            }
            head = g_list_next(head);
        }

        // 填写 Authority RR 部分
        head = answers;
        while (head != NULL) {
            struct DnsAnswer *reply = head->data;
            if (reply->authority_rr > 0) {
                memcpy(reply_buf + offset, reply->authority_buf, reply->authority_buf_len);
                offset += reply->authority_buf_len;
            }
            if (offset > 512) {
                g_list_free_full(answers, free);
                free(args->buf);
                free(args);
                return;
            }
            head = g_list_next(head);
        }
        g_list_free_full(answers, free);

        // 将响应头部转换为网络字节序
        dns_header_htons(reply_header);

        // 将响应sendto给客户端
        socklen_t len;
        if (args->client_addr.ss_family == AF_INET) {
            len = sizeof(struct sockaddr_in);
        } else {
            len = sizeof(struct sockaddr_in6);
        }
        sendto(args->sockfd, reply_buf, offset, 0, (struct sockaddr *) &args->client_addr, len);
        free(questions);
    }

    free(args->buf);
    free(args);
}

void dns_header_ntohs(struct DnsHeader *header)
{
    header->id = ntohs(header->id);
    header->qdcount = ntohs(header->qdcount);
    header->ancount = ntohs(header->ancount);
    header->nscount = ntohs(header->nscount);
    header->arcount = ntohs(header->arcount);
}

void dns_header_htons(struct DnsHeader *header)
{
    header->id = htons(header->id);
    header->qdcount = htons(header->qdcount);
    header->ancount = htons(header->ancount);
    header->nscount = htons(header->nscount);
    header->arcount = htons(header->arcount);
}

void dns_header_dump(struct DnsHeader *header)
{
    if (server_config.debug_level >= 2) {
        debug("QRYDNS, id = %d, qdcount = %d", header->id, header->qdcount);
    }
}

int dns_parse_questions(char *buf, struct DnsQuestion questions[])
{
    int offset = sizeof(struct DnsHeader);
    // 获取 DNS 头部中的 qdcount 字段,表示问题的数量
    int qdcount = ((struct DnsHeader *) buf)->qdcount;
    for (int i = 0; i < qdcount; i++) {
        memset(&questions[i], 0, sizeof(struct DnsQuestion));
        // 调用 dns_parse_qname() 解析问题名,并更新 offset
        offset = dns_parse_qname(buf, offset, questions[i].qname);
        // 读取问题类型(qtype),并转换为主机字节序,存储在 questions[i].qtype 中。更新 offset
        questions[i].qtype = ntohs(*(uint16_t *) (buf + offset));
        offset += sizeof(uint16_t);
        // 读取问题类(qclass),并转换为主机字节序,存储在 questions[i].qclass 中。更新 offset
        questions[i].qclass = ntohs(*(uint16_t *) (buf + offset));
        // 解析完成，返回偏移量
        offset += sizeof(uint16_t);
    }
    return offset;
}

int dns_qname_compressed(char count_char)
{
    // 高两位为 1 则为压缩的域名
    return (count_char & 0xc0) == 0xc0;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

int dns_parse_qname(const char *buf, int offset, char *qname)
{
    int i = 0;
    while (buf[offset] != 0) {
        if (dns_qname_compressed(buf[offset])) {
            // 取后 14 位，即相对 buf 的偏移量
            int new_offset = ntohs(*(uint16_t *) (buf + offset)) & 0x3fff;
            // 递归解析
            dns_parse_qname(buf, new_offset, qname + i);
            return offset + 2;
        } else {
            int len = (unsigned char) buf[offset];
            for (int j = 0; j < len; j++) {
                qname[i++] = buf[offset + j + 1];
            }
            qname[i++] = '.';
            offset += len + 1;
        }
    }
    qname[i - 1] = '\0';
    return offset + 1;
}

#pragma clang diagnostic pop

void dns_question_dump(struct DnsHeader *header, struct DnsQuestion *question)
{
    if (server_config.debug_level >= 2) {
        debug("QUERY, id = %d, qname = %s, qtype = %x, qclass = %x",
              header->id, question->qname, question->qtype, question->qclass);
    } else if (server_config.debug_level >= 1) {
        debug("QUERY, id = %d, qname = %s", header->id, question->qname);
    }
}

struct DnsAnswer *dns_query(struct DnsQuestion *question)
{
    int packet_len;
    char packet[MAX_DNSBUF_LEN];
    uint16_t qid = dns_query_buf_new(question, packet, &packet_len);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53);
    inet_pton(AF_INET, server_config.dns_server_ipaddr, &server_addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(sockfd, packet, packet_len, MSG_WAITALL,
           (struct sockaddr *) &server_addr, sizeof server_addr);

    // 设置请求上级 DNS 的超时时间
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * server_config.rto;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int n;
    try_recv:
    if ((n = (int) recvfrom(sockfd, packet, sizeof packet, 0, NULL, NULL)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) { // 接收超时
            if (server_config.debug_level >= 2) {
                warning("TIMEOUT: server query id = %d", qid);
            }
            return 0;
        }
    }

    struct DnsHeader *header = (struct DnsHeader *) packet;
    dns_header_ntohs(header);
    if (header->id != qid || header->qr != 1) {
        goto try_recv; // 丢弃不属于这次 DNS 请求的响应包
    }

    // 保存 DNS 响应包
    struct DnsAnswer *answer = malloc(sizeof(struct DnsAnswer));
    strcpy(answer->qname, question->qname);
    answer->answer_rr = header->ancount;
    answer->authority_rr = header->nscount;
    answer->cached_time = time(NULL);

    // 解析 DNS 响应包中各个 RR，计算本条查询结果的 TTL
    int offset = packet_len;
    uint32_t min_ttl = 0x7fffffff; // 答案的最小 TTL 作为本条查询结果的 TTL
    char tmp[MAX_DOMAIN_LEN]; // 临时 buf, 用于存储解析到的各种 name

    // 解析 DNS 响应包中的 Answer RR
    for (int i = 0; i < header->ancount; i++) {
        offset = dns_parse_qname(packet, offset, tmp);
        uint16_t type = ntohs(*(uint16_t *) (packet + offset));
        offset += sizeof(uint16_t);
        offset += sizeof(uint16_t);
        uint32_t ttl = ntohl(*(uint32_t *) (packet + offset));
        min_ttl = (min_ttl < ttl ? min_ttl : ttl);
        offset += sizeof(uint32_t);
        offset += sizeof(uint16_t);
        switch (type) {
            case A:
                offset += sizeof(uint32_t);
                break;
            case AAAA:
                offset += sizeof(uint8_t) * 16;
                break;
            case CNAME:
                offset = dns_parse_qname(packet, offset, tmp);
                break;
            case MX:
                offset += sizeof(uint16_t);
                offset = dns_parse_qname(packet, offset, tmp);
                break;
            default:
                error("未知的 DNS 类型: %d", type);
                free(answer);
                return NULL;
        }
    }
    if (offset > packet_len) {
        answer->answer_buf_len = offset - packet_len;
        memcpy(answer->answer_buf, packet + packet_len, answer->answer_buf_len);
    }

    // 解析 DNS 响应包中的 Authority RR
    if (n > offset) {
        answer->authority_buf_len = n - offset;
        memcpy(answer->authority_buf, packet + offset, answer->authority_buf_len);
    }
    for (int i = 0; i < header->nscount; i++) {
        offset = dns_parse_qname(packet, offset, tmp);
        offset += sizeof(uint16_t);
        offset += sizeof(uint16_t);
        uint32_t ttl = ntohl(*(uint32_t *) (packet + offset));
        min_ttl = (min_ttl < ttl ? min_ttl : ttl);
        offset += sizeof(uint32_t);
        offset += sizeof(uint16_t);

        // 跳过 Authority RR 的 RDATA 部分
        // Primary name server
        offset = dns_parse_qname(packet, offset, tmp);
        // Responsible authority's mailbox
        offset = dns_parse_qname(packet, offset, tmp);
        // Serial number
        offset += sizeof(uint32_t);
        // Refresh interval
        offset += sizeof(uint32_t);
        // Retry interval
        offset += sizeof(uint32_t);
        // Expiration limit
        offset += sizeof(uint32_t);
        // Minimum TTL
        offset += sizeof(uint32_t);
    }

    answer->ttl = (int) min_ttl;
    return answer;
}

uint16_t dns_query_buf_new(struct DnsQuestion *question, char *buf, int *packet_len)
{
    pthread_mutex_lock(&id_mutex);
    uint16_t qid = id++;
    pthread_mutex_unlock(&id_mutex);

    struct DnsHeader header = {
            .id = qid,
            .rd = 1,
            .tc = 0,
            .aa = 0,
            .opcode = 0,
            .qr = 0,
            .rcode = 0,
            .cd = 0,
            .ad = 0,
            .z = 0,
            .ra = 0,
            .qdcount = 1,
            .ancount = 0,
            .nscount = 0,
            .arcount = 0
    };
    dns_header_htons(&header);

    int offset = sizeof header;
    memcpy(buf, &header, offset);
    offset += dns_to_qname(question->qname, buf + offset);
    *(uint16_t *) (buf + offset) = htons(question->qtype);
    offset += sizeof(uint16_t);
    *(uint16_t *) (buf + offset) = htons(question->qclass);
    offset += sizeof(uint16_t);
    *packet_len = offset;

    return qid;
}

int dns_to_qname(const char *name, char *buf)
{
    int len = (int) strlen(name);
    int i = 0, j = 0;
    while (i <= len) {
        if (i == len) {
            buf[j] = (char) (i - j);
            break;
        } else if (name[i] == '.') {
            buf[j] = (char) (i - j);
            j = i + 1;
        } else {
            buf[i + 1] = name[i];
        }
        i++;
    }
    buf[i + 1] = '\0';
    return i + 2;
}

struct DnsAnswer *dns_resolve(struct DnsQuestion *question)
{
    if (question->qtype != A && question->qtype != AAAA
        && question->qtype != CNAME && question->qtype != MX) {
        if (server_config.debug_level >= 2) {
            warning("不支持的 DNS 查询类型: %d", question->qtype);
        }
        return NULL;
    }

    // 1. 访问 DNS 规则文件，如果规则中存在问题的回答，直接返回
    struct DnsResource *resource = malloc(sizeof(struct DnsResource));
    if (match_filerules(question, resource)) {
        if (server_config.debug_level >= 2) {
            debug("FILERULES MATCH: %s", resource->name);
        }

        if (resource->rdata.A.addr == 0) { // 不良网站拦截
            if (server_config.debug_level >= 2) {
                debug("BLOCKED: %s", resource->name);
            }
            free(resource);
            return NULL;
        }

        // 根据 resource 构建 answer
        struct DnsAnswer *answer = malloc(sizeof(struct DnsAnswer));
        answer->answer_rr = 1, answer->authority_rr = 0;
        answer->ttl = (int) resource->ttl;
        strcpy(answer->qname, resource->name);
        answer->cached_time = time(NULL);
        answer->answer_buf_len = dns_resource_to_buf(resource, answer->answer_buf);
        free(resource);
        return answer;
    } else {
        free(resource);
    }

    // 2. 访问 Cache, 如果 Cache 中存在问题的回答并且未过期，直接返回
    struct DnsAnswer *answer;
    if ((answer = match_cacherules(question))) {
        if (server_config.debug_level >= 2) {
            debug("CACHE MATCH: %s", question->qname);
        }

        // Cache 返回的即副本，不需要复制
        return answer;
    }

    // 3. 如果缓存未命中，向 DNS 服务器发送查询请求，获取响应
    if ((answer = dns_query(question))) {
        if (server_config.debug_level >= 2) {
            debug("RELAY DNS QUERY: %s", question->qname);
        }

        // 将响应写入 Cache
        // 复制一份 answer, 因为返回的 answer 会被释放
        struct DnsAnswer *copy = malloc(sizeof(struct DnsAnswer));
        memcpy(copy, answer, sizeof(struct DnsAnswer));
        insert_cache(question, copy);
        return answer;
    }

    // 4. 都不成功，那就返回失败
    return NULL;
}

int dns_resource_to_buf(struct DnsResource *resource, char *buf)
{
    int offset = 0;
    offset += dns_to_qname(resource->name, buf + offset);
    *(uint16_t *) (buf + offset) = htons(resource->type);
    offset += sizeof(uint16_t);
    *(uint16_t *) (buf + offset) = htons(resource->class);
    offset += sizeof(uint16_t);
    *(uint32_t *) (buf + offset) = htonl(resource->ttl);
    offset += sizeof(uint32_t);
    *(uint16_t *) (buf + offset) = htons(resource->rdlength);
    offset += sizeof(uint16_t);
    switch (resource->type) {
        case A:
            *(uint32_t *) (buf + offset) = resource->rdata.A.addr;
            offset += sizeof(uint32_t);
            break;
        case AAAA:
            memcpy(buf + offset, resource->rdata.AAAA.addr, sizeof(uint8_t) * 16);
            offset += sizeof(uint8_t) * 16;
            break;
        case CNAME:
            offset += dns_to_qname(resource->rdata.CNAME.cname, buf + offset);
            break;
        case MX:
            *(uint16_t *) (buf + offset) = htons(resource->rdata.MX.preference);
            offset += sizeof(uint16_t);
            offset += dns_to_qname(resource->rdata.MX.mxname, buf + offset);
            break;
        default:
            fatal("未知的 DNS 类型: %d", resource->type);
            exit(-1);
    }
    return offset;
}
