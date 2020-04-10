/*
 * Testing passing MM_ptr as function parameters and as return value.
 * */

#include <stdbool.h>
#include "inc.h"

//
// Test using a mm_ptr of function parameter.
//
void f0(mm_ptr<Data> p, bool free_it) {
    if (free_it) {
        mm_free<Data>(p);
        return;
    }

    (*p).i = 10;
    (*p).j = 20;
    (*p).d = 3.14;

    if (p->i != 10 || p->j != 20 || p->d != 3.14) {
        perror("ERROR: Testing parameter passing failed in function f0().\n");
    }
}

//
// Test passing mm_ptr to a function and checking if the pointee's change
// in the callee are visial in the caller.
//
void f1() {
    print_start("passing MM_ptr to a function");
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<Data> p = mm_alloc<Data>(sizeof(Data));
    f0(p, false);

    if (p->i != 10 || p->j != 20 || p->d != 3.14) {
        perror("ERROR: Testing parameter passing failed in function f1().\n");
    }

    // Free the struct in the callee.
    f0(p, true);
    // There should be a "illegal instruction" core dump for the next line.
    printf("i = %d.\n", (*p).i);              // should abort

resume:
    print_end("passing MM_ptr to a function");
}

//
// Test using multiple MM_ptr passed by function parameter.
void f2(mm_ptr<Data> p0, mm_ptr<Data> p1) {
    if (p0->i != p1->i || p0->l != p1->l || p0->d != p1->d) {
        perror("ERROR: Testing parameter passing failed in function f2().\n");
    }
}

//
// Test passing multiple MM_ptr to a function.
void f3() {
    print_start("passing multiple MM_ptr to a function");

    mm_ptr<Data> p = mm_alloc<Data>(sizeof(Data));
    mm_ptr<Data> p1 = mm_alloc<Data>(sizeof(Data));

    p->i = 10;
    p->l = 20;
    p->d = 3.14;

    p1->i = 10;
    p1->l = 20;
    p1->d = 3.14;

    f2(p, p1);

    print_end("passing multiple MM_ptr to a function");
}

//
// Test returning an MM_ptr from a function.
//
// Here we add a "int i"parameter but doesn't use it because the
// current Checked C requires that
// "function with no prototype cannot have a return type that is a checked type"
//  We may need relex this restriction for MM_ptr.
mm_ptr<Data> f4(int i) {
    mm_ptr<Data> p = mm_alloc<Data>(sizeof(Data));
    p->i = 10;
    p->l = 20;
    p->d = 3.14;

    return p;
}

void f5() {
    print_start("return an MM_ptr from a function");

    mm_ptr<Data> p = f4(0);

    if (p->i != 10 || p->l != 20 || p->d != 3.14) {
        perror("ERROR: Testing parameter passing failed in function f1().\n");
    }

    print_end("return an MM_ptr from a function");
}

int main(int argc, char *argv[]) {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);

    f1();

    f3();

    f5();

    print_main_end(__FILE__);
    return 0;
}
