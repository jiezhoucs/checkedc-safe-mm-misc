/**
 * Testing basic functionalities of MM_ptr:
 * - declaration
 * - reading/writing from/to a struct's fields.
 * */

#include <stdbool.h>
#include "header.h"


//
// Test the basic declaration and dereferences to a mmsafe_ptr.
//
void f0() {
    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mmsafe_ptr<Node> node_ptr = mmsafe_alloc<Node>(sizeof(Node));

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
    mmsafe_free<Node> (node_ptr);
    // There should be a "illegal instruction" core dump for the next line.
    printf("node's val = %d.\n", (*node_ptr).val);  // should abort

resume:
    putchar('\n');
}

void f1() {
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
    printf("Finished testing dereferencing & free an MM_Ptr inside a struct.\n\n");
}

int main() {
    print_start("basic declaration and dereference of MM_ptr");

    f0();

    f1();

    print_end("basic declaration and dereference of MM_ptr");
    return 0;
}
