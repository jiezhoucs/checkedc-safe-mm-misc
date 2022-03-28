/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2021, Daniel Stenberg, <daniel@haxx.se>, et al.
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

#include "strcase.h"

#define ENABLE_CURLX_PRINTF
/* use our own printf() functions */
#include "curlx.h"

#include "tool_cfgable.h"
#include "tool_convert.h"
#include "tool_msgs.h"
#include "tool_binmode.h"
#include "tool_getparam.h"
#include "tool_paramhlp.h"
#include "tool_formparse.h"

#include "memdebug.h" /* keep this as LAST include */

/* Macros to free const pointers. */
#define CONST_FREE(x)           free((void *) (x))
#define CONST_SAFEFREE(x)       Curl_safefree(*((void **) &(x)))

/* tool_mime functions. */
static mm_array_ptr<struct tool_mime> tool_mime_new(mm_ptr<struct tool_mime> parent,
                                       toolmimekind kind)
{
  mm_ptr<struct tool_mime> m = MM_SINGLE_CALLOC(struct tool_mime);

  if(m) {
    m->kind = kind;
    m->parent = parent;
    if(parent) {
      m->prev = parent->subparts;
      parent->subparts = m;
    }
  }
  return m;
}

static mm_array_ptr<struct tool_mime> tool_mime_new_parts(mm_ptr<struct tool_mime> parent)
{
  return tool_mime_new(parent, TOOLMIME_PARTS);
}

static mm_ptr<struct tool_mime> tool_mime_new_data(mm_ptr<struct tool_mime> parent,
                                            mm_array_ptr<const char> data)
{
  mm_ptr<struct tool_mime> m = NULL;

  data = mm_strdup(data);
  if(data) {
    m = tool_mime_new(parent, TOOLMIME_DATA);
    if(!m)
      CONST_FREE(data);
    else
      m->data = data;
  }
  return m;
}

static mm_ptr<struct tool_mime> tool_mime_new_filedata(mm_ptr<struct tool_mime> parent,
                                                mm_array_ptr<const char> filename,
                                                bool isremotefile,
                                                CURLcode *errcode)
{
  CURLcode result = CURLE_OK;
  mm_ptr<struct tool_mime> m = NULL;

  *errcode = CURLE_OUT_OF_MEMORY;
  if(mm_strcmp(filename, "-")) {
    /* This is a normal file. */
    filename = mm_strdup(filename);
    if(filename) {
      m = tool_mime_new(parent, TOOLMIME_FILE);
      if(!m)
        CONST_FREE(filename);
      else {
        m->data = filename;
        if(!isremotefile)
          m->kind = TOOLMIME_FILEDATA;
       *errcode = CURLE_OK;
      }
    }
  }
  else {        /* Standard input. */
    int fd = fileno(stdin);
    mm_array_ptr<char> data = NULL;
    curl_off_t size;
    curl_off_t origin;
    struct_stat sbuf;

    set_binmode(stdin);
    origin = ftell(stdin);
    /* If stdin is a regular file, do not buffer data but read it
       when needed. */
    if(fd >= 0 && origin >= 0 && !fstat(fd, &sbuf) &&
#ifdef __VMS
       sbuf.st_fab_rfm != FAB$C_VAR && sbuf.st_fab_rfm != FAB$C_VFC &&
#endif
       S_ISREG(sbuf.st_mode)) {
      size = sbuf.st_size - origin;
      if(size < 0)
        size = 0;
    }
    else {  /* Not suitable for direct use, buffer stdin data. */
      size_t stdinsize = 0;

      if(file2memory(&data, &stdinsize, stdin) != PARAM_OK) {
        /* Out of memory. */
        return m;
      }

      if(ferror(stdin)) {
        result = CURLE_READ_ERROR;
        mm_Curl_safefree(char, data);
        data = NULL;
      }
      else if(!stdinsize) {
        /* Zero-length data has been freed. Re-create it. */
        data = mm_strdup_from_raw("");
        if(!data)
          return m;
      }
      size = curlx_uztoso(stdinsize);
      origin = 0;
    }
    m = tool_mime_new(parent, TOOLMIME_STDIN);
    if(!m)
      mm_Curl_safefree(char, data);
    else {
      m->data = data;
      m->origin = origin;
      m->size = size;
      m->curpos = 0;
      if(!isremotefile)
        m->kind = TOOLMIME_STDINDATA;
      *errcode = result;
    }
  }
  return m;
}

