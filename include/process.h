//
// Created by rn7s2 on 2023/6/22.
//

#include <sys/types.h>

#ifndef BUPT_SCS_DNS_RELAY_PROCESS_H
#define BUPT_SCS_DNS_RELAY_PROCESS_H

/**
 * fork 一个子进程执行 func, 并在父进程中立即返回
 * @param func 子进程的操作
 * @return 子进程的 pid, -1 为 fork 失败
 */
pid_t fork_and_detach(void (*func)());

#endif //BUPT_SCS_DNS_RELAY_PROCESS_H
