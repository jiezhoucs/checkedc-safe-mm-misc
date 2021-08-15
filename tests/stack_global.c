/**
 * Tests of temporal memory safety for the stack and global objects.
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
    mm_array_ptr<char> p0;
    Obj o0;
    Obj o;
    mm_ptr<Obj> p1;
    Obj arr[10];
    Obj *op;
} NewNode;


void f0(mm_array_ptr<char> p, mm_ptr<int> pi) {
    if (*p != 'c' || *pi != 42) {
        print_error("stack.c::f0():: passing a local array to a function");
    }
}

/*
 * Test passing the address of a local variable to function call that takes
 * an mmsafe pointer.
 * */
void f1() {
    print_start("passing mmsafe pointers to local variables to function call");
    char buf[100];
    buf[2] = 'c';
    int num = 42;
    f0(buf + 2, &num);
    print_end("passing mmsafe pointers to local variables to function call");
}

/**
 * Test passing the address of a local static variable to a function call
 * that takes an mmsafe pointer.
 * */
void f2() {
    print_start("passing mmsafe pointers to local static variables to function call");
    static char buf[100];
    static int num = 42;
    buf[2] = 'c';
    f0(buf + 2, &num);
    print_end("passing mmsafe pointers to local static variables to function call");
}

/*
 * Test passing the address of a global variable to a function that takes
 * an mmsafe pointer pointer.
 * */
static char gs_buf[100];
static int gs_num = 42;
char g_buf[100];
int g_num = 42;
void f3() {
    print_start("passing mmsafe pointers to global variables to function call");
    gs_buf[2] = 'c';
    g_buf[2] = 'c';
    f0(gs_buf + 2, &gs_num);
    f0(g_buf + 2, &g_num);
    print_end("passing mmsafe pointers to global variables to function call");
}

/* Assigning the address of a local object to the heap pointed by an mm_ptr. */
void f4(mm_ptr<NewNode> pn) {
    char buf[100];
    buf[0] = 'a';
    pn->p0 = buf;
    if (pn->p0[0] != 'a') {
        printf("%d\n", pn->p0[0]);
        print_error("stack.c::f4(): assigning a local array to an mm_array_ptr<char>");
    }
}

/*
 * Test stack object escaping.
 * */
void f5() {
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    print_start("dereferencing a local variable after its frame is released");
    mm_ptr<NewNode> pn = mm_alloc<NewNode>(sizeof(NewNode));
    f4(pn);

    printf("%c\n", pn->p0[0]); // UAF: pn->p0[0] is one the stack of f4().

resume:
    print_end("dereferencing a local variable after its frame is released");
}

int main() {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    f1();
    f2();
    f3();

    f5();

    print_main_end(__FILE__);
    return 0;
}
