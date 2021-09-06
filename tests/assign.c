/**
 * Test assginment-related functionalities for MMSafe_ptr.
 * */

#include "debug.h"

typedef struct {
    int i;
    long j;
    float f;
    Node n;
    int ir[30];
    mm_ptr<Node> pn;
} Obj;

typedef struct {
    int i;
    long l;
    mm_array_ptr<char> p0;
    Obj o0;
    Obj o;
    mm_ptr<Obj> po;
    int ir[30];
    Obj oarr[10];
    Obj *op;
} NewNode;

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
        print_error("Assignment testing failed in function f0");
    }

    mm_free<Data>(p1);

    int i = p0->i;  // should raise an Illegal Instruction error.

resume:
    printf("Testing assignment between MMSafe_ptr succeeded.\n\n");
}

//
// Test assigning NULL to an MMSafe_ptr.
//
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

//
// Test assigning an MM_array_ptr to another.
//
void f2() {
    if (setjmp(resume_context) == 1) goto resume;

    mm_array_ptr<int> p = mm_array_alloc<int>(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) p[i] = i + 20;
    mm_array_ptr<int> p1 = p;
    for (int i = 0; i < 10; i++) {
        if (p[i] != p1[i])
            print_error("assign.c::f2(): pointer assignment test failed");
    }

resume:
    printf("Testing assignment between _MM_array_ptr succeeded.\n\n");
}

//
// Test assigning NULL to an _MM_array_ptr and an array of _MM_array_ptr.
//
void f3() {
    if (setjmp(resume_context) == 1) goto resume0;

    mm_array_ptr<int> p0 = NULL;
    p0[10] = 10;  // should raise a segfault
    print_error("assign.c::f3(): assigning NULL ptr test failed");

resume0:
    print_end("assigning NULL to an _MM_array_ptr");

    if (setjmp(resume_context) == 1) goto resume1;

    mm_array_ptr<int> p_arr[2] = { NULL };
    p_arr[1][1] = 10;  // segmentation fault
    print_error("assign.c::f3(): assigning NULL ptr test failed");

resume1:
    print_end("assigning NULL to an _MM_array_ptr in an array");
}

/*
 * Test assigning the result of a ternary expression to a pointer.
 * */
void f4() {
    print_start("assigning the result of a ternary expression to a pointer");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<Data> pd = mm_alloc<Data>(sizeof(Data));
    pd->i = 42;
    pd->j = 84;

    mm_ptr<Data> pd1 = pd->i > 21 ? pd : NULL;
    mm_ptr<int> pi = pd->j > 42 ? &pd->i : NULL;

    if (pd1->i != 42 || *pi != 42) {
        print_error("assign.c::f4(): assigning the result of a ternary expression "
                    "to an mmsafe pointer");
    }

    mm_free<Data>(pd);
    printf("%d\n", *pi);   // UAF

resume:
    print_end("assigning the result of a ternary expression to a pointer");
}

/*
 * Test assigning an array of a struct pointed by an mmptr to an mmarrayptr.
 * */
void f5() {
    print_start("assigning an array of a struct pointed by an mmptr to an mmarrayptr");
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume0;

    mm_ptr<NewNode> pnn = mm_alloc<NewNode>(sizeof(NewNode));
    pnn->ir[10] = 42;
    pnn->oarr[5].ir[20] = 84;
    mm_array_ptr<int> pi = pnn->ir;
    mm_array_ptr<int> pi2 = pnn->oarr[5].ir + 20;

    if (pi[10] != 42 || *pi2 != 84) {
        print_error("assign.c::f5(): error0");
    }

    mm_free<NewNode>(pnn);
    pi[10] = 40;  /* UAF */

resume0:
    if (setjmp(resume_context) == 1) goto resume1;

    pnn = mm_alloc<NewNode>(sizeof(NewNode));
    pnn->po = mm_alloc<Obj>(sizeof(Obj));
    pnn->po->ir[10] = 100;
    pi = pnn->po->ir;

    if (*(pi + 10) != 100) {
        print_error("assign.c::f5(): error1");
    }

    mm_free<Obj>(pnn->po);
    *pi = 10;  /* UAF */

resume1:
    print_end("assigning an array of a struct pointed by an mmptr to an mmarrayptr");
}

int main(int argc, char *argv[]) {
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    printf("===== Begin testing assignment functionality. =====\n");

    // _MM_ptr testing
    f0();

    f1();

    // _MM_array_ptr testing
    f2();

    f3();

    f4();

    f5();

    printf("===== Finish testing assignment functionality. =====\n\n");
    return 0;
}
