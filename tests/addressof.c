/*
 * Tests on the address-of ('&') operator.
 * */

#include "debug.h"

typedef struct {
    int i;
    long j;
    float f;
    Node n;
    mm_ptr<Node> pn;
} Obj;

typedef struct {
    int i;
    long l;
    mm_array_ptr<int> p0;
    Obj o0;
    Obj o;
    mm_ptr<Obj> p1;
    Obj arr[10];
    Obj *op;
} NewNode;

/*
 * Test '&' on an inner object inside a struct.
 * */
void f0() {
    print_start("address-of operator '&' on an field of a struct");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume1;

    mm_ptr<NewNode> p = mm_alloc<NewNode>(sizeof(NewNode));

    // Test getting the address of a struct field.
    p->o.j = 42;
    mm_ptr<Obj> p1 = &p->o;
    if (p1->j != 42) {
        print_error("addressof.c::f0: getting address of a struct field");
    }
    mm_free<NewNode>(p);
    printf("%ld\n", p1->j);

resume1:
    // Test '&' on an element of an array of a struct.
    if (setjmp(resume_context) == 1) goto resume2;

    p = mm_alloc<NewNode>(sizeof(NewNode));
    p->arr[4].j = 42;
    mm_ptr<Obj> p2 = &p->arr[4];
    if (p2->j != 42) {
        print_error("addressof.c::f0: getting address of an item in an array "
                    "of a struct");
    }
    mm_free<NewNode>(p);
    printf("%ld\n", p2->j);

resume2:
    // Test '&' on object pointed by an mm_ptr pointed by another mm_ptr.
    if (setjmp(resume_context) == 1) goto resume3;

    p = mm_alloc<NewNode>(sizeof(NewNode));
    p->p1 = mm_alloc<Obj>(sizeof(Obj));
    p->p1->n.val = 42;
    mm_ptr<Node> p3 = &p->p1->n;
    if (p3->val != 42) {
        print_error("addressof.c::f0: getting the address of a field in a "
                    "struct pointed by an mm_ptr that is in another struct "
                    "that is pointed by another mm_ptr");
    }
    mm_free<Obj>(p->p1);
    printf("%d\n", p3->val);

resume3:
    // Test "&(*p).o".
    if (setjmp(resume_context) == 1) goto resume4;

    p = mm_alloc<NewNode>(sizeof(NewNode));
    p->o.j = 42;
    p1 = &(*p).o;
    if (p1->j != 42) {
        print_error("addressof.c::f0: getting address of a struct field (2)");
    }
    mm_free<NewNode>(p);
    printf("%ld\n", p1->j);

resume4:
    // Test '&' on an field of a struct inside a struct.
    if (setjmp(resume_context) == 1) goto resume5;

    p = mm_alloc<NewNode>(sizeof(NewNode));
    p->o.n.val = 42;
    p3 = &p->o.n;
    if (p3->val != 42) {
        print_error("addressof.c::f0: get the address of a field of a struct "
                    "inside another struct");
    }
    mm_free<NewNode>(p);
    printf("%d\n", p3->val);

resume5:
    // Test '&' on a field of a struct inside another struct inside another struct.
    if (setjmp(resume_context) == 1) goto resume;

    p = mm_alloc<NewNode>(sizeof(NewNode));
    p->o.n.val = 42;
    mm_ptr<int> p4 = &p->o.n.val;
    if (*p4 != 42) {
        print_error("addressof.c::f0: getting the address of a field of a struct "
                    "inside another struct inside another struct");
    }
    mm_free<NewNode>(p);
    printf("%d\n", *p4);

resume:
    print_end("address-of operator '&' on an field of a struct");
}

/*
 * Test '&' on an item of an array pointed by an mm_array_ptr.
 * */
void f1() {
    print_start("address-of operator '&' on an item of an array");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume1;

    mm_array_ptr<NewNode> p = mm_array_alloc<NewNode>(sizeof(NewNode) * 10);
    p[3].l = 42;
    p[3].o.j = 84;
    mm_ptr<NewNode> p1 = &p[3];
    if (p1->l != 42 || p1->o.j != 84) {
        print_error("addressof.c::f1: getting the address of a struct item of "
                    "an array of such struct");
    }
    mm_free<NewNode>(p);
    printf("%ld\n", p1->l);  // UAF

resume1:
    if (setjmp(resume_context) == 1) goto resume;

    p = mm_array_alloc<NewNode>(sizeof(NewNode) * 10);
    p[2].arr[4].j = 42;
    mm_ptr<Obj> p2 = &p[2].arr[4];
    if (p2->j != 42) {
        print_error("addressof.c::f1: getting the address of an item of an "
                    "array inside a struct item of an array of such struct");
    }

    p++;
    p[2].arr[4].f = 3.14;
    mm_ptr<float> p3 = &p[2].arr[4].f;
    if (*p3 != 3.14f) {
        print_error("addressof.c::f1: getting the address of an item of an "
                    "array inside a struct item of an array of such struct");
    }
    mm_array_free<NewNode>(--p);  // UAF
    printf("%f\n", *p3);

resume:
    print_end("address-of operator '&' on an item of an array");
}

//
// More tests.
//
void f2() {
    print_start("more tests on the address-of operator '&'");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    // Test deferencing through a raw pointer inside a struct pointed by an mmsafe ptr.
    mm_ptr<NewNode> p = mm_alloc<NewNode>(sizeof(NewNode));
    p->op = malloc(sizeof(Obj));
    p->op->i = 42;
    int *ip = &p->op->i;
    if (*ip != 42) {
        print_error("addressof.c::f2: dereferencing through a raw pointer "
                    "inside a struct pointed by an mmptr");
    }

    // Test using '[]' to dereference a raw pointer inside a struct pointed by an mmsafe ptr.
    p->op = malloc(sizeof(Obj) * 30);
    p->op[10].i = 42;
    ip = &p->op[10].i;
    if (*ip != 42) {
        print_error("addressof.c::f2: using '[]' to dereference through a raw pointer "
                    "inside a struct pointed by an mmptr");
    }

    // Test derefencing through an mmsafe ptr inside a struct pointed by an mmsafe ptr.
    p->p0 = mm_array_alloc<int>(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) {
        p->p0[i] = i + 10;
    }
    mm_ptr<int> pi = &p->p0[5];
    if (*pi != 15) {
        print_error("addressof.c::f2: dereferencing through an mmarrayptr "
                    "inside a struct pointed by an mmptr");
    }
    mm_array_free<int>(p->p0);
    *pi = 20;  // UAF

resume:
    print_end("more tests on the address-of operator '&'");
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
