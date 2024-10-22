#ifndef HEADER_CURL_TOOL_FORMPARSE_H
#define HEADER_CURL_TOOL_FORMPARSE_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2020, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include "tool_setup.h"

#include <safe_mm_checked.h>

/* Private structure for mime/parts. */

typedef enum {
  TOOLMIME_NONE = 0,
  TOOLMIME_PARTS,
  TOOLMIME_DATA,
  TOOLMIME_FILE,
  TOOLMIME_FILEDATA,
  TOOLMIME_STDIN,
  TOOLMIME_STDINDATA
} toolmimekind;

struct tool_mime {
  /* Structural fields. */
  toolmimekind kind;            /* Part kind. */
  mm_ptr<struct tool_mime> parent;     /* Parent item. */
  mm_ptr<struct tool_mime> prev;       /* Previous sibling (reverse order link). */
  /* Common fields. */
  mm_array_ptr<const char> data;             /* Actual data or data filename. */
  mm_array_ptr<const char> name;             /* Part name. */
  mm_array_ptr<const char> filename;         /* Part's filename. */
  mm_array_ptr<const char> type;             /* Part's mime type. */
  mm_array_ptr<const char> encoder;          /* Part's requested encoding. */
  struct curl_slist *headers;   /* User-defined headers. */
  /* TOOLMIME_PARTS fields. */
  mm_ptr<struct tool_mime> subparts;   /* Part's subparts. */
  /* TOOLMIME_STDIN/TOOLMIME_STDINDATA fields. */
  curl_off_t origin;            /* Stdin read origin offset. */
  curl_off_t size;              /* Stdin data size. */
  curl_off_t curpos;            /* Stdin current read position. */
  struct GlobalConfig *config;  /* For access from callback. */
};

size_t tool_mime_stdin_read(char *buffer,
                            size_t size, size_t nitems, void *arg);
int tool_mime_stdin_seek(void *instream, curl_off_t offset, int whence);

int formparse(mm_ptr<struct OperationConfig> config,
              mm_array_ptr<const char> input,
              mm_ptr<mm_ptr<struct tool_mime>> mimeroot,
              mm_ptr<mm_ptr<struct tool_mime>> mimecurrent,
              bool literal_value);
CURLcode tool2curlmime(CURL *curl, mm_ptr<struct tool_mime> m, curl_mime **mime);
void tool_mime_free(mm_ptr<struct tool_mime> mime);

#endif /* HEADER_CURL_TOOL_FORMPARSE_H */
