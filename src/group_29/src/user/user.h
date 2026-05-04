#pragma once

#include "libc/stdint.h"
#include "../memory/heap.h"
#include "../string/string.h"

#define USER_MAX_USERNAME_LENGTH 24

typedef struct {
    char username[USER_MAX_USERNAME_LENGTH + 1];
} UserData;

void init_username(void);
int8_t user_set_username(const char username[]);
const char* user_get_username();
