/**
 * Test assginment-related functionalities for MMSafe_ptr.
 * */

#include "inc.h"

//
// Testing assigning an MMSafe_ptr to another.
void f0() {
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<Data> p0 = mm_alloc<Data>(sizeof(Data));
    p0->i = 1;
    p0->l = 100;
    p0->d = 4.2;

    mm_ptr<Data> p1 = p0;

    // Checked if the new pointer points to the correct memory object.
    if (p1->i != 1 || p1->l != 100 || p1->d != 4.2 || p1->f != p0->f) {
        perror("Assignment testing failed in function f0!\n");
    }

    mm_free<Data>(p1);

    int i = p0->i;  // should raise an Illegal Instruction error.

resume:
    printf("Testing assignment between MMSafe_ptr succeeded.\n\n");
}

//
// Test assigning NULL to an MMSafe_ptr.
void f1() {
    if (setjmp(resume_context) == 1) goto resume0;

    mm_ptr<Data> p0 = NULL;
    p0->i = 10;  // should raise a segfault

resume0:
    printf("Finished testing assigning NULL to an MM_ptr.\n");

    if (setjmp(resume_context) == 1) goto resume1;

    mm_ptr<Data> p_arr[2] = { NULL };
    p_arr[1]->i = 10;  // segmentation fault

resume1:
    printf("Finished testing assigning NULL to an MM_ptr in an array.\n\n");
}


int main(int argc, char *argv[]) {
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    printf("===== Begin testing assignment functionality. =====\n");

    f0();

    f1();


    printf("===== Finish testing assignment functionality. =====\n\n");
    return 0;
}