void tool_mime_free(mm_ptr<struct tool_mime> mime)
{
  if(mime) {
    if(mime->subparts)
      tool_mime_free(mime->subparts);
    if(mime->prev)
      tool_mime_free(mime->prev);
    CONST_SAFEFREE(mime->name);
    CONST_SAFEFREE(mime->filename);
    CONST_SAFEFREE(mime->type);
    CONST_SAFEFREE(mime->encoder);
    CONST_SAFEFREE(mime->data);
    curl_slist_free_all(mime->headers);
    MM_FREE(struct tool_mime, mime);
  }
}


/* Mime part callbacks for stdin. */
size_t tool_mime_stdin_read(char *buffer,
                            size_t size, size_t nitems, void *arg)
{
  struct tool_mime *sip = (struct tool_mime *) arg;
  curl_off_t bytesleft;
  (void) size;  /* Always 1: ignored. */

  if(sip->size >= 0) {
    if(sip->curpos >= sip->size)
      return 0;  /* At eof. */
    bytesleft = sip->size - sip->curpos;
    if(curlx_uztoso(nitems) > bytesleft)
      nitems = curlx_sotouz(bytesleft);
  }
  if(nitems) {
    if(sip->data) {
      /* Return data from memory. */
      memcpy(buffer, _GETCHARPTR(sip->data) + curlx_sotouz(sip->curpos), nitems);
    }
    else {
      /* Read from stdin. */
      nitems = fread(buffer, 1, nitems, stdin);
      if(ferror(stdin)) {
        /* Show error only once. */
        if(sip->config) {
          warnf(sip->config, "stdin: %s\n", strerror(errno));
          sip->config = NULL;
        }
        return CURL_READFUNC_ABORT;
      }
    }
    sip->curpos += curlx_uztoso(nitems);
  }
  return nitems;
}

int tool_mime_stdin_seek(void *instream, curl_off_t offset, int whence)
{
  struct tool_mime *sip = (struct tool_mime *) instream;

  switch(whence) {
  case SEEK_CUR:
    offset += sip->curpos;
    break;
  case SEEK_END:
    offset += sip->size;
    break;
  }
  if(offset < 0)
    return CURL_SEEKFUNC_CANTSEEK;
  if(!sip->data) {
    if(fseek(stdin, (long) (offset + sip->origin), SEEK_SET))
      return CURL_SEEKFUNC_CANTSEEK;
  }
  sip->curpos = offset;
  return CURL_SEEKFUNC_OK;
}

/* Translate an internal mime tree into a libcurl mime tree. */

