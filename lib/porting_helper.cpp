//
// Library that assists porting programs.
//
// During porting a program, it is possible that two types of illegal free happen:
//
// 1. mm_free() is called to free a pointer return by an original malloc.
// 2. Original free() is called to free the raw pointer of an mmsafe pointer.
//
// The first situation rises when a union contains both mmsafe pointer()s and
// raw pointers, but a free() is used to accept an mmsafe pointer type to
// free any pointer.
//
// The second situation happens when we extracts the raw pointer of an mmsafe
// pointer and assigns to a raw pointer that has not beend ported.
//

#include "porting_helper.h"
#include <unordered_set>

using namespace std;

unordered_set<void*> mmsafe_ptrs;

#if defined __cplusplus
extern "C" {
#endif


// Check if a pointer is the raw ptr of an mmsafe pointer.
bool is_an_mmsafe_ptr(void *p) {
  return mmsafe_ptrs.find(p) != mmsafe_ptrs.end();
}

// Insert the raw pointer of an mmsafe pointer in the mmsafe pointer set.
void insert_mmsafe_ptr(void *p) {
  mmsafe_ptrs.insert(p);
}

// Erase the raw pointer of an mmsafe pointer from the mmsafe pointer set.
void erase_mmsafe_ptr(void *p) {
  mmsafe_ptrs.erase(p);
}

// Use this to replace original free() calls. This will handle the case when
// the raw pointer of an mmsafe pointer is passed to free().
void uncertain_free(void *p) {
  if (is_an_mmsafe_ptr(p)) {
    // From calling free() on the raw pointer of an mmsafe pointer.
    // Invalidate the lock and then do the real free.
    *((uint32_t *)((char *)p - 8)) = 0;
    erase_mmsafe_ptr(p);
    free((char *)p - 16);
  } else {
    free(p);
  }
}

#if defined __cplusplus
}
#endif
