/*
 * More tests on pointer dereference.
 * */

#include "debug.h"

//
// Testing dereferencing an MM_ptr inside a struct.
void f0() {
    print_start("dereferencing & free an MM_Ptr inside a struct");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    printf("Testing dereferencing an MM_ptr inside a struct.\n");
    mm_ptr<Node> p = mm_alloc<Node>(sizeof(Node));
    p->next = mm_alloc<Node>(sizeof(Node));

    p->next->val = 10;
    p->next->l = 42;
    p->next->c = 'a';

    if (p->next->val != 10 || p->next->l != 42 || p->next->c != 'a') {
        print_error("Dereferencing testing failed in function f1");
    }

    printf("Testing freeing an MM_ptr inside a struct.\n");
    mm_free<Node>(p->next);

    p->next->val = 10;    // should raise an illegal instruction.

resume:
    print_end("dereferencing & free an MM_Ptr inside a struct");
}


//
// Testing dereferencing an MM_ptr inside an array.
void f1() {
    print_start("dereferencing an MM_ptr inside an array");
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<Data> p_arr[5] = { NULL };

    p_arr[1] = mm_alloc<Data>(sizeof(Data));
    p_arr[1]->i = 10;
    p_arr[1]->d = 4.2;

    mm_ptr<Data> p1 = p_arr[1];

    if (p1->i != 10 || p_arr[1]->d != 4.2) {
        perror("ERROR: Testing dereferencing an MM_ptr inside an array failed\
                in function f1().\n");
    }

    p_arr[1] = p_arr[0];
    int i = p_arr[1]->i;  // should raise a segfault

resume:
    print_end("dereferencing an MM_ptr inside an array");
}

//
// Testing dereferencing an _MM_array_ptr to an array of structs.
void f2() {
    print_start("dereferencing an _MM_array_ptr to structs");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_array_ptr<Data> p = mm_array_alloc<Data>(sizeof(Data) * 10);
    p[0].i = 10;
    p[0].l = 42;
    p[0].d = 3.14;

    p[9] = p[0];
    if (p[9].i != 10 || p[9].l != 42 || p[9].d != 3.14) {
        print_error("dereference.c::f2(): testing dereference an array of structs");
    }

    mm_array_free<Data>(p);
    p[0].i = 10;
    print_error("dereference.c::f2(): UAF not caught");

resume:
    print_end("dereferencing an _MM_array_ptr to structs");
}

int main() {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    f0();

    f1();

    f2();

    print_main_end(__FILE__);
    return 0;
}
