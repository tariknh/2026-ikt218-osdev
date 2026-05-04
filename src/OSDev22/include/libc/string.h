#pragma once

#include "stdint.h"
#include "stddef.h"

/* Returnerer lengden på en null-terminert streng */
size_t strlen(const char* str);

/* Fyller et minneområde med en bestemt verdi */
void* memset(void* dest, int val, size_t count);