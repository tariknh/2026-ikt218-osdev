#ifndef PAGING_H
#define PAGING_H

#include <libc/stdbool.h>

// Turn on paging with a simple identity map
void init_paging(void);
// Check that paging is active
bool paging_self_test(void);

#endif
