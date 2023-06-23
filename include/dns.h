//
// Created by rn7s2 on 2023/6/23.
//

#ifndef BUPT_SCS_DNS_RELAY_DNS_H
#define BUPT_SCS_DNS_RELAY_DNS_H

#include <stdint.h>

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

#endif //BUPT_SCS_DNS_RELAY_DNS_H