static CURLcode tool2curlparts(CURL *curl, mm_ptr<struct tool_mime> m,
                               curl_mime *mime)
{
  CURLcode ret = CURLE_OK;
  curl_mimepart *part = NULL;
  curl_mime *submime = NULL;
  mm_array_ptr<const char> filename = NULL;

  if(m) {
    ret = tool2curlparts(curl, m->prev, mime);
    if(!ret) {
      part = curl_mime_addpart(mime);
      if(!part)
        ret = CURLE_OUT_OF_MEMORY;
    }
    if(!ret) {
      filename = m->filename;
      switch(m->kind) {
      case TOOLMIME_PARTS:
        ret = tool2curlmime(curl, m, &submime);
        if(!ret) {
          ret = curl_mime_subparts(part, submime);
          if(ret)
            curl_mime_free(submime);
        }
        break;

      case TOOLMIME_DATA:
#ifdef CURL_DOES_CONVERSIONS
        /* Our data is always textual: convert it to ASCII. */
        {
          size_t size = strlen(m->data);
          /* Checked C: Omit changing the next line. */
          char *cp = malloc(size + 1);

          if(!cp)
            ret = CURLE_OUT_OF_MEMORY;
          else {
            memcpy(cp, m->data, size + 1);
            ret = convert_to_network(cp, size);
            if(!ret)
              ret = curl_mime_data(part, cp, CURL_ZERO_TERMINATED);
            free(cp);
          }
        }
#else
        ret = curl_mime_data(part, _GETCHARPTR(m->data), CURL_ZERO_TERMINATED);
#endif
        break;

      case TOOLMIME_FILE:
      case TOOLMIME_FILEDATA:
        ret = curl_mime_filedata(part, _GETCHARPTR(m->data));
        if(!ret && m->kind == TOOLMIME_FILEDATA && !filename)
          ret = curl_mime_filename(part, NULL);
        break;

      case TOOLMIME_STDIN:
        if(!filename)
          filename = "-";
        /* FALLTHROUGH */
      case TOOLMIME_STDINDATA:
        ret = curl_mime_data_cb(part, m->size,
                                (curl_read_callback) tool_mime_stdin_read,
                                (curl_seek_callback) tool_mime_stdin_seek,
                                NULL, _GETPTR(struct tool_mime, m));
        break;

      default:
        /* Other cases not possible in this context. */
        break;
      }
    }
    if(!ret && filename)
      ret = curl_mime_filename(part, _GETCHARPTR(filename));
    if(!ret)
      ret = curl_mime_type(part, _GETCHARPTR(m->type));
    if(!ret)
      ret = curl_mime_headers(part, m->headers, 0);
    if(!ret)
      ret = curl_mime_encoder(part, _GETCHARPTR(m->encoder));
    if(!ret)
      ret = curl_mime_name(part, _GETCHARPTR(m->name));
  }
  return ret;
}

CURLcode tool2curlmime(CURL *curl, mm_ptr<struct tool_mime> m, curl_mime **mime)
{
  CURLcode ret = CURLE_OK;

  *mime = curl_mime_init(curl);
  if(!*mime)
    ret = CURLE_OUT_OF_MEMORY;
  else
    ret = tool2curlparts(curl, m->subparts, *mime);
  if(ret) {
    curl_mime_free(*mime);
    *mime = NULL;
  }
  return ret;
}

/*
 * helper function to get a word from form param
 * after call get_parm_word, str either point to string end
 * or point to any of end chars.
 */
static mm_array_ptr<char> get_param_word(mm_ptr<struct OperationConfig> config,
    mm_array_ptr<char> *str, mm_array_ptr<char> *end_pos, char endchar)
{
  mm_array_ptr<char> ptr = *str;
  /* the first non-space char is here */
  mm_array_ptr<char> word_begin = ptr;
  mm_array_ptr<char> ptr2 = NULL;
  mm_array_ptr<char> escape = NULL;

  if(*ptr == '"') {
    ++ptr;
    while(*ptr) {
      if(*ptr == '\\') {
        if(ptr[1] == '\\' || ptr[1] == '"') {
          /* remember the first escape position */
          if(!escape)
            escape = ptr;
          /* skip escape of back-slash or double-quote */
          ptr += 2;
          continue;
        }
      }
      if(*ptr == '"') {
        bool trailing_data = FALSE;
        *end_pos = ptr;
        if(escape) {
          /* has escape, we restore the unescaped string here */
          ptr = ptr2 = escape;
          do {
            if(*ptr == '\\' && (ptr[1] == '\\' || ptr[1] == '"'))
              ++ptr;
            *ptr2++ = *ptr++;
          }
          while(ptr < *end_pos);
          *end_pos = ptr2;
        }
        ++ptr;
        while(*ptr && *ptr != ';' && *ptr != endchar) {
          if(!ISSPACE(*ptr))
            trailing_data = TRUE;
          ++ptr;
        }
        if(trailing_data)
          warnf(config->global, "Trailing data after quoted form parameter\n");
        *str = ptr;
        return word_begin + 1;
      }
      ++ptr;
    }
    /* end quote is missing, treat it as non-quoted. */
    ptr = word_begin;
  }

  while(*ptr && *ptr != ';' && *ptr != endchar)
    ++ptr;
  *str = *end_pos = ptr;
  return word_begin;
}

/* Append slist item and return -1 if failed. */
static int slist_append(struct curl_slist **plist, const char *data)
{
  struct curl_slist *s = curl_slist_append(*plist, data);

  if(!s)
    return -1;

  *plist = s;
  return 0;
}

