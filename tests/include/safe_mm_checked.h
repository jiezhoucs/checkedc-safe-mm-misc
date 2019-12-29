/*
 * This files provides a customized memory allocator and a deallocator
 * for struct objects pointed by a _TS_ptr<T>.
 * */
#ifndef _SAFE_MM_CHECKED_H
#define _SAFE_MM_CHECKED_H

#include "stdchecked.h"
#include <stdint.h>
#include <stdlib.h>

// A helper struct that has the "same" inner structure as a mmsafe_ptr.
// It is used to help create a _TS_ptr.
typedef struct {
  void *p;
  uint64_t ID;
} _TS_ptr_Rep;

uint64_t key = 1;

//
// Function: mmsafe_alloc()
//
// This is a customized memory allocator. First it creates a helper struct
// _TS_ptr_Rep and initializes its two field accordingly. Second it casts
// the type of an implicit pointer to the type of a pointer to a _TS_ptr.
// Finally it returns the dereference of the pointer to _TS_ptr.
//
__attribute__ ((noinline))
for_any(T) tsptr<T> mmsafe_alloc(unsigned long struct_size) {
    void *raw_ptr = malloc(struct_size);

    // Generate a random number as the ID.
    // FIXME: replace the naive rand() function with a robust random
    // number generator which gives a good 64-bit random number.
    uint64_t new_ID = key++;
    // We assume that ID is always the first field of a struct; so here
    // we can set the ID without knowing the concrete structure of a struct.
    *((uint64_t *)(raw_ptr)) = new_ID;

    // Create a helper struct.
    _TS_ptr_Rep safe_ptr = { .p = raw_ptr, .ID = new_ID };

    tsptr<T> *mmsafe_ptr_ptr = (tsptr<T> *)&safe_ptr;
    return *mmsafe_ptr_ptr;
}

//
// Function: mmsafe_free()
//
// This is a customized memory deallocator. It sets the ID of the struct to 0
// and calls free() from the stdlib to free the struct.
//
// @param p - a _TS_ptr whose pointee is going to be freed.
//
for_any(T) void mmsafe_free(tsptr<T> p) {
  // Without the "volatile" keyword, Clang may optimize away the next
  // statement.
  volatile _TS_ptr_Rep *mmsafe_ptr_ptr = (_TS_ptr_Rep *)&p;

  // This step may not be necessary in some cases. In some implementation,
  // free() zeros out all bytes of the memory region of the freed object.
  *((uint64_t *)(mmsafe_ptr_ptr->p)) = 0;

  free(mmsafe_ptr_ptr->p);
}
#endif
