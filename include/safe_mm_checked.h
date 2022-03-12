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
#define _GETPTR(T, p) ((T *)(p))
#define _GETARRAYPTR(T, p) ((T *)p)
#define _GETCHARPTR(p) ((char *)(p))

/* These macros provide convenience for programmers to type a little less. */
#define MM_ALLOC(T) mm_alloc<T>(sizeof(T))
#define MM_ARRAY_ALLOC(T, n) mm_array_alloc<T>(sizeof(T) * n)
#define MM_REALLOC(T, p, n) mm_array_realloc<T>(p, n)
#define MM_CALLOC(s, T) mm_calloc<T>(s, sizeof(T))
#define MM_SINGLE_CALLOC(T) mm_single_calloc<T>(sizeof(T))
#define MM_FREE(T, p) mm_free<T>(p)
#define MM_ARRAY_FREE(T, p) mm_array_free<T>(p)
#define MM_CHECKED(T, p) mm_checked<T>(p);
#define MM_ARRAY_CHECKED(T, p) mmarray_checked<T>(p);

for_any(T) mm_ptr<T> mm_alloc(size_t size);
for_any(T) void mm_free(mm_ptr<const T> const p);

/* mmsafe version of heap allocators and free() */
for_any(T) mm_array_ptr<T> mm_array_alloc(size_t array_size);
for_any(T) mm_array_ptr<T> mm_array_realloc(mm_array_ptr<T> p, size_t size);
for_any(T) mm_array_ptr<T> mm_calloc(size_t nmemb, size_t size);
for_any(T) mm_ptr<T> mm_single_calloc(size_t size);
for_any(T) void mm_array_free(mm_array_ptr<const T> const p);

/* mmsafe strdup/strndup */
mm_array_ptr<char> mm_strdup(mm_array_ptr<char> p);
mm_array_ptr<char> mm_strdup_from_raw(const char *p);

/* Extract the raw pointer from a checked pointer. */
/* Deprecated */
for_any(T) void *_getptr_mm(mm_ptr<const T> const p);
for_any(T) void *_getptr_mm_array(mm_array_ptr<const T> const p);

for_any(T) mm_ptr<T> create_invalid_mm_ptr(uint64_t val);

for_any(T) mm_array_ptr<T> mmptr_to_mmarrayptr(mm_ptr<T> p);
for_any(T) mm_ptr<T> mmarrayptr_to_mmptr(mm_array_ptr<T> p);

for_any(T) void _setptr_mm_array(mm_array_ptr<const T> *p, char *new_p);
/* Create an mmsafe pointer based on an existing checked pointer. */
for_any(T) mm_array_ptr<T> _create_mm_array_ptr(mm_array_ptr<T> p, char *new_p);

/* Mark a checked pointer to be valid */
for_any(T) void mm_checked(mm_ptr<T> p);
for_any(T) void mmarray_checked(mm_array_ptr<T> p);

/* Marshaling an array of mm_array_ptr to an array of raw pointers. */
for_any(T) void **_marshal_shared_array_ptr(mm_array_ptr<mm_array_ptr<T>> p);

#endif