/* Read headers from a file and append to list. */
static int read_field_headers(mm_ptr<struct OperationConfig> config,
                              mm_array_ptr<const char> filename, FILE *fp,
                              struct curl_slist **pheaders)
{
  size_t hdrlen = 0;
  size_t pos = 0;
  bool incomment = FALSE;
  int lineno = 1;
  char hdrbuf[999]; /* Max. header length + 1. */

  for(;;) {
    int c = getc(fp);
    if(c == EOF || (!pos && !ISSPACE(c))) {
      /* Strip and flush the current header. */
      while(hdrlen && ISSPACE(hdrbuf[hdrlen - 1]))
        hdrlen--;
      if(hdrlen) {
        hdrbuf[hdrlen] = '\0';
        if(slist_append(pheaders, hdrbuf)) {
          fprintf(config->global->errors,
                  "Out of memory for field headers!\n");
          return -1;
        }
        hdrlen = 0;
      }
    }

    switch(c) {
    case EOF:
      if(ferror(fp)) {
        fprintf(config->global->errors,
                "Header file %s read error: %s\n", _GETCHARPTR(filename), strerror(errno));
        return -1;
      }
      return 0;    /* Done. */
    case '\r':
      continue;    /* Ignore. */
    case '\n':
      pos = 0;
      incomment = FALSE;
      lineno++;
      continue;
    case '#':
      if(!pos)
        incomment = TRUE;
      break;
    }

    pos++;
    if(!incomment) {
      if(hdrlen == sizeof(hdrbuf) - 1) {
        warnf(config->global, "File %s line %d: header too long (truncated)\n",
              _GETCHARPTR(filename), lineno);
        c = ' ';
      }
      if(hdrlen <= sizeof(hdrbuf) - 1)
        hdrbuf[hdrlen++] = (char) c;
    }
  }
  /* NOTREACHED */
}

