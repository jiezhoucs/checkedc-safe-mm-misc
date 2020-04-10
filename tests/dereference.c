/*
 * More tests on pointer dereference.
 * */

#include "header.h"

//
// Testing dereferencing an MM_ptr inside a struct.
void f0() {
    print_start("dereferencing & free an MM_Ptr inside a struct");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    printf("Testing dereferencing an MM_ptr inside a struct.\n");
    mmsafe_ptr<Node> p = mmsafe_alloc<Node>(sizeof(Node));
    p->next = mmsafe_alloc<Node>(sizeof(Node));

    p->next->val = 10;
    p->next->l = 42;
    p->next->c = 'a';

    if (p->next->val != 10 || p->next->l != 42 || p->next->c != 'a') {
        perror("ERROR: Dereferencing testing failed in function f1!\n");
    }

    printf("Testing freeing an MM_ptr inside a struct.\n");
    mmsafe_free<Node>(p->next);

    p->next->val = 10;    // should raise an illegal instruction.

resume:
    print_end("dereferencing & free an MM_Ptr inside a struct");
}


//
// Testing dereferencing an MM_ptr inside an array.
void f1() {
    print_start("dereferencing an MM_ptr inside an array");
    if (setjmp(resume_context) == 1) goto resume;

    mmsafe_ptr<Data> p_arr[5] = { NULL };

    p_arr[1] = mmsafe_alloc<Data>(sizeof(Data));
    p_arr[1]->i = 10;
    p_arr[1]->d = 4.2;

    mmsafe_ptr<Data> p1 = p_arr[1];

    if (p1->i != 10 || p_arr[1]->d != 4.2) {
        perror("ERROR: Testing dereferencing an MM_ptr inside an array failed\
                in function f1().\n");
    }

    p_arr[1] = p_arr[0];
    int i = p_arr[1]->i;  // should raise a segfault

resume:
    print_end("dereferencing an MM_ptr inside an array");
}

int main() {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    f0();

    f1();

    print_main_end(__FILE__);
    return 0;
}
