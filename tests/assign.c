/**
 * Test assginment-related functionalities for MMSafe_ptr.
 * */

#include "header.h"

int main(int argc, char *argv[]) {
    mmsafe_ptr<Data> p0 = mmsafe_alloc<Data>(sizeof(Data));
    p0->i = 1;
    p0->l = 100;
    p0->d = 4.2;

    mmsafe_ptr<Data> p1 = p0;

    // Checked if the new pointer points to the correct memory object.
    if (p1->i != 1 || p1->l != 100 || p1->d != 4.2 || p1->f != p0->f) {
        perror("Assignment testing failed at line 15!\n");
    }
    printf("Assignment testing succeeded.\n");

    mmsafe_free<Data>(p1);

    int i = p0->i;  // should raise an Illegal Instruction error.
    return 0;
}
