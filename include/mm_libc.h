#ifndef _SAFE_MM_LIBC_H
#define _SAFE_MM_LIBC_H

#include "safe_mm_checked.h"

/* string utilities */
#define mm_strlen(p) strlen((_GETCHARPTR(p)))
#define mm_strcmp(s1,s2) strcmp(_GETCHARPTR(s1),_GETCHARPTR(s2))
#define mm_strncmp(s1,s2,n) strncmp(_GETCHARPTR(s1),_GETCHARPTR(s2),n)
#define mm_strcpy(d,s) strcpy(_GETCHARPTR(d),_GETCHARPTR(s))
#define mm_strncpy(d,s,n) strncpy(_GETCHARPTR(d),_GETCHARPTR(s),n)
#define mm_memcpy(d,s,n) memcpy(_GETCHARPTR(d),_GETCHARPTR(s),n)
#define mm_memmove(d,s,n) memmove(_GETCHARPTR(d),_GETCHARPTR(s),n)
#define mm_memcmp(s1,s2,n) memcmp(_GETCHARPTR(s1),_GETCHARPTR(s2),n)
#define mm_memset(s,c,n) memset(_GETCHARPTR(s),c,n)
#define mm_strcspn(s,r) strcspn(_GETCHARPTR(s),_GETCHARPTR(r))
/* file utilities */
#define mm_fopen(p,m) fopen(_GETCHARPTR(p),m)
#define mm_open(p,f) open(_GETCHARPTR(p),f)
#define mm_stat(p,s) stat(_GETCHARPTR(p),s)
#define mm_unlink(p) unlink(_GETCHARPTR(p))
#define mm_write(f,b,c) write(f,_GETCHARPTR(b),c)
#define mm_fwrite(p,size,n,stream) fwrite(_GETCHARPTR(p),size,n,stream)

mm_array_ptr<char> mm_strchr(mm_array_ptr<const char> p, int c);
mm_array_ptr<char> mm_strrchr(mm_array_ptr<const char> p, int c);
mm_array_ptr<char> mm_strstr(mm_array_ptr<const char> p, const char *needle);
mm_array_ptr<char> mm_strtok(mm_array_ptr<char> str, const char *delim,
                             mm_array_ptr<char> ostr);
mm_array_ptr<char> mm_strtok_r(mm_array_ptr<char> str, const char *delim,
                               char **saveptr, mm_array_ptr<char> ostr);
mm_array_ptr<char> mm_memchr(mm_array_ptr<const char> s, int c, size_t n);
mm_array_ptr<char> mm_memrchr(mm_array_ptr<const char> s, int c, size_t n);

void mm_qsort(mm_array_ptr<mm_ptr<void>> base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *));

unsigned long int mm_strtoul(mm_array_ptr<const char> nptr, mm_array_ptr<char> *endptr, int base);
long int mm_strtol(mm_array_ptr<const char> nptr, mm_array_ptr<char> *endptr, int base);
double mm_strtod(mm_array_ptr<const char> nptr, mm_array_ptr<char> *endptr);
#endif
