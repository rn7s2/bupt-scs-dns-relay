//
// Created by rn7s2 on 2023/6/22.
//

#ifndef BUPT_SCS_DNS_RELAY_UTIL_H
#define BUPT_SCS_DNS_RELAY_UTIL_H

/**
 * 将字符串转换为小写
 * @param str 字符串
 */
void str_tolower(char *str);

/**
 * 将字符串转换为大写
 * @param str 字符串
 */
void str_toupper(char *str);

/**
 * 格式化时间
 * @param output 输出缓冲区
 */
void format_time(char *output);

/**
 * 获取 CPU 核心数
 * @return CPU 核心数
 */
int get_cpu_num();

/**
 * 交换指针
 * @param a
 * @param b
 */
void swap_ptr(void *a, void *b);

#endif //BUPT_SCS_DNS_RELAY_UTIL_H
