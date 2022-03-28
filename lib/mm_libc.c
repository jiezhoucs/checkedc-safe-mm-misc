/**
 * MM safe version of common libc functions that needs special handling to
 * return an mmsafe pointer.
 * */

#include "safe_mm_checked.h"
#include <string.h>

#ifdef MM_DEBUG
#include <stdio.h>
#endif

// A helper struct that has the same inner structure as an mmsafe ptr.
typedef struct {
  void *p;
  uint64_t key_offset;
} _MMSafe_ptr_Rep;

/* A helper function  for mm_strchr, mm_strchr, etc.
 * See the comment of  _create_mm_array_ptr() in safe_mm_checked.c for details.
 *
 * */
static mm_array_ptr<char>
_create_mm_array_ptr_char(mm_array_ptr<const char> p, char *new_p) {
    if (new_p == NULL) return NULL;

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

/* strrchr */
mm_array_ptr<char> mm_strrchr(mm_array_ptr<const char> p, int c) {
    char *new_p = strrchr(_GETCHARPTR(p), c);
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

/* memchr() */
mm_array_ptr<char> mm_memchr(mm_array_ptr<const char> s, int c, size_t n) {
  char *ret_p = memchr(_GETPTR(char, s), c, n);
  return _create_mm_array_ptr_char(s, ret_p);
}

/* memrchr() */
mm_array_ptr<char> mm_memrchr(mm_array_ptr<const char> s, int c, size_t n) {
  char *ret_p = memrchr(_GETPTR(char, s), c, n);
  return _create_mm_array_ptr_char(s, ret_p);
}

/* strtok()
 *
 * The last argument is the pointer to the original string. It is used to
 * compute the new mm_array_ptr for the non-first-time call(s) to the fn.
 * */
mm_array_ptr<char> mm_strtok(mm_array_ptr<char> str, const char *delim,
                             mm_array_ptr<char> ostr) {
  char *ret_p = strtok(_GETCHARPTR(str), delim);
  if (ret_p == NULL) return NULL;

  if (str != NULL) {
    // First call.
    return _create_mm_array_ptr_char(str, ret_p);
  } else {
    // Subsequent call(s).
    return _create_mm_array_ptr_char(ostr, ret_p);
  }
}

/* strtok_r() */
mm_array_ptr<char> mm_strtok_r(mm_array_ptr<char> str, const char *delim,
                               char **saveptr,
                               mm_array_ptr<char> ostr) {
  char *ret_p = strtok_r(_GETCHARPTR(str), delim, saveptr);
  if (ret_p == NULL) return NULL;

  if (str != NULL) {
    // First call.
    return _create_mm_array_ptr_char(str, ret_p);
  } else {
    // Subsequent call(s).
    return _create_mm_array_ptr_char(ostr, ret_p);
  }
}

/* qsort()
 *
 * TODO? Should we make this accepts pointer of a generic type?
 * */
void mm_qsort(mm_array_ptr<mm_ptr<void>> base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *)) {
    void **raw_base = _marshal_mm_ptr<void>(base, nmemb);
    qsort(raw_base, nmemb, size, compar);

    // Reorganize the original array of mm_ptr.
    for (unsigned i = 0; i < nmemb; i++) {
      void *p_raw = raw_base[i];
      for (unsigned j = i; j < nmemb; j++) {
        void *p_mmsafe = (void *)(base[j]);
        if (p_mmsafe == p_raw) {
          // Swap the two mm_ptr.
          mm_ptr<void> tmp = base[i];
          base[i] = base[j];
          base[j] = tmp;
        }
      }
    }
    free(raw_base);
}

/* strtoul() */
unsigned long int mm_strtoul(mm_array_ptr<const char> nptr,
                             mm_array_ptr<char> *endptr, int base) {
    unsigned long int result = strtoul(_GETCHARPTR(nptr), (char**)endptr, base);
    if (endptr != NULL) {
        *endptr = _create_mm_array_ptr_char(nptr, _GETCHARPTR(*endptr));
    }
    return result;
}

/* strtol() */
long int mm_strtol(mm_array_ptr<const char> nptr, mm_array_ptr<char> *endptr, int base) {
    long int result = strtol(_GETCHARPTR(nptr), (char**)endptr, base);
    if (endptr != NULL) {
        *endptr = _create_mm_array_ptr_char(nptr, _GETCHARPTR(*endptr));
    }
    return result;
}

double mm_strtod(mm_array_ptr<const char> nptr, mm_array_ptr<char> *endptr) {
    double result = strtod(_GETCHARPTR(nptr), (char**)endptr);
    if (endptr != NULL) {
        *endptr = _create_mm_array_ptr_char(nptr, _GETCHARPTR(*endptr));
    }
    return result;
}