static int get_param_part(mm_ptr<struct OperationConfig> config, char endchar,
                          mm_array_ptr<char> *str, mm_array_ptr<char> *pdata,
                          mm_array_ptr<char> *ptype, mm_array_ptr<char> *pfilename,
                          mm_array_ptr<char> *pencoder, struct curl_slist **pheaders)
{
  mm_array_ptr<char> p = *str;
  mm_array_ptr<char> type = NULL;
  mm_array_ptr<char> filename = NULL;
  mm_array_ptr<char> encoder = NULL;
  mm_array_ptr<char> endpos = NULL;
  mm_array_ptr<char> tp = NULL;
  char sep;
  char type_major[128] = "";
  char type_minor[128] = "";
  mm_array_ptr<char> endct = NULL;
  struct curl_slist *headers = NULL;

  if(ptype)
    *ptype = NULL;
  if(pfilename)
    *pfilename = NULL;
  if(pheaders)
    *pheaders = NULL;
  if(pencoder)
    *pencoder = NULL;
  while(ISSPACE(*p))
    p++;
  tp = p;
  *pdata = get_param_word(config, &p, &endpos, endchar);
  /* If not quoted, strip trailing spaces. */
  if(*pdata == tp)
    while(endpos > *pdata && ISSPACE(endpos[-1]))
      endpos--;
  sep = *p;
  *endpos = '\0';
  while(sep == ';') {
    while(ISSPACE(*++p))
      ;

    if(!endct && checkprefix("type=", p)) {
      for(p += 5; ISSPACE(*p); p++)
        ;
      /* set type pointer */
      type = p;

      /* verify that this is a fine type specifier */
      if(2 != sscanf(_GETCHARPTR(type), "%127[^/ ]/%127[^;, \n]", type_major, type_minor)) {
        warnf(config->global, "Illegally formatted content-type field!\n");
        curl_slist_free_all(headers);
        return -1; /* illegal content-type syntax! */
      }

      /* now point beyond the content-type specifier */
      p = type + strlen(type_major) + strlen(type_minor) + 1;
      for(endct = p; *p && *p != ';' && *p != endchar; p++)
        if(!ISSPACE(*p))
          endct = p + 1;
      sep = *p;
    }
    else if(checkprefix("filename=", p)) {
      if(endct) {
        *endct = '\0';
        endct = NULL;
      }
      for(p += 9; ISSPACE(*p); p++)
        ;
      tp = p;
      filename = get_param_word(config, &p, &endpos, endchar);
      /* If not quoted, strip trailing spaces. */
      if(filename == tp)
        while(endpos > filename && ISSPACE(endpos[-1]))
          endpos--;
      sep = *p;
      *endpos = '\0';
    }
    else if(checkprefix("headers=", p)) {
      if(endct) {
        *endct = '\0';
        endct = NULL;
      }
      p += 8;
      if(*p == '@' || *p == '<') {
        mm_array_ptr<char> hdrfile = NULL;
        FILE *fp;
        /* Read headers from a file. */

        do {
          p++;
        } while(ISSPACE(*p));
        tp = p;
        hdrfile = get_param_word(config, &p, &endpos, endchar);
        /* If not quoted, strip trailing spaces. */
        if(hdrfile == tp)
          while(endpos > hdrfile && ISSPACE(endpos[-1]))
            endpos--;
        sep = *p;
        *endpos = '\0';
        fp = fopen(_GETCHARPTR(hdrfile), FOPEN_READTEXT);
        if(!fp)
          warnf(config->global, "Cannot read from %s: %s\n", _GETCHARPTR(hdrfile),
                strerror(errno));
        else {
          int i = read_field_headers(config, hdrfile, fp, &headers);

          fclose(fp);
          if(i) {
            curl_slist_free_all(headers);
            return -1;
          }
        }
      }
      else {
        mm_array_ptr<char> hdr = NULL;

        while(ISSPACE(*p))
          p++;
        tp = p;
        hdr = get_param_word(config, &p, &endpos, endchar);
        /* If not quoted, strip trailing spaces. */
        if(hdr == tp)
          while(endpos > hdr && ISSPACE(endpos[-1]))
            endpos--;
        sep = *p;
        *endpos = '\0';
        if(slist_append(&headers, _GETCHARPTR(hdr))) {
          fprintf(config->global->errors, "Out of memory for field header!\n");
          curl_slist_free_all(headers);
          return -1;
        }
      }
    }
    else if(checkprefix("encoder=", p)) {
      if(endct) {
        *endct = '\0';
        endct = NULL;
      }
      for(p += 8; ISSPACE(*p); p++)
        ;
      tp = p;
      encoder = get_param_word(config, &p, &endpos, endchar);
      /* If not quoted, strip trailing spaces. */
      if(encoder == tp)
        while(endpos > encoder && ISSPACE(endpos[-1]))
          endpos--;
      sep = *p;
      *endpos = '\0';
    }
    else if(endct) {
      /* This is part of content type. */
      for(endct = p; *p && *p != ';' && *p != endchar; p++)
        if(!ISSPACE(*p))
          endct = p + 1;
      sep = *p;
    }
    else {
      /* unknown prefix, skip to next block */
      mm_array_ptr<char> unknown = get_param_word(config, &p, &endpos, endchar);

      sep = *p;
      *endpos = '\0';
      if(*unknown)
        warnf(config->global, "skip unknown form field: %s\n", _GETCHARPTR(unknown));
    }
  }

  /* Terminate content type. */
  if(endct)
    *endct = '\0';

  if(ptype)
    *ptype = type;
  else if(type)
    warnf(config->global, "Field content type not allowed here: %s\n", _GETCHARPTR(type));

  if(pfilename)
    *pfilename = filename;
  else if(filename)
    warnf(config->global,
          "Field file name not allowed here: %s\n", _GETCHARPTR(filename));

  if(pencoder)
    *pencoder = encoder;
  else if(encoder)
    warnf(config->global,
          "Field encoder not allowed here: %s\n", _GETCHARPTR(encoder));

  if(pheaders)
    *pheaders = headers;
  else if(headers) {
    warnf(config->global,
          "Field headers not allowed here: %s\n", _GETCHARPTR(headers->data));
    curl_slist_free_all(headers);
  }

  *str = p;
  return sep & 0xFF;
}


