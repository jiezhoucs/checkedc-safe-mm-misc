/**
 * This header file defines a few structs used for testing.
 * */

#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>

#include <signal.h>
#include <setjmp.h>

#include "safe_mm_checked.h"

jmp_buf resume_context;  // to resume execution after seg fault or illegal instruction

typedef struct data {
    uint64_t ID;
    int i;
    int j;
    long l;
    unsigned long ul;
    float f;
    double d;
} Data;

typedef struct node{
    uint64_t ID;
    int val;
    long l;
    mmsafe_ptr<struct node> next;
    char c;
} Node;


//
// This signal handler resumes execution after a segmentation fault.
void segv_handler(int sig) {
    printf("Segmantation fault due to dereferencing a NULL pointer.\n");
    longjmp(resume_context, 1);
}

//
// This illegal instruction signal handler resumes execution
// after a Use-After-Free is detected.
void ill_handler(int sig) {
    printf("A UAF bug was detected.\n");
    longjmp(resume_context, 1);
}

//
// Printing helper functions.
void print_start(char *feature) {
  printf("------- Beging testing %s.-------\n", feature);
}

void print_end(char *feature) {
  printf("------- Finished testing %s.-------\n\n", feature);
}

void print_main_start(char *filename) {
  printf("========Begin testing %s.========\n", filename);
}

void print_main_end(char *filename) {
  printf("========Finished testing %s.========\n\n", filename);
}

#endif
