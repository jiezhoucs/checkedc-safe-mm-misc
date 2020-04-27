/*
 * This files provides a customized memory allocator and a deallocator
 * for struct objects pointed by a _MM_ptr<T>.
 * */

#include "safe_mm_checked.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define ID_SIZE 8

// A helper struct that has the "same" inner structure as a mm_ptr.
// It is used to help create a _MM_ptr.
typedef struct {
  void *p;
  uint64_t ID;
} _MM_ptr_Rep;


// A helper struct that has the "same" inner structure as a mm_array_ptr.
// It is used to help create a _MM_array_ptr.
typedef struct {
  void *p;
  uint64_t ID;
  void *p_ID;  // pointer to ID
} _MM_array_ptr_Rep;


uint64_t key = 1;

//
// Function: mm_alloc()
//
// This is a customized memory allocator to allocate a struct object
// on the heap. First it creates a helper struct
// _MM_ptr_Rep and initializes its two field accordingly. Second it casts
// the type of an implicit pointer to the type of a pointer to a _MM_ptr.
// Finally it returns the dereference of the pointer to _MM_ptr.
//
__attribute__ ((noinline))
for_any(T) mm_ptr<T> mm_alloc(unsigned long struct_size) {
    void *raw_ptr = malloc(struct_size + ID_SIZE);

    // Generate a random number as the ID.
    // FIXME: replace the naive rand() function with a robust random
    // number generator which gives a good 64-bit random number.
    uint64_t new_ID = key++;
    // We assume that ID is always the first field of a struct; so here
    // we can set the ID without knowing the concrete structure of a struct.
    *((uint64_t *)(raw_ptr)) = new_ID;

    // Create a helper struct.
    // Note that in the initialization of the _MM_array_ptr_Rep, it adds
    // ID_SIZE to raw_ptr whose type is "void *". Arithmetic on "void *"
    // is undefined behavior by C's specification. For GCC or Clang,
    // the "-pedantic-errors" flag would cause a program to fail compiling
    // if it has such an undefined behavior.
    // Related reading: https://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c
    _MM_ptr_Rep safe_ptr = { .p = raw_ptr + ID_SIZE, .ID = new_ID };

    mm_ptr<T> *mm_ptr_ptr = (mm_ptr<T> *)&safe_ptr;
    return *mm_ptr_ptr;
}

//
// Function: mm_array_alloc()
//
// This is a customized memory allocator to allocator an array on the heap.
// Because the allocated array has an ID attached right before the first
// element of it, the allocator allocates 8 more bytes for the ID.
// It returns a _MM_array_ptr that contains a pointer to the first element
// and a pointer to the ID.
//
__attribute__ ((noinline))
for_any(T) mm_array_ptr<T> mm_array_alloc(unsigned long array_size) {
  void *raw_ptr = malloc(array_size + ID_SIZE);

  uint64_t new_ID = key++;
  *((uint64_t *)(raw_ptr)) = new_ID;

  _MM_array_ptr_Rep safe_ptr = {
    .p = raw_ptr + ID_SIZE, .ID = new_ID, .p_ID = raw_ptr
  };

  mm_array_ptr<T> *mm_array_ptr_ptr = (mm_array_ptr<T> *)&safe_ptr;
  return *mm_array_ptr_ptr;
}


//
// Function: mm_free()
//
// This is a customized memory deallocator. It sets the ID of the struct to 0
// and calls free() from the stdlib to free the struct.
//
// @param p - a _MM_ptr whose pointee is going to be freed.
//
for_any(T) void mm_free(mm_ptr<T> p) {
  // Without the "volatile" keyword, Clang may optimize away the next
  // statement.
  volatile _MM_ptr_Rep *mm_ptr_ptr = (_MM_ptr_Rep *)&p;

  // Compute the real starting address of the allocated heap struct.
  uint64_t *raw_ptr = (uint64_t *)(((uint64_t)(mm_ptr_ptr->p)) - 8);
  // This step may not be necessary in some cases. In some implementation,
  // free() zeros out all bytes of the memory region of the freed object.
  *raw_ptr = 0;

  free(raw_ptr);
}

for_any(T) void mm_array_free(mm_array_ptr<T> p) {
  // TODO
}
