/**
 * This header file defines a few structs used for testing.
 * */

#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <execinfo.h>

#include "safe_mm_checked.h"

// Print error message in red
#define COLOR_RESET   "\033[0m"
#define RED           "\033[31m"

#define CALL_STACK_DEPTH 1024

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

// to resume execution after seg fault or illegal instruction
jmp_buf resume_context;

void segv_handler(int sig);

void ill_handler(int sig);

void print_start(char *feature);
void print_end(char *feature);
void print_main_start(char *filename);
void print_main_end(char *filename);
void print_error(char *err);

// Dump the call stack
void print_callstack(void);

#endif
