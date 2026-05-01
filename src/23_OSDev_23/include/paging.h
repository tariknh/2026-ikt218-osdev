#ifndef PAGING_H
#define PAGING_H

// Set up identity-mapped paging for the first 4MB and enable paging via CR0
void init_paging();

#endif /* PAGING_H */
