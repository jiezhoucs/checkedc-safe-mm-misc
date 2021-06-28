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
#define HIGH32BITS_MASK 0x00000000ffffffff

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
uint32_t key = 3;

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
for_any(T) mm_ptr<T> mm_alloc(size_t size) {
    // We need the HEAP_PADDING to ensure that mm_ptr inside a struct
    // is aligned by 16 bytes.
    // See this issue for the reason: https://github.com/jzhou76/checkedc-llvm/issues/2
    void *raw_ptr = malloc(size + HEAP_PADDING + LOCK_SIZE);
    if (raw_ptr == NULL) return NULL;

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
for_any(T) mm_array_ptr<T> mm_array_alloc(size_t array_size) {
  void *raw_ptr = malloc(array_size + LOCK_SIZE + HEAP_PADDING);
  if (raw_ptr == NULL) return NULL;

#ifdef DEBUG
  printf("[DEBUG] Allocated: %p\n", raw_ptr);
#endif

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
for_any(T) mm_array_ptr<T> mm_array_realloc(mm_array_ptr<T> p, size_t size) {
    // Get the original raw pointer.
    void * old_raw_ptr = ((_MM_array_ptr_Rep *)&p)->p;
    old_raw_ptr = old_raw_ptr - LOCK_SIZE - HEAP_PADDING;

    void *new_raw_ptr = realloc(old_raw_ptr, size);
    if (new_raw_ptr == old_raw_ptr) {
        return p;
    }

    // The new object is placed in a different location and the old one
    // is freed. The old object's lock needs to be invalidated.
    *((uint64_t *)(old_raw_ptr + HEAP_PADDING)) = 0;

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
// The motivation of creating this function is that some code tries to
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


/*
 * Function: create_invalid_mm_ptr()
 *
 * This function creates an invalid mm_ptr with the raw pointer value
 * set to the argument ptr_val.
 *
 * The motivation of having such a function is that sometimes a function
 * with a return type of pointer may want to return different
 * invalid pointers for different erroneous situations. One example is the
 * fdwatch_get_next_client_data() function of thttpd. It returns "(void *)0"
 * and "(void *)-1" for different errors.
 * */
for_any(T) mm_ptr<T> create_invalid_mm_ptr(uint64_t ptr_val) {
  _MM_ptr_Rep mmptr;
  mmptr.p = (void *)ptr_val;
  mm_ptr<T> *mm_ptr_ptr = (mm_ptr<T> *)&mmptr;
  return *mm_ptr_ptr;
}

/**
 * Function: mmptr_to_mmarrayptr()
 *
 * This function converts an mmptr to an mm_array_ptr. Our design generates
 * an mm_ptr for the address-of operator on an item of an array pointed
 * by an mm_array_ptr (https://github.com/jzhou76/checkedc-clang/commit/471fa1e721c8e72640c81c7c21b0c09a74af7346).
 * However, sometimes the result pointer may be used to do pointer arithmetic.
 * For example, function really_clear_connection() of thttpd computes the
 * inteval between the element pointed by an mm_ptr and the beginning of the
 * array that contains the element (https://github.com/jzhou76/checkedc-safe-mm-misc/blob/master/benchmarks/baseline/thttpd-2.29/thttpd.c#L2053)
 *
 * */
for_any(T) mm_array_ptr<T> mmptr_to_mmarrayptr(mm_ptr<T> p) {
    _MM_ptr_Rep *mmptr_ptr = (_MM_ptr_Rep *)&p;
    _MM_array_ptr_Rep mmarrayptr;
    mmarrayptr.p = mmptr_ptr->p;
    mmarrayptr.key = (mmptr_ptr->key_offset >> 32) & HIGH32BITS_MASK;
    mmarrayptr.lock_ptr = mmptr_ptr->p -
        (mmptr_ptr->key_offset & HIGH32BITS_MASK) - LOCK_SIZE;
    mm_array_ptr<T> *mmarrayptr_ptr = (mm_array_ptr<T> *)&mmarrayptr;
    return *mmarrayptr_ptr;
}

/*
 * Function: mmarrayptr_to_mmptr()
 *
 * This function converts an mm_array_ptr to an mm_ptr. This function handles
 * the situation when a program uses the "->" operator to dereference
 * an mm_array_ptr.
 *
 * */
for_any(T) mm_ptr<T> mmarrayptr_to_mmptr(mm_array_ptr<T> p) {
    _MM_array_ptr_Rep *mmarrayptr_ptr = (_MM_array_ptr_Rep *)&p;
    _MM_ptr_Rep mmptr;
    mmptr.p = mmarrayptr_ptr->p;
    uint64_t key = mmarrayptr_ptr->key;
    uint64_t offset = mmarrayptr_ptr->p - (void *)mmarrayptr_ptr->lock_ptr + LOCK_SIZE;
    mmptr.key_offset = (key << 32) & offset;
    mm_ptr<T> *mmptr_ptr = (mm_ptr<T> *)&mmptr;
    return *mmptr_ptr;
}


#if 0
/*
 * Function: _setptr_mm()
 *
 * Do we need this function?
 *
 * */
for_any(T) void _setptr_mm(mm_ptr<const T> *p, char *new_p) {
}
#endif

/*
 * Function: _setptr_mm_array()
 *
 * This function updates an mm_array_ptr with a provided raw C pointer.
 * This is a very dangerous behavior but it is needed for interacting with
 * certain library code. For example, a program may call strtod() and then
 * use the endptr to update an existing pointer. (The parse_number_value()
 * function of parson (https://github.com/kgabis/parson/blob/master/parson.c#L862)
 * shows such an example.)
 * If the to-be-updated pointer is an mm_array_ptr and we would like to keep it
 * as a checked pointer, then we need this function.
 *
 * FIXME: The current implementation of mm_array_ptr has an individual
 * lock_addr field and thus the update of its raw C pointer does not require
 * updating any metadata. But in our new design the mm_array_ptr has an offset
 * subfield and it needs to be updated based on the new raw pointer.
 *
 * */
for_any(T) void _setptr_mm_array(mm_array_ptr<const T> *p, char *new_p) {
    *((char **)p) = new_p;
}
