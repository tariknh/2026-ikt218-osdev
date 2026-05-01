#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>


static uint32_t seed = 12345;

static inline uint32_t rand_next() {
    seed = seed * 1664525 + 1013904223;
    return seed;
}

static inline int rand_range(int min, int max) {
    return (rand_next() % ((max + 1) - min) + min);
}

#endif