/*
 * This files provides a customized memory allocator and a deallocator
 * for struct objects pointed by a _MM_ptr<T>.
 * */

#include "safe_mm_checked.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define LOCK_SIZE 8
#define HEAP_PADDING 8

// A helper struct that has the same inner structure as an mm_ptr.
// It is used to help create an mm_ptr.
typedef struct {
  void *p;
  uint64_t key;
} _MM_ptr_Rep;


// A helper struct that has the "same" inner structure as a mm_array_ptr.
// It is used to help create a _MM_array_ptr.
typedef struct {
  void *p;
  uint64_t key;
  uint64_t *lock_ptr;  // pointer to the lock.
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
// Note that arithmetic on "void *" is undefined behavior by C's specification.
// For GCC or Clang, the "-pedantic-errors" flag would cause a program to fail
// compiling if it has such an undefined behavior.
// Related reading: https://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c
__attribute__ ((noinline))
for_any(T) mm_ptr<T> mm_alloc(unsigned long struct_size) {
    // We need the HEAP_PADDING to ensure that mm_ptr inside a struct
    // is aligned by 16 bytes.
    // See this issue for the reason: https://github.com/jzhou76/checkedc-llvm/issues/2
    void *raw_ptr = malloc(struct_size + HEAP_PADDING + LOCK_SIZE);

    // Generate a random number as the key.
    // FIXME: replace the naive rand() function with a robust random
    // number generator which gives a good 64-bit random number.
    uint64_t new_key = key++;
    // We assume that key is located right before the first field of a struct.
    raw_ptr += HEAP_PADDING;
    *((uint64_t *)(raw_ptr)) = new_key;

    // Create a helper struct.
    _MM_ptr_Rep safe_ptr = { .p = raw_ptr + LOCK_SIZE, .key = new_key };

    mm_ptr<T> *mm_ptr_ptr = (mm_ptr<T> *)&safe_ptr;
    return *mm_ptr_ptr;
}

//
// Function: mm_array_alloc()
//
// This is a customized memory allocator to allocator an array on the heap.
// Because the allocated array has an lock attached right before the first
// element of it, the allocator allocates 8 more bytes for the lock.
// It returns a _MM_array_ptr that contains a pointer to the first element
// and a pointer to the lock.
//
__attribute__ ((noinline))
for_any(T) mm_array_ptr<T> mm_array_alloc(unsigned long array_size) {
  void *raw_ptr = malloc(array_size + LOCK_SIZE + HEAP_PADDING);

  uint64_t new_key = key++;
  raw_ptr += HEAP_PADDING;
  *((uint64_t *)(raw_ptr)) = new_key;

  _MM_array_ptr_Rep safe_ptr = {
    .p = raw_ptr + LOCK_SIZE, .key = new_key, .lock_ptr = raw_ptr
  };

  mm_array_ptr<T> *mm_array_ptr_ptr = (mm_array_ptr<T> *)&safe_ptr;
  return *mm_array_ptr_ptr;
}


//
// Function: mm_free()
//
// This is a customized memory deallocator for mm_ptr.
// It sets the lock of the singleton memory object to 0
// and calls free() from the stdlib to free the whole memory object.
//
// @param p - a _MM_ptr whose pointee is going to be freed.
//
for_any(T) void mm_free(mm_ptr<T> p) {
  // Without the "volatile" keyword, Clang may optimize away the next
  // statement.
  volatile _MM_ptr_Rep *mm_ptr_ptr = (_MM_ptr_Rep *)&p;

  void *lock_ptr = mm_ptr_ptr->p - LOCK_SIZE;
  // This step may not be necessary in some cases. In some implementation,
  // free() zeros out all bytes of the memory region of the freed object.
  *(uint64_t *)lock_ptr = 0;

  free(lock_ptr - HEAP_PADDING);
}

//
// Function: mm_array_free()
//
// This is a customized memory deallocator to free heap arrays pointed by
// mm_array_ptr<T>.
//
// @param p - a _MM_array_ptr whose pointee is going to be freed.
//
for_any(T) void mm_array_free(mm_array_ptr<T> p) {
    volatile _MM_array_ptr_Rep *mm_array_ptr_ptr = (_MM_array_ptr_Rep *)&p;
    *(mm_array_ptr_ptr->lock_ptr) = 0;
    free(mm_array_ptr_ptr->p - LOCK_SIZE - HEAP_PADDING);
}


//
// Function: _getptr_mm()
//
// This function extracts the inner raw pointer to the start of a struct
// from an mm_ptr<T>.
// The motivation of creating this function is that some code tris to
// cast an mm_ptr<T> to a raw pointer to another type of struct,
// e.g., the loadtree() function in the bh benchmark of the Olden
// benchmark suite.
//
// However, this is a danger operation. Unless a programmer is completely
// sure that doing so is safe, this function should be avoided.
//
// Maybe we should implement this as a compiler intrinsic?
//
// @param p - the safe pointer whose inner raw pointer to be extracted.
//
for_any(T) void *_getptr_mm(mm_ptr<T> p) {
    return ((_MM_ptr_Rep *)&p)->p;
}

//
// This function extracts the inner raw pointer to the start of a struct
// from an mm_array_ptr<T>.
//
for_any(T) void *_getptr_mm_array(mm_array_ptr<T> p) {
    return ((_MM_array_ptr_Rep *)&p)->p;
}