/***************************************************************************
 *
 * formparse()
 *
 * Reads a 'name=value' parameter and builds the appropriate linked list.
 *
 * If the value is of the form '<filename', field data is read from the
 * given file.

 * Specify files to upload with 'name=@filename', or 'name=@"filename"'
 * in case the filename contain ',' or ';'. Supports specified
 * given Content-Type of the files. Such as ';type=<content-type>'.
 *
 * If literal_value is set, any initial '@' or '<' in the value string
 * loses its special meaning, as does any embedded ';type='.
 *
 * You may specify more than one file for a single name (field). Specify
 * multiple files by writing it like:
 *
 * 'name=@filename,filename2,filename3'
 *
 * or use double-quotes quote the filename:
 *
 * 'name=@"filename","filename2","filename3"'
 *
 * If you want content-types specified for each too, write them like:
 *
 * 'name=@filename;type=image/gif,filename2,filename3'
 *
 * If you want custom headers added for a single part, write them in a separate
 * file and do like this:
 *
 * 'name=foo;headers=@headerfile' or why not
 * 'name=@filemame;headers=@headerfile'
 *
 * To upload a file, but to fake the file name that will be included in the
 * formpost, do like this:
 *
 * 'name=@filename;filename=/dev/null' or quote the faked filename like:
 * 'name=@filename;filename="play, play, and play.txt"'
 *
 * If filename/path contains ',' or ';', it must be quoted by double-quotes,
 * else curl will fail to figure out the correct filename. if the filename
 * tobe quoted contains '"' or '\', '"' and '\' must be escaped by backslash.
 *
 ***************************************************************************/

/* Convenience macros for null pointer check. */
#define NULL_CHECK(ptr, init, retcode)                                  \
  do {                                                                  \
    (ptr) = (init);                                                     \
    if(!(ptr)) {                                                        \
      warnf(config->global, "out of memory!\n");                        \
      curl_slist_free_all(headers);                                     \
      mm_Curl_safefree(char, contents);                                          \
      return retcode;                                                   \
    }                                                                   \
  } while(0)

#define SET_TOOL_MIME_PTR(m, field, retcode)                            \
  do {                                                                  \
    if(field)                                                           \
      NULL_CHECK((m)->field, mm_strdup(field), retcode);                   \
  } while(0)

