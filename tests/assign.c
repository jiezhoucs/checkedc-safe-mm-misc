/**
 * Test assginment-related functionalities for MMSafe_ptr.
 * */

#include "header.h"
#include <signal.h>
#include <setjmp.h>

jmp_buf resume_context;

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
// Testing assigning an MMSafe_ptr to another.
void f0() {
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mmsafe_ptr<Data> p0 = mmsafe_alloc<Data>(sizeof(Data));
    p0->i = 1;
    p0->l = 100;
    p0->d = 4.2;

    mmsafe_ptr<Data> p1 = p0;

    // Checked if the new pointer points to the correct memory object.
    if (p1->i != 1 || p1->l != 100 || p1->d != 4.2 || p1->f != p0->f) {
        perror("Assignment testing failed in function f0!\n");
    }

    mmsafe_free<Data>(p1);

    int i = p0->i;  // should raise an Illegal Instruction error.

resume:
    printf("Testing assignment between MMSafe_ptr succeeded.\n\n");
}


//
// Test assigning NULL to an MMSafe_ptr.
void f1() {
    signal(SIGSEGV, segv_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mmsafe_ptr<Data> p0 = NULL;
    p0->i = 10;  // should raise a segfault

resume:
    printf("Testing assigning NULL to an MMSafe_ptr succeeded.\n\n");
}


// Test assigning NULL to an array of MMSafe_ptr.
void f2() {
    mmsafe_ptr<Data> p_arr[2] = { NULL };

    // TODO: dereference one of the pointers.
}

int main(int argc, char *argv[]) {
    printf("===== Begin testing assignment functionality. =====\n");

    f0();

    f1();

    f2();

    printf("===== Finish testing assignment functionality. =====\n");
    return 0;
}
