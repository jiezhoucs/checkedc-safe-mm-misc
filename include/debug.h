/**
 * This header file defines a few structs used for testing.
 * */

#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <stdint.h>

#include <signal.h>
#include <setjmp.h>

#include "safe_mm_checked.h"

typedef struct data {
    int i;
    int j;
    long l;
    unsigned long ul;
    float f;
    double d;
} Data;

typedef struct node{
    int val;
    long l;
    mm_ptr<struct node> next;
    char c;
} Node;

void segv_handler(int sig);

void ill_handler(int sig);

void print_start(char *feature);
void print_end(char *feature);
void print_main_start(char *filename);
void print_main_end(char *filename);

#endif
