/*
 * Testing the key check optimization pass. We will not run this program but
 * mainly to examine the generated IR.
 * */

#include "debug.h"

typedef struct {
    int i;
    int l;
    long j;
    double f;
    Node n;
    mm_array_ptr<int> parr;
    mm_ptr<Node> pn;
    int *p;
    char *buf;
} Obj;

typedef struct {
    int i;
    long l;
    mm_array_ptr<int> p0;
    Obj o0;
    Obj o;
    mm_ptr<Obj> p1;
    Obj arr[10];
    Obj *rp;
} NewNode;


__attribute__ ((noinline))
void f0(mm_ptr<Obj> po, int i, long l, mm_array_ptr<int> pi) {
    printf("%d, %ld, %d\n", po->i + i, po->l + l, pi[i]);
}


__attribute__ ((noinline))
void f1(int num) {
    mm_ptr<Obj> po = mm_alloc<Obj>(sizeof(Obj));
    po->i = num;

    mm_array_ptr<int> pi = mm_array_alloc<int>(sizeof(int) * num);
    pi[0] = num * 2;

    f0(po, num, (long)(num * 2), pi);
}

#if 0
int main(int argc, char *argv[]) {
    print_main_start(__FILE__);

    f1(argc);

    print_main_end(__FILE__);
    return 0;
}
#endif
