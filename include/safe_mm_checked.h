/*
 * Header file for the two customized memory allocators for heap struct and
 * ararays pointer to by our new mm-safe pointers.
 * pointer
 */

#ifndef _SAFE_MM_CHECKED_H
#define _SAFE_MM_CHECKED_H

#include <stdlib.h>
#include <stdint.h>
#include "stdchecked.h"

/* Extract the raw pointer from a checked pointer. */
#define _GETPTR(T, p) _getptr_mm<T>(p)
#define _GETARRAYPTR(T, p) _getptr_mm_array<T>(p)

for_any(T) mm_ptr<T> mm_alloc(size_t size);
for_any(T) void mm_free(mm_ptr<T> p);

for_any(T) mm_array_ptr<T> mm_array_alloc(size_t array_size);
for_any(T) mm_array_ptr<T> mm_array_realloc(mm_array_ptr<T> p, size_t size);
for_any(T) void mm_array_free(mm_array_ptr<T> p);

/* Extract the raw pointer from a checked pointer. */
for_any(T) void *_getptr_mm(mm_ptr<T> p);
for_any(T) void *_getptr_mm_array(mm_array_ptr<T> p);

for_any(T) mm_ptr<T> create_invalid_mm_ptr(uint64_t val);

for_any(T) mm_array_ptr<T> mmptr_to_mmarrayptr(mm_ptr<T> p);
for_any(T) mm_ptr<T> mmarrayptr_to_mmptr(mm_array_ptr<T> p);

for_any(T) void _setptr_mm_array(mm_array_ptr<const T> *p, char *new_p);
#endif
