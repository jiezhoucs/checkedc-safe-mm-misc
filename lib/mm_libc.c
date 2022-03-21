/**
 * MM safe version of common libc functions that needs special handling to
 * return an mmsafe pointer.
 * */

#include "safe_mm_checked.c"

/* A helper function  for mm_strchr, mm_strchr, etc.
 * See the comment of  _create_mm_array_ptr() in safe_mm_checked.c for details.
 *
 * TODO: Remove _create_mm_array_ptr().
 * */
static mm_array_ptr<char>
_create_mm_array_ptr_char(mm_array_ptr<const char> p, char *new_p) {
    _MMSafe_ptr_Rep *base_safeptr_ptr = (_MMSafe_ptr_Rep *)&p;
    _MMSafe_ptr_Rep new_safeptr = {
        new_p,
        base_safeptr_ptr->key_offset + (new_p - (char *)(base_safeptr_ptr->p))
  };

  return *((mm_array_ptr<char> *)&new_safeptr);
}


/* strchr */
mm_array_ptr<char> mm_strchr(mm_array_ptr<const char> p, int c) {
    char *new_p = strchr(_GETCHARPTR(p), c);
    return _create_mm_array_ptr_char(p, new_p);
}

/* strpbrk */
mm_array_ptr<char> mm_strpbrk(mm_array_ptr<const char> p, const char *accept) {
    char *new_p = strpbrk(_GETCHARPTR(p), accept);
    return _create_mm_array_ptr_char(p, new_p);
}

/* strstr() */
mm_array_ptr<char> mm_strstr(mm_array_ptr<const char> p, const char *needle) {
  char *new_p = strstr(_GETCHARPTR(p), needle);
  return _create_mm_array_ptr_char(p, new_p);
}
