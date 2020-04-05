/**
 * This header file defines a few structs used for testing.
 * */

#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include "safe_mm_checked.h"

typedef struct data {
    uint64_t ID;
    int i;
    int j;
    long l;
    unsigned long ul;
    float f;
    double d;
} Data;

typedef struct {
    uint64_t ID;
    int val;
    long long_val;
    char c;
} Node;

#endif