int formparse(mm_ptr<struct OperationConfig> config,
              mm_array_ptr<const char> input,
              mm_ptr<mm_ptr<struct tool_mime>> mimeroot,
              mm_ptr<mm_ptr<struct tool_mime>> mimecurrent,
              bool literal_value)
{
  /* input MUST be a string in the format 'name=contents' and we'll
     build a linked list with the info */
  mm_array_ptr<char> name = NULL;
  mm_array_ptr<char> contents = NULL;
  mm_array_ptr<char> contp = NULL;
  mm_array_ptr<char> data = NULL;
  mm_array_ptr<char> type = NULL;
  mm_array_ptr<char> filename = NULL;
  mm_array_ptr<char> encoder = NULL;
  struct curl_slist *headers = NULL;
  mm_ptr<struct tool_mime> part = NULL;
  CURLcode res;

  /* Allocate the main mime structure if needed. */
  if(!*mimecurrent) {
    NULL_CHECK(*mimeroot, tool_mime_new_parts(NULL), 1);
    *mimecurrent = *mimeroot;
  }

  /* Make a copy we can overwrite. */
  NULL_CHECK(contents, mm_strdup(input), 2);

  /* Scan for the end of the name. */
  contp = mm_strchr(contents, '=');
  if(contp) {
    int sep = '\0';
    if(contp > contents)
      name = contents;
    *contp++ = '\0';

    if(*contp == '(' && !literal_value) {
      /* Starting a multipart. */
      sep = get_param_part(config, '\0',
                           &contp, &data, &type, NULL, NULL, &headers);
      if(sep < 0) {
        mm_Curl_safefree(char, contents);
        return 3;
      }
      NULL_CHECK(part, tool_mime_new_parts(*mimecurrent), 4);
      *mimecurrent = part;
      part->headers = headers;
      headers = NULL;
      SET_TOOL_MIME_PTR(part, type, 5);
    }
    else if(!name && !mm_strcmp(contp, ")") && !literal_value) {
      /* Ending a multipart. */
      if(*mimecurrent == *mimeroot) {
        warnf(config->global, "no multipart to terminate!\n");
        mm_Curl_safefree(char, contents);
        return 6;
        }
      *mimecurrent = (*mimecurrent)->parent;
    }
    else if('@' == contp[0] && !literal_value) {

      /* we use the @-letter to indicate file name(s) */

      mm_ptr<struct tool_mime> subparts = NULL;

      do {
        /* since this was a file, it may have a content-type specifier
           at the end too, or a filename. Or both. */
        ++contp;
        sep = get_param_part(config, ',', &contp,
                             &data, &type, &filename, &encoder, &headers);
        if(sep < 0) {
          mm_Curl_safefree(char, contents);
          return 7;
        }

        /* now contp point to comma or string end.
           If more files to come, make sure we have multiparts. */
        if(!subparts) {
          if(sep != ',')    /* If there is a single file. */
            subparts = *mimecurrent;
          else
            NULL_CHECK(subparts, tool_mime_new_parts(*mimecurrent), 8);
        }

        /* Store that file in a part. */
        NULL_CHECK(part,
                   tool_mime_new_filedata(subparts, data, TRUE, &res), 9);
        part->headers = headers;
        headers = NULL;
        part->config = config->global;
        if(res == CURLE_READ_ERROR) {
            /* An error occurred while reading stdin: if read has started,
               issue the error now. Else, delay it until processed by
               libcurl. */
          if(part->size > 0) {
            warnf(config->global,
                  "error while reading standard input\n");
            mm_Curl_safefree(char, contents);
            return 10;
          }
          CONST_SAFEFREE(part->data);
          part->data = NULL;
          part->size = -1;
          res = CURLE_OK;
        }
        SET_TOOL_MIME_PTR(part, filename, 11);
        SET_TOOL_MIME_PTR(part, type, 12);
        SET_TOOL_MIME_PTR(part, encoder, 13);

        /* *contp could be '\0', so we just check with the delimiter */
      } while(sep); /* loop if there's another file name */
      part = (*mimecurrent)->subparts;  /* Set name on group. */
    }
    else {
      if(*contp == '<' && !literal_value) {
        ++contp;
        sep = get_param_part(config, '\0', &contp,
                             &data, &type, NULL, &encoder, &headers);
        if(sep < 0) {
          mm_Curl_safefree(char, contents);
          return 14;
        }

        NULL_CHECK(part, tool_mime_new_filedata(*mimecurrent, data, FALSE,
                                                &res), 15);
        part->headers = headers;
        headers = NULL;
        part->config = config->global;
        if(res == CURLE_READ_ERROR) {
            /* An error occurred while reading stdin: if read has started,
               issue the error now. Else, delay it until processed by
               libcurl. */
          if(part->size > 0) {
            warnf(config->global,
                  "error while reading standard input\n");
            mm_Curl_safefree(char, contents);
            return 16;
          }
          CONST_SAFEFREE(part->data);
          part->data = NULL;
          part->size = -1;
          res = CURLE_OK;
        }
      }
      else {
        if(literal_value)
          data = contp;
        else {
          sep = get_param_part(config, '\0', &contp,
                               &data, &type, &filename, &encoder, &headers);
          if(sep < 0) {
            mm_Curl_safefree(char, contents);
            return 17;
          }
        }

        NULL_CHECK(part, tool_mime_new_data(*mimecurrent, data), 18);
        part->headers = headers;
        headers = NULL;
      }

      SET_TOOL_MIME_PTR(part, filename, 19);
      SET_TOOL_MIME_PTR(part, type, 20);
      SET_TOOL_MIME_PTR(part, encoder, 21);

      if(sep) {
        *contp = (char) sep;
        warnf(config->global,
              "garbage at end of field specification: %s\n", _GETCHARPTR(contp));
      }
    }

    /* Set part name. */
    SET_TOOL_MIME_PTR(part, name, 22);
  }
  else {
    warnf(config->global, "Illegally formatted input field!\n");
    mm_Curl_safefree(char, contents);
    return 23;
  }
  mm_Curl_safefree(char, contents);
  return 0;
}
