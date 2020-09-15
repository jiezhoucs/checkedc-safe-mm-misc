/*
 * Test cast-related operations.
 * */

#include "debug.h"
#include <string.h>

typedef struct {
    short type;
    unsigned year;
    char name[20];
    float weight;
    char maker[20];
} Vehicle;

typedef struct {
    short type;
    unsigned year;
    char name[20];
    float weight;
    char maker[20];
    unsigned top_speed;
} Car;


//
// Test casting one MMSafe pointer of one type to another type.
//
void f0() {
    if (setjmp(resume_context) == 1) goto resume;

    mm_ptr<Car> p0 = mm_alloc<Car>(sizeof(Car));
    p0->type = 1;
    strncpy(p0->name, "Mustang", 20);
    p0->weight = 3600;
    strncpy(p0->maker, "Ford", 20);
    p0->top_speed = 155;

    mm_ptr<Vehicle> p1 = (mm_ptr<Vehicle>)p0;

    // Basic field equality tests.
    if (strcmp(p0->name, p1->name) != 0 || strcmp(p0->maker, p1->maker) ||
        p0->weight != p1->weight) {
        print_error("Cast testing failed in f0()");
    }

    if (strcmp(p0->name, ((mm_ptr<Vehicle>)p0)->name) ||
        strcmp(p0->maker, ((mm_ptr<Vehicle>)p0)->maker) ||
        p0->weight != ((mm_ptr<Vehicle>)p0)->weight) {
        print_error("Cast testing failed in f0()");
    }

    mm_free<Vehicle>(p1);

    // Should raise an Illegal instruction error.
    printf("Car maker : %s\n", p0->maker);

resume:
    printf("Testing explicit cast between MMSafe pointers succeeded.\n");
}

int main(int argc, char *argv[]) {
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    printf("===== Begin testing cast functionality. =====\n");

    f0();

    printf("===== Finish testing cast functionality. =====\n\n");
    return 0;
}
