/**
 * Tests of the _multiple qualifier
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

_multiple int gi = 30;
_multiple int gi1;
_multiple static int sgi = 40;
_multiple static int sgi1;

_multiple NewNode GNN;
_multiple static NewNode SGNN;

/*
 * Testing _multiple local and static local variables.
 * */
void f0() {
    print_start("_multiple local variables");
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    _multiple int i = 10;
    mm_ptr<int> p = &i;
    if (*p != 10) {
        print_error("multiple.c::f0():: getting the address of a stack integer");
    }

    // Test getting the address of a field of a local struct.
    _multiple NewNode NN;
    mm_ptr<long> pl = &NN.l;
    mm_ptr<Obj> po = &NN.o;
    mm_ptr<int> pi = &NN.arr[3].i;
    NN.l = 10, NN.o.j = 20, NN.arr[3].i = 30;
    if (*pl != 10 || po->j != 20 || *pi != 30) {
        print_error("multiple.c::f0():: getting the address of a field of a "
                    "local struct");
    }

    // Test getting the address of a static local integer.
    _multiple static int si = 30;
    p = &si;
    if (*p != 30) {
        print_error("multiple.c::f0():: getting the address of a static local integer");
    }

    // Test getting the address of a field of a static local variable.
    _multiple static NewNode SNN;
    pl = &SNN.l;
    po = &SNN.o;
    pi = &SNN.arr[3].i;
    SNN.l = 10, SNN.o.j = 20, SNN.arr[3].i = 30;
    if (*pl != 10 || po->j != 20 || *pi != 30) {
        print_error("multiple.c::f0():: getting the address of a field of a "
                    "static local struct");
    }

    print_end("_multiple local variables");
    return;

resume:
    print_error("multiple.c::f0(): Key Check failed! This shouldn't happen!");
}

/**
 * Test _multiple global variables
 * */
void f1() {
    print_start("_multiple global variables");
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    // Test getting the address of a global int.
    mm_ptr<int> pgi = &gi;
    mm_ptr<int> pgi1 = &gi1;
    gi1 = 35;
    if (*pgi != 30 || *pgi1 != 35) {
        print_error("multiple.c::f1():: getting the address of a global int");
    }

    // Test getting the address of a static global int.
    mm_ptr<int> psgi = &sgi;
    mm_ptr<int> psgi1 = &sgi1;
    sgi1 = 45;
    if (*psgi != 40 || *psgi1 != 45) {
        print_error("multiple.c::f1():: getting the address of a static global int");
    }

    // Test getting the address of a field of a global variable.
    mm_ptr<long> pl = &GNN.l;
    mm_ptr<Obj> po = &GNN.o;
    mm_ptr<int> pi = &GNN.arr[3].i;
    GNN.l = 10, GNN.o.j = 20, GNN.arr[3].i = 30;
    if (*pl != 10 || po->j != 20 || *pi != 30) {
        print_error("multiple.c::f0():: getting the address of a field of a "
                    "global struct");
    }

    // Test getting the address of a field of a static global variable.
    pl = &SGNN.l;
    po = &SGNN.o;
    pi = &SGNN.arr[3].i;
    SGNN.l = 10, SGNN.o.j = 20, SGNN.arr[3].i = 30;
    if (*pl != 10 || po->j != 20 || *pi != 30) {
        print_error("multiple.c::f0():: getting the address of a field of a "
                    "static global struct");
    }
    print_end("_multiple global variables");
    return;

resume:
    print_error("multiple.c::f0(): Key Check failed! This shouldn't happen!");
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
