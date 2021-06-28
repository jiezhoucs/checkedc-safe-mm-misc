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

/**
 * Test dereferencing a _MM_array_ptr inside a struct.
 * */
typedef struct {
    int i;
    long l;
    mm_array_ptr<Data> dp_arr;
} DataWithPtrArr;

void f3() {
    print_start("dereferencing an _MM_array_ptr inside a struct");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<DataWithPtrArr> p = mm_alloc<DataWithPtrArr>(sizeof(DataWithPtrArr));
    p->dp_arr = mm_array_alloc<Data>(sizeof(Data) * 10);
    p->dp_arr[1].i = 10;
    p->dp_arr[1].l = 42;
    p->dp_arr[1].d = 3.14;
    p->dp_arr[9] = p->dp_arr[1];

    Data d = p->dp_arr[9];
    if (d.i != 10 || d.l != 42 || d.d != 3.14) {
        print_error("dereference.c::f3(): testing dereferencing an _MM_array_ptr"
                " inside a struct");
    }

    mm_array_free<Data>(p->dp_arr);
    p->dp_arr[0] = d;
    print_error("dereference.c::f3(): UAF not caught");

resume:
    print_end("dereferencing an _MM_array_ptr inside a struct");
}

/**
 * Test dereferencing an _MM_array_ptr inside an array of _MM_array_ptr.
 * */
void f4() {
    print_start("dereferencing an _MM_array_ptr inside an array of _MM_array_ptr");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_array_ptr<mm_array_ptr<Data>> p =
        mm_array_alloc<mm_array_ptr<Data>>(sizeof(mm_array_ptr<Data>) * 10);
    p[0] = mm_array_alloc<Data>(sizeof(Data) * 10);
    p[1] = mm_array_alloc<Data>(sizeof(Data) * 10);

    for(int i = 0; i < 10; i++) {
        p[0][i].i = i + 1;
        p[1][i].i = i + 2;
    }

    for (int i = 0; i < 10; i++) {
        if (p[0][i].i + 1 != p[1][i].i) {
            print_error("dereference.c::f4(): dereferencing an _MM_array_ptr"
                    "inside an array of _MM_array_ptr");
        }
    }

    mm_array_free<Data>(p[1]);
    p[1][1].i = 10;
    print_error("dereference.c::f4(): UAF not caught");

resume:
    print_end("dereferencing an _MM_array_ptr inside an array of _MM_array_ptr");
}

/**
 * Test directly dereferencing an mmsafeptr pointed by another pointer
 * via "**", e.g., **p.
 * */
void f5(mm_array_ptr<int> *p, mm_ptr<mm_ptr<Node>> p1) {
    print_start("dereferencing an mmsafeptr pointed by another pointer");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume1;

    if (**p != 10 || (**p1).val != 42) {
        print_error("dereference.c::f6(): dereferencing an mmsafeptr pointer "
                    "by another pointer");
    }

    mm_array_free<int>(*p);
    printf("%d\n", **p);   // UAF

resume1:
    if (setjmp(resume_context) == 1) goto resume;
    mm_free<Node>(*p1);
    printf("%d\n", (**p1).val);   // UAF

resume:
    print_end("dereferencing an mmsafeptr pointed by another pointer");
}

/* This function calls f5() */
void f6() {
    mm_array_ptr<int> p = mm_array_alloc<int>(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) p[i] = i + 10;

    mm_ptr<Node> pN = mm_alloc<Node>(sizeof(Node));
    pN->val = 42;
    mm_ptr<mm_ptr<Node>> ppN = mm_alloc<mm_ptr<Node>>(sizeof(mm_ptr<Node>));
    *ppN = pN;

    f5(&p, ppN);
}

int main() {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    f0();

    f1();

    f2();

    f3();

    f4();

    f6();

    print_main_end(__FILE__);
    return 0;
}
