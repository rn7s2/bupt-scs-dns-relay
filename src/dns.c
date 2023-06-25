//
// Created by rn7s2 on 2023/6/23.
//

#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include "args.h"
#include "dns.h"
#include "filerules.h"
#include "cache.h"
#include "logger.h"

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
    // TODO 请在这里实现 DNS 请求的处理
    // 请求的数据存放在 buf 缓冲区
    struct DnsHeader *header = (struct DnsHeader *) args->buf;
    // 网络字节序转为主机字节序
    dns_header_ntohs(header);
    if (server_config.debug_level >= 2) {
        dns_header_dump(header);
    }

    if (header->qr == 0) {
        // 如果是查询请求
        struct DnsQuestion *questions = (struct DnsQuestion *) malloc(sizeof(struct DnsQuestion) * header->qdcount);
        // 解析里面的问题，并返回偏移量
        int offset = dns_parse_questions(args->buf, questions);
        for (int i = 0; i < header->qdcount; i++) {
            // dns_question_dump(&questions[i]);
        }

        // 分配DNS响应缓冲区,并复制原始请求
        char reply_buf[MAX_DNSBUF_LEN] = {0};
        memcpy(reply_buf, args->buf, offset);
        ((struct DnsHeader *) reply_buf)->qr = 1; // 将回答标记置为 1
        // 对每个问题记录进行域名解析,获取响应
        for (int i = 0; i < header->qdcount; i++) {
            struct DnsAnswer *reply = dns_resolve(&questions[i]);
            // dns_reply_dump(&reply);
            offset = dns_answer_to_resource_record(reply, reply_buf + offset);
            free(reply);
            // 若长度大于512则丢弃
            if (offset > 512) {
                free(args);
                return;
            }
        }

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
    debug("id: %d\n", header->id);
    debug("qr: %d\n", header->qr);
    debug("opcode: %d\n", header->opcode);
    debug("aa: %d\n", header->aa);
    debug("tc: %d\n", header->tc);
    debug("rd: %d\n", header->rd);
    debug("ra: %d\n", header->ra);
    debug("z: %d\n", header->z);
    debug("ad: %d\n", header->ad);
    debug("cd: %d\n", header->cd);
    debug("rcode: %d\n", header->rcode);
    debug("qdcount: %d\n", header->qdcount);
    debug("ancount: %d\n", header->ancount);
    debug("nscount: %d\n", header->nscount);
    debug("arcount: %d\n", header->arcount);
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

int dns_parse_qname(const char *buf, int offset, char *qname)
{
    int i = 0;
    while (buf[offset] != 0) {
        int len = (unsigned char) buf[offset];
        for (int j = 0; j < len; j++) {
            qname[i++] = buf[offset + j + 1];
        }
        qname[i++] = '.';
        offset += len + 1;
    }
    qname[i - 1] = '\0';
    return offset + 1;
}

void dns_question_dump(struct DnsQuestion *question)
{
    debug("qname: %s\n", question->qname);
    debug("qtype: %d\n", question->qtype);
    debug("qclass: %d\n", question->qclass);
}

struct DnsAnswer *dns_query(struct DnsQuestion *question)
{
    char packet[MAX_DNSBUF_LEN];
    uint16_t qid = dns_query_buf_new(question, packet);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53);
    inet_pton(AF_INET, server_config.dns_server_ipaddr, &server_addr.sin_addr.s_addr);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(sockfd, packet, sizeof packet, MSG_WAITALL,
           (struct sockaddr *) &server_addr, sizeof server_addr);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // TODO 改为计算出的 RTO (重传超时时间)
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    try_recv:
    if (recvfrom(sockfd, packet, sizeof packet, MSG_WAITALL, NULL, NULL) < 0) {
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

    // 解析 DNS 响应包
    size_t len = sizeof(struct DnsAnswer) + header->ancount * sizeof(struct DnsResource);
    struct DnsAnswer *answer = malloc(len);
    memset(answer, 0, len);
    int offset = sizeof(struct DnsHeader);
    strcpy(answer->qname, question->qname);
    answer->cached_time = time(NULL);
    for (int i = 0; i < header->ancount; i++) {
        offset = dns_parse_qname(packet, offset, answer->resources[i].name);
        answer->resources[i].type = ntohs(*(uint16_t *) (packet + offset));
        offset += sizeof(uint16_t);
        answer->resources[i].class = ntohs(*(uint16_t *) (packet + offset));
        offset += sizeof(uint16_t);
        answer->resources[i].ttl = ntohl(*(uint32_t *) (packet + offset));
        offset += sizeof(uint32_t);
        answer->resources[i].rdlength = ntohs(*(uint16_t *) (packet + offset));
        offset += sizeof(uint16_t);
        switch (answer->resources[i].type) {
            case A:
                inet_ntop(AF_INET, packet + offset, (char *) &(answer->resources[i].rdata.A.addr), INET_ADDRSTRLEN);
                offset += sizeof(uint32_t);
                break;
            case AAAA:
                inet_ntop(AF_INET6, packet + offset, (char *) &(answer->resources[i].rdata.AAAA.addr),
                          INET6_ADDRSTRLEN);
                offset += sizeof(uint32_t) * 4;
                break;
            case CNAME:
                offset += dns_parse_qname(packet, offset, answer->resources[i].rdata.CNAME.cname);
                break;
            case MX:
                answer->resources[i].rdata.MX.preference = ntohs(*(uint16_t *) (packet + offset));
                offset += sizeof(uint16_t);
                offset += dns_parse_qname(packet, offset, answer->resources[i].rdata.MX.mxname);
                break;
            default:
                error("未知的 DNS 类型: %d", answer->resources[i].type);
                return NULL;
        }
        answer->size++;
    }
    return answer;
}

uint16_t dns_query_buf_new(struct DnsQuestion *question, char *buf)
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
    buf[offset] = '\0';

    return qid;
}

