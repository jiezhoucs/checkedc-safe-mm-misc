/*
 * Testing passing MM_ptr as function parameters and as return value.
 * */

#include <stdbool.h>
#include "debug.h"

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
        print_error("Testing parameter passing failed in function f0()");
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
        print_error("Testing parameter passing failed in function f1()");
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
        print_error("Testing parameter passing failed in function f2()");
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
mm_ptr<Data> f4() {
    mm_ptr<Data> p = mm_alloc<Data>(sizeof(Data));
    p->i = 10;
    p->l = 20;
    p->d = 3.14;

    return p;
}

//
// Test using mm_ptr as a function's return value.
void f5() {
    print_start("return an MM_ptr from a function");

    mm_ptr<Data> p = f4();

    if (p->i != 10 || p->l != 20 || p->d != 3.14) {
        print_error("Testing parameter passing failed in function f5()");
    }

    print_end("return an MM_ptr from a function");
}

//
// Test using _MM_array_ptr as a function parameter.
//
void f6(mm_array_ptr<int> p, bool free_it) {
    print_start("using a _MM_array_ptr as a function parameter");
    if (free_it) mm_array_free<int>(p);

    if (setjmp(resume_context) == 1) goto resume;

    if (p[0] != 42)
        print_error("func.c::f6(): test using mm_array_ptr as func paramter failed");
    p[0] = 84;

resume:
    print_end("using a _MM_array_ptr as a function parameter");
}

//
// Test passing mm_array_ptr to a function and checking if the pointee's change
// in the callee are visibel in the caller.
//
void f7() {
    print_start("passing an mm_array_ptr as a function argument "\
            "and using it in the callee");
    mm_array_ptr<int> p = mm_array_alloc<int>(sizeof(int) * 10);
    p[0] = 42;
    f6(p, false);
    if (p[0] != 84)
        print_error("func.c::f7(): test passing mm_array_ptr to function failed");

    f6(p, true);

    print_end("passing an mm_array_ptr as a function argument "\
            "and using it in the callee");
}

/*
 * Test using multiple mm_array_ptr arguments.
 * */
void
f8(mm_array_ptr<int> p0, mm_array_ptr<long> p1, bool free_p0, bool free_p1) {
    if (setjmp(resume_context) == 1) goto resume;

    print_start("using multiple _MM_array_ptr function arguments");
    if (free_p0) mm_array_free<int>(p0);
    if (free_p1) mm_array_free<long>(p1);

    if (p0[1] != 42 || p1[2] != 84) {
        print_error("func.c::f8(): test using mm_array_ptr function arguments failed");
    }
    p0[0] = p0[1];
    p1[1] = p1[2];

resume:
    print_end("using multiple _MM_array_ptr function arguments");
}

/*
 * Test passing multiple mm_array_ptr as function arguments and checking if
 * the pointees' changes in the callee are visible to the caller.
 * */
void f9() {
    print_start("passing multiple mm_array_ptr as function arguments "\
            "and using them in the callee");
    mm_array_ptr<int> p0 = mm_array_alloc<int>(sizeof(int) * 10);
    mm_array_ptr<long> p1 = mm_array_alloc<long>(sizeof(long) * 10);
    p0[1] = 42;
    p1[2] = 84;
    f8(p0, p1, false, false);

    if (p0[0] != p0[1] || p1[1] != p1[2]) {
        print_error("func.c::f9(): test passing multiple mm_array_ptr failed");
    }

    f8(p0, p1, false, true);
    print_end("passing multiple mm_array_ptr as function arguments "\
            "and using them in the callee");
}

//
// Test returning an _MM_array_ptr from a function.
//
mm_array_ptr<int> f10() {
    mm_array_ptr<int> p = mm_array_alloc<int>(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) p[i] = i;

    return p;
}

//
// Test using mm_array_ptr as a function's return value.
void f11() {
    print_end("using mm_array_ptr from a function's return value");
    mm_array_ptr<int> p = f10();
    for (int i = 0; i < 10; i++) {
        if (p[i] != i)
            print_error("func.c::f11(): test using an mm_array_ptr as a function's\
                    return value failed");
    }

    print_end("using mm_array_ptr from a function's return value");
}

int main(int argc, char *argv[]) {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);

    f1();

    f3();

    f5();

    f7();

    f9();

    f11();

    print_main_end(__FILE__);
    return 0;
}
