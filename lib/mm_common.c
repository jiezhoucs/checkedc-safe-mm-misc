/* mm_common.c - Helper functions that generate mmsafe pointers.
 *
 * Maybe we should combine this file with mm_libc.c.
 * */

#include "safe_mm_checked.h"
#include <string.h>

/*
 * Function: mm_memdup()
 *
 * Duplicates size byte(s) starting from a given pointer.
 * */
mm_array_ptr<void> mm_memdup(mm_array_ptr<void> src, size_t len) {
  mm_array_ptr<void> buffer = MM_ARRAY_ALLOC(void, len);
  if (!buffer) return NULL;

  memcpy(_GETARRAYPTR(void, buffer), _GETARRAYPTR(void, src), len);

  return buffer;
}
