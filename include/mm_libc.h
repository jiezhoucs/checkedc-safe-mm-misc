#ifndef _SAFE_MM_LIBC_H
#define _SAFE_MM_LIBC_H

#include "safe_mm_checked.h"

#define mm_strlen(p) strlen((_GETCHARPTR(p)))
#define mm_strcmp(s1,s2) strcmp(_GETCHARPTR(s1),_GETCHARPTR(s2))
#define mm_strncmp(s1,s2,n) strncmp(_GETCHARPTR(s1),_GETCHARPTR(s2),n)
#define mm_strcpy(d,s) strcpy(_GETCHARPTR(d),_GETCHARPTR(s))
#define mm_memcpy(d,s,n) memcpy(_GETCHARPTR(d),_GETCHARPTR(s),n)

mm_array_ptr<char> mm_strtok_r(mm_array_ptr<char> str, const char *delim,
                               char **saveptr, mm_array_ptr<char> ostr);
mm_array_ptr<char> mm_memchr(mm_array_ptr<const char> s, int c, size_t n);
mm_array_ptr<char> mm_memrchr(mm_array_ptr<const char> s, int c, size_t n);

#endif
