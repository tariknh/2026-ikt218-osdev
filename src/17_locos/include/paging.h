#ifndef PAGING_H
#define PAGING_H

#include <libc/stdbool.h>

void init_paging(void);
bool paging_self_test(void);

#endif