int dns_to_qname(const char *name, char *buf)
{
    int len = (int) strlen(name);
    int i = 0, j = 0;
    while (i < len) {
        if (name[i] == '.') {
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
    // 1. 访问 DNS 规则文件，如果规则中存在问题的回答，直接返回
    struct DnsAnswer *answer = malloc(sizeof(struct DnsAnswer) + sizeof(struct DnsResource));
    if (match_filerules(question, (struct DnsResource *) (answer + sizeof(struct DnsAnswer)))) {
        answer->size = 1;
        strcpy(answer->qname, question->qname);
        answer->cached_time = 0;
        return answer;
    } else {
        free(answer);
    }

    // 2. 访问 Cache, 如果 Cache 中存在问题的回答并且未过期，直接返回
    if ((answer = match_cacherules(question))) {
        return answer;
    }

    // 3. 如果缓存未命中，向 DNS 服务器发送查询请求，获取响应
    if ((answer = dns_query(question))) {
        // 将响应写入 Cache
        // 复制一份 answer, 因为返回的 answer 会被释放
        size_t len = sizeof(struct DnsAnswer) + answer->size * sizeof(struct DnsResource);
        struct DnsAnswer *copy = malloc(len);
        memcpy(copy, answer, len);
        insert_cache(question, copy);
        return answer;
    }

    // 4. 都不成功，那就返回失败
    return NULL;
}

//void dns_answer_dump(struct DnsAnswer *reply)
//{
//    // TODO
//}

int dns_answer_to_resource_record(struct DnsAnswer *reply, char *buf)
{
    int offset = 0;
    for (int i = 0; i < reply->size; i++) {
        offset += dns_to_qname(reply->qname, buf + offset);
        *(uint16_t *) (buf + offset) = htons(reply->resources[i].type);
        offset += sizeof(uint16_t);
        *(uint16_t *) (buf + offset) = htons(reply->resources[i].class);
        offset += sizeof(uint16_t);
        *(uint32_t *) (buf + offset) = htonl(reply->resources[i].ttl);
        offset += sizeof(uint32_t);
        *(uint16_t *) (buf + offset) = htons(reply->resources[i].rdlength);
        offset += sizeof(uint16_t);
        switch (reply->resources[i].type) {
            case A:
                *(uint32_t *) (buf + offset) = *(uint32_t *) &(reply->resources[i].rdata.A.addr);
                offset += sizeof(uint32_t);
                break;
            case AAAA:
                *(uint32_t *) (buf + offset) = *(uint32_t *) &(reply->resources[i].rdata.AAAA.addr);
                offset += sizeof(uint32_t) * 4;
                break;
            case CNAME:
                offset += dns_to_qname(reply->resources[i].rdata.CNAME.cname, buf + offset);
                break;
            case MX:
                *(uint16_t *) (buf + offset) = htons(reply->resources[i].rdata.MX.preference);
                offset += sizeof(uint16_t);
                offset += dns_to_qname(reply->resources[i].rdata.MX.mxname, buf + offset);
                break;
            default:
                fatal("未知的 DNS 类型: %d", reply->resources[i].type);
                exit(-1);
        }
    }
    return offset;
}
