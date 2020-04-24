//
// Header file for the two customized memory allocators for heap struct and
// ararays pointer to by our new mm-safe pointers.
// pointer

#ifndef _SAFE_MM_CHECKED_H
#define _SAFE_MM_CHECKED_H

#include "stdchecked.h"

for_any(T) mm_ptr<T> mm_alloc(unsigned long struct_size);

for_any(T) mm_array_ptr<T> mm_array_alloc(unsigned long array_size);

for_any(T) void mm_free(mm_ptr<T> p);

for_any(T) void mm_array_free(mm_array_ptr<T> p);

#endif
