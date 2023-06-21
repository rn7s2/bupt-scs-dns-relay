//
// Created by rn7s2 on 2023/6/22.
//

#include "process.h"

#include <stdlib.h>
#include <unistd.h>

pid_t fork_and_detach(void (*func)())
{
    pid_t pid = fork();
    if (pid < 0) { // fork 失败
        return -1;
    }
    if (pid == 0) { // 在子进程中
        (*func)();
        exit(0);
    }
    return pid;
}
