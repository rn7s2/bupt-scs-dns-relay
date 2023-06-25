//
// Created by rn7s2 on 2023/6/23.
//

#include <malloc.h>
#include <string.h>
#include "dns.h"
#include "filerules.h"
#include "cache.h"

void handle_dns_request(struct RequestArgs *args, void *user_data)
{
    // TODO 请在这里实现 DNS 请求的处理
    /*请求的数据存放在 buf 缓冲区*/
    struct DnsHeader *header = (struct DnsHeader *) args->buf;
    /*网络字节序转为主机字节序*/
    dns_header_ntohs(header);
    // dns_header_dump(header);

    if (header->qr == 0) {
        /*如果是查询请求*/
        struct DnsQuestion *questions = (struct DnsQuestion *) malloc(sizeof(struct DnsQuestion) * header->qdcount);
        /*解析里面的问题，并返回偏移量*/
        int offset = dns_parse_questions(args->buf, questions);
        for (int i = 0; i < header->qdcount; i++) {
            // dns_question_dump(&questions[i]);
        }

        /*分配DNS响应缓冲区,并复制原始请求*/
        char reply_buf[MAX_DNSBUF_LEN] = {0};
        memcpy(reply_buf, args->buf, offset);
        // TODO set qr = 1
        /*对每个问题记录进行域名解析,获取响应*/
        for (int i = 0; i < header->qdcount; i++) {
            // struct DnsResource reply = dns_resolve(&questions[i]);
            // dns_reply_dump(&reply);
            // offset = dns_reply_to_resource_record(&reply, reply_buf + offset);
        }
        /*若长度大于512则丢弃*/
//        if (offset > 512)
//            continue;

        /*将响应sendto给客户端*/
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

//void dns_header_dump(struct DnsHeader *header)
//{
//    printf("id: %d\n", header->id);
//    printf("qr: %d\n", header->qr);
//    printf("opcode: %d\n", header->opcode);
//    printf("aa: %d\n", header->aa);
//    printf("tc: %d\n", header->tc);
//    printf("rd: %d\n", header->rd);
//    printf("ra: %d\n", header->ra);
//    printf("z: %d\n", header->z);
//    printf("ad: %d\n", header->ad);
//    printf("cd: %d\n", header->cd);
//    printf("rcode: %d\n", header->rcode);
//    printf("qdcount: %d\n", header->qdcount);
//    printf("ancount: %d\n", header->ancount);
//    printf("nscount: %d\n", header->nscount);
//    printf("arcount: %d\n", header->arcount);
//}

int dns_parse_questions(char *buf, struct DnsQuestion questions[])
{
    int offset = sizeof(struct DnsHeader);
    /*获取 DNS 头部中的 qdcount 字段,表示问题的数量*/
    int qdcount = ((struct DnsHeader *) buf)->qdcount;
    for (int i = 0; i < qdcount; i++) {
        memset(&questions[i], 0, sizeof(struct DnsQuestion));
        /*调用 dns_parse_qname() 解析问题名,并更新 offset*/
        offset = dns_parse_qname(buf, offset, questions[i].qname);
        /*读取问题类型(qtype),并转换为主机字节序,存储在 questions[i].qtype 中。更新 offset*/
        questions[i].qtype = ntohs(*(uint16_t *) (buf + offset));
        offset += sizeof(uint16_t);
        /*读取问题类(qclass),并转换为主机字节序,存储在 questions[i].qclass 中。更新 offset*/
        questions[i].qclass = ntohs(*(uint16_t *) (buf + offset));
        /*解析完成，返回偏移量*/
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

//void dns_question_dump(struct DnsQuestion *question)
//{
//    printf("qname: %s\n", question->qname);
//    printf("qtype: %d\n", question->qtype);
//    printf("qclass: %d\n", question->qclass);
//}

struct DnsResource dns_resolve(struct DnsQuestion *question)
{
    // 1. 访问 DNS 规则文件，如果规则中存在问题的回答，直接返回
    struct DnsResource resource = {0};
    if (match_filerules(question, &resource)) {
        return resource;
    }

    // 2. 访问 Cache, 如果 Cache 中存在问题的回答并且未过期，直接返回
    if (match_cacherules(question, &resource)) {
        return resource;
    }

    // 3. 如果缓存未命中，向 DNS 服务器发送查询请求，获取响应

    // 4. 如果上级 DNS 服务器超过 RTO (重传超时) 或失败，那么返回失败

    // 5. 如果成功响应，则插入 Cache
    // TODO: 应保证 resource 为 malloc 出的内存
}

//void dns_reply_dump(struct DnsResource *reply)
//{
//    // TODO
//}

int dns_reply_to_resource_record(struct DnsResource *reply, char *buf)
{
    // TODO
}
