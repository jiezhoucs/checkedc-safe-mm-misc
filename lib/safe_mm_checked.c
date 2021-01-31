/*
 * This files defines customized memory allocators and deallocators
 * for memory objects pointed by mm_ptr and mm_array_ptr.
 *
 * Note that the current implementation assumes we adopt a 32-32 key-offset
 * metadata design for mm_ptr. In the paper we say that we support two
 * key-offset schemes: 32-32 and 40-24. Also it is not totally clear to us
 * which is faster key-offset or offset-key. For the 32-32 option it might
 * be the same but the key-offset one might be a little faster for
 * the 40-24 one because the constant used in an "and" instruction can be
 * hardcoded in the instruction instead of loading from another register.
 *
 * */

#include "safe_mm_checked.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define LOCK_SIZE 8  // It is actually 4 bytes in current implementation.
#define HEAP_PADDING 8

// A helper struct that has the same inner structure as an mm_ptr.
// It is used to help create an mm_ptr.
typedef struct {
  void *p;
  uint64_t key_offset;
} _MM_ptr_Rep;


// A helper struct that has the "same" inner structure as a mm_array_ptr.
// It is used to help create a _MM_array_ptr.
typedef struct {
  void *p;
  uint64_t key;
  uint64_t *lock_ptr;  // pointer to the lock.
} _MM_array_ptr_Rep;


// Our current implementation uses a 32-bit key. We may need to change it
// to 40-bit key later.
uint32_t key = 1;

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
for_any(T) mm_ptr<T> mm_alloc(unsigned long size) {
    // We need the HEAP_PADDING to ensure that mm_ptr inside a struct
    // is aligned by 16 bytes.
    // See this issue for the reason: https://github.com/jzhou76/checkedc-llvm/issues/2
    void *raw_ptr = malloc(size + HEAP_PADDING + LOCK_SIZE);

    // Generate a random number as the key.
    // FIXME: replace the naive rand() function with a robust random
    // number generator.
    uint32_t new_key = key++;
    // The lock is located right before the first field of the referent.
    raw_ptr += HEAP_PADDING;
    *((uint32_t *)(raw_ptr)) = new_key;

    // Create a helper struct.
    _MM_ptr_Rep safe_ptr = { .p = raw_ptr + LOCK_SIZE, .key_offset = new_key };
    // Move the key to the highest 32 bits and make the offset 0.
    safe_ptr.key_offset <<= 32;

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
// Function: mm_array_realloc()
//
// This is the mm-safe version of realloc() for arrays.
//
// Note:
// Now we assume that if the re-allocated "new" object starts from the
// same starting address of the "old" object, then they are the same object
// and hence the "new" object uses the same lock as the "old" one.
// If the new object is really re-allocated and starts from somewhere else,
// the old one would be freed and we need create a new lock for the new object.
//
__attribute__ ((noinline))
for_any(T) mm_array_ptr<T> mm_array_realloc(mm_array_ptr<T> p, unsigned long size) {
    // Get the original raw pointer.
    void * old_raw_ptr = ((_MM_array_ptr_Rep *)&p)->p;
    old_raw_ptr = old_raw_ptr - LOCK_SIZE - HEAP_PADDING;

    void *new_raw_ptr = realloc(old_raw_ptr, size);
    if (new_raw_ptr == old_raw_ptr) {
        return p;
    }

    // Use a new key for the new object.
    uint64_t new_key = key++;
    new_raw_ptr += HEAP_PADDING;
    *((uint64_t *)new_raw_ptr) = new_key;

    _MM_array_ptr_Rep safe_ptr = {
        .p = new_raw_ptr + LOCK_SIZE, .key = new_key, .lock_ptr = new_raw_ptr
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
  *(uint32_t *)lock_ptr = 0;

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
