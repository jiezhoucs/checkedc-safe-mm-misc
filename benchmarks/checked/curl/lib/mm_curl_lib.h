#ifndef MM_CURL_LIB_H
#define MM_CURL_LIB_H

#include "curl_setup.h"
#include <curl/mprintf.h>
#include "dynbuf.h"

#include <safe_mm_checked.h>

void mm_Curl_dyn_init(mm_ptr<struct dynbuf> s, size_t toobig);
void mm_Curl_dyn_free(mm_ptr<struct dynbuf> s);
CURLcode mm_Curl_dyn_addn(mm_ptr<struct dynbuf> s, mm_array_ptr<const void> mem, size_t len)
  WARN_UNUSED_RESULT;
CURLcode mm_Curl_dyn_add(mm_ptr<struct dynbuf> s, mm_array_ptr<const char> str)
  WARN_UNUSED_RESULT;
CURLcode mm_Curl_dyn_addf(mm_ptr<struct dynbuf> s, const char *fmt, ...)
  WARN_UNUSED_RESULT;
CURLcode mm_Curl_dyn_vaddf(mm_ptr<struct dynbuf> s, const char *fmt, va_list ap)
  WARN_UNUSED_RESULT;

void mm_Curl_dyn_reset(mm_ptr<struct dynbuf> s);
mm_array_ptr<char> mm_Curl_dyn_ptr(mm_ptr<const struct dynbuf> s);
size_t mm_Curl_dyn_len(mm_ptr<const struct dynbuf> s);
#endif
