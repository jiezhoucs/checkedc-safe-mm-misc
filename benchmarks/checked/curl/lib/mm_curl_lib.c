#include "mm_curl_lib.h"
#include "curl_memory.h"
#include "memdebug.h"

#define MIN_FIRST_ALLOC 32

#define DYNINIT 0xbee51da /* random pattern */

/*
 * Init a dynbuf struct.
 */
void mm_Curl_dyn_init(mm_ptr<struct dynbuf> s, size_t toobig)
{
  DEBUGASSERT(s);
  DEBUGASSERT(toobig);
  s->bufr = NULL;
  s->leng = 0;
  s->allc = 0;
  s->toobig = toobig;
#ifdef DEBUGBUILD
  s->init = DYNINIT;
#endif
}

/*
 * free the buffer and re-init the necessary fields. It doesn't touch the
 * 'init' field and thus this buffer can be reused to add data to again.
 */
void mm_Curl_dyn_free(mm_ptr<struct dynbuf> s)
{
  DEBUGASSERT(s);
  MM_curl_free(char, s->bufr);
  s->leng = s->allc = 0;
}

/*
 * Store/append an chunk of memory to the dynbuf.
 */
static CURLcode dyn_nappend(mm_ptr<struct dynbuf> s,
                            mm_array_ptr<const unsigned char> mem, size_t len)
{
  size_t indx = s->leng;
  size_t a = s->allc;
  size_t fit = len + indx + 1; /* new string + old string + zero byte */

  /* try to detect if there's rubbish in the struct */
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(s->toobig);
  DEBUGASSERT(indx < s->toobig);
  DEBUGASSERT(!s->leng || s->bufr);

  if(fit > s->toobig) {
    mm_Curl_dyn_free(s);
    return CURLE_OUT_OF_MEMORY;
  }
  else if(!a) {
    DEBUGASSERT(!indx);
    /* first invoke */
    if(fit < MIN_FIRST_ALLOC)
      a = MIN_FIRST_ALLOC;
    else
      a = fit;
  }
  else {
    while(a < fit)
      a *= 2;
  }

  if(a != s->allc) {
    /* this logic is not using Curl_saferealloc() to make the tool not have to
       include that as well when it uses this code */
    mm_array_ptr<char> p = mm_array_realloc<char>(s->bufr, a);
    if(!p) {
      mm_Curl_safefree(char, s->bufr);
      s->leng = s->allc = 0;
      return CURLE_OUT_OF_MEMORY;
    }
    s->bufr = p;
    s->allc = a;
  }

  if(len)
    mm_memcpy(_GETCHARPTR(&s->bufr[indx]), mem, len);
  s->leng = indx + len;
  s->bufr[s->leng] = 0;
  return CURLE_OK;
}

CURLcode mm_Curl_dyn_addn(mm_ptr<struct dynbuf> s, mm_array_ptr<const void> mem,
                          size_t len) {
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  return dyn_nappend(s, (mm_array_ptr<unsigned char>)mem, len);
}

CURLcode mm_Curl_dyn_add(mm_ptr<struct dynbuf> s, mm_array_ptr<const char> str) {
  size_t n = mm_strlen(str);
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  return dyn_nappend(s, (mm_array_ptr<unsigned char>)str, n);
}

/*
 * Clears the string, keeps the allocation. This can also be called on a
 * buffer that already was freed.
 */
void mm_Curl_dyn_reset(mm_ptr<struct dynbuf> s)
{
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  if(s->leng)
    s->bufr[0] = 0;
  s->leng = 0;
}

/*
 * Append a string vprintf()-style
 */
CURLcode mm_Curl_dyn_vaddf(mm_ptr<struct dynbuf> s, const char *fmt, va_list ap)
{
#ifdef BUILDING_LIBCURL
  int rc;
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  rc = Curl_dyn_vprintf(_GETPTR(struct dynbuf, s), fmt, ap);

  if(!rc)
    return CURLE_OK;
#else
  char *str;
  str = vaprintf(fmt, ap); /* this allocs a new string to append */

  if(str) {
    CURLcode result = mm_dyn_nappend(s, (mm_ptr<unsigned char>)str, strlen(str));
    free(str);
    return result;
  }
  /* If we failed, we cleanup the whole buffer and return error */
  mm_Curl_dyn_free(s);
#endif
  return CURLE_OUT_OF_MEMORY;
}


/*
 * Append a string printf()-style
 */
CURLcode mm_Curl_dyn_addf(mm_ptr<struct dynbuf> s, const char *fmt, ...)
{
  CURLcode result;
  va_list ap;
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  va_start(ap, fmt);
  result = mm_Curl_dyn_vaddf(s, fmt, ap);
  va_end(ap);
  return result;
}

/*
 * Returns a pointer to the buffer.
 */
mm_array_ptr<char> mm_Curl_dyn_ptr(mm_ptr<const struct dynbuf> s)
{
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  return s->bufr;
}

/*
 * Returns the length of the buffer.
 */
size_t mm_Curl_dyn_len(mm_ptr<const struct dynbuf> s)
{
  DEBUGASSERT(s);
  DEBUGASSERT(s->init == DYNINIT);
  DEBUGASSERT(!s->leng || s->bufr);
  return s->leng;
}
