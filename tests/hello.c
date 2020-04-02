#if 0
#include <stdio.h>
#endif

#include "safe_mm_checked.h"

typedef struct {
    uint64_t ID;
    int val;
    long long_val;
    float f;
} Data;

int main(int argc, char *argv[]) {
    mmsafe_ptr<Data> data_ptr = mmsafe_alloc<Data>(sizeof(Data));

    (*data_ptr).val = 10;

    mmsafe_free<Data>(data_ptr);

    return 0;
}
