//
// Created by rn7s2 on 2023/6/22.
//

#include "util.h"

#include <ctype.h>

void str_tolower(char *str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        if (isupper(str[i])) {
            str[i] = tolower(str[i]);
        }
    }
}
