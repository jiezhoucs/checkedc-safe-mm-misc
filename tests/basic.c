/**
 * Testing basic functionalities of MM_ptr:
 * - declaration
 * - reading/writing from/to a struct's fields.
 * */

#include <stdbool.h>
#include "debug.h"


//
// Test the basic declaration and dereferences to a mm_ptr.
//
void f0() {
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<Node> node_ptr = mm_alloc<Node>(sizeof(Node));

    // Test writing values to a struct.
    printf("Testing writing values to a struct's fields.\n");
    (*node_ptr).val = 10;
    (*node_ptr).l = 100;
    (*node_ptr).c = 'z';
    printf("node's val = %d.\n", (*node_ptr).val);  // should print 10
    printf("node's long_val = %ld.\n",
        (*node_ptr).l);                             // should print 100
    printf("node's c = %c.\n", (*node_ptr).c);      // should print 'z'
    putchar('\n');

    // Test reading values from a struct.
    printf("Testing reading values to a struct's fields.\n");
    int val = (*node_ptr).val;
    long long_val = (*node_ptr).l;
    char c = (*node_ptr).c;
    printf("val = %d.\n", val);                     // should print 10
    printf("long_val = %ld.\n", long_val);          // should print 100
    printf("c = %c.\n", c);                         // should print 'z'
    putchar('\n');

    // Test Use-after-Free
    printf("Testing UAF of a MM_ptr.\n");
    mm_free<Node> (node_ptr);
    // There should be a "illegal instruction" core dump for the next line.
    printf("node's val = %d.\n", (*node_ptr).val);  // should abort

resume:
    putchar('\n');
}

//
// Test basic declaration and dereferences to a mm_array_ptr.
void f1() {
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_array_ptr<int> p = mm_array_alloc<int>(sizeof(int) * 10);
    printf("Testing writing and reading values to an array.\n");
    for (int i = 0; i < 10; i++) p[i] = i;
    for (int i = 0; i < 10; i++) {
        if (p[i] != i)
            print_error("basic.c::f1(): writing and reading test failed");
    }

    // Test UAF
    printf("Testing UAF of a MM_array_ptr.\n");
    mm_array_free<int>(p);
    p[0] = 10;
    print_error("basic.c::f1(): testing UAF failed");

resume:
    putchar('\n');
}

int main() {
    print_main_start(__FILE__);

    f0();

    f1();

    print_main_end(__FILE__);
    return 0;
}
