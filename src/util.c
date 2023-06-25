//
// Created by rn7s2 on 2023/6/22.
//

#include "util.h"

#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <sys/unistd.h>

void str_tolower(char *str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        if (isupper(str[i])) {
            str[i] = tolower(str[i]);
        }
    }
}

void format_time(char *output)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(output, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900,
            timeinfo->tm_mon + 1, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

/**
 * 获取 CPU 核心数
 * @return CPU 核心数
 */
int get_cpu_num()
{
    return (int) sysconf(_SC_NPROCESSORS_ONLN);
}

void swap_ptr(void *a, void *b)
{
    void *tmp = a;
    a = b;
    b = tmp;
}
