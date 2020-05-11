/**
 * This file is used for debugging purpose.
 * */

#include "debug.h"


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

// Print error message in RED color.
void print_error(char *err) {
    printf("%sERROR: %s.%s\n", RED, err, COLOR_RESET);
    exit(-1);
}
