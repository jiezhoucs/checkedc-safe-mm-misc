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

#include <sys/stat.h>

#ifdef WIN32
#  include <direct.h>
#endif

#define ENABLE_CURLX_PRINTF
/* use our own printf() functions */
#include "curlx.h"

#include "tool_dirhie.h"

#include "memdebug.h" /* keep this as LAST include */

#ifdef NETWARE
#  ifndef __NOVELL_LIBC__
#    define mkdir mkdir_510
#  endif
#endif

#if defined(WIN32) || (defined(MSDOS) && !defined(__DJGPP__))
#  define mkdir(x,y) (mkdir)((x))
#  ifndef F_OK
#    define F_OK 0
#  endif
#endif

static void show_dir_errno(FILE *errors, const char *name)
{
  switch(errno) {
#ifdef EACCES
  case EACCES:
    fprintf(errors, "You don't have permission to create %s.\n", name);
    break;
#endif
#ifdef ENAMETOOLONG
  case ENAMETOOLONG:
    fprintf(errors, "The directory name %s is too long.\n", name);
    break;
#endif
#ifdef EROFS
  case EROFS:
    fprintf(errors, "%s resides on a read-only file system.\n", name);
    break;
#endif
#ifdef ENOSPC
  case ENOSPC:
    fprintf(errors, "No space left on the file system that will "
            "contain the directory %s.\n", name);
    break;
#endif
#ifdef EDQUOT
  case EDQUOT:
    fprintf(errors, "Cannot create directory %s because you "
            "exceeded your quota.\n", name);
    break;
#endif
  default :
    fprintf(errors, "Error creating directory %s.\n", name);
    break;
  }
}

/*
 * Create the needed directory hierarchy recursively in order to save
 *  multi-GETs in file output, ie:
 *  curl "http://my.site/dir[1-5]/file[1-5].txt" -o "dir#1/file#2.txt"
 *  should create all the dir* automagically
 */

#if defined(WIN32) || defined(__DJGPP__)
/* systems that may use either or when specifying a path */
#define PATH_DELIMITERS "\\/"
#else
#define PATH_DELIMITERS DIR_CHAR
#endif


CURLcode create_dir_hierarchy(mm_array_ptr<const char> outfile, FILE *errors)
{
  mm_array_ptr<char> tempdir = NULL;
  mm_array_ptr<char> tempdir2 = NULL;
  mm_array_ptr<char> outdup = NULL;
  mm_array_ptr<char> dirbuildup = NULL;
  CURLcode result = CURLE_OK;
  size_t outlen;

  outlen = mm_strlen(outfile);
  outdup = mm_strdup(outfile);
  if(!outdup)
    return CURLE_OUT_OF_MEMORY;

  /* Checked C: No need to port the next line as it is only passed to libc fn
   * and show_dir_errno() which just prints stuffs, i.e., no escaping. */
  dirbuildup = MM_ARRAY_ALLOC(char, outlen + 1);
  if(!dirbuildup) {
    mm_Curl_safefree(char, outdup);
    return CURLE_OUT_OF_MEMORY;
  }
  dirbuildup[0] = '\0';

  /* Allow strtok() here since this isn't used threaded */
  /* !checksrc! disable BANNEDFUNC 2 */
  tempdir = mm_strtok(outdup, PATH_DELIMITERS, outdup);

  while(tempdir != NULL) {
    bool skip = false;
    tempdir2 = mm_strtok(NULL, PATH_DELIMITERS, outdup);
    /* since strtok returns a token for the last word even
       if not ending with DIR_CHAR, we need to prune it */
    if(tempdir2 != NULL) {
      size_t dlen = mm_strlen(dirbuildup);
      if(dlen)
        msnprintf(_GETCHARPTR(&dirbuildup[dlen]), outlen - dlen, "%s%s",
            DIR_CHAR, _GETCHARPTR(tempdir));
      else {
        if(outdup == tempdir) {
#if defined(MSDOS) || defined(WIN32)
          /* Skip creating a drive's current directory.
             It may seem as though that would harmlessly fail but it could be
             a corner case if X: did not exist, since we would be creating it
             erroneously.
             eg if outfile is X:\foo\bar\filename then don't mkdir X:
             This logic takes into account unsupported drives !:, 1:, etc. */
          mm_array_ptr<char> p = mm_strchr(tempdir, ':');
          if(p && !p[1])
            skip = true;
#endif
          /* the output string doesn't start with a separator */
          mm_strcpy(dirbuildup, tempdir);
        }
        else
          msnprintf(_GETCHARPTR(dirbuildup), outlen, "%s%s", DIR_CHAR, _GETCHARPTR(tempdir));
      }
      /* Create directory. Ignore access denied error to allow traversal. */
      if(!skip && (-1 == mkdir(_GETCHARPTR(dirbuildup), (mode_t)0000750)) &&
         (errno != EACCES) && (errno != EEXIST)) {
        show_dir_errno(errors, _GETCHARPTR(dirbuildup));
        result = CURLE_WRITE_ERROR;
        break; /* get out of loop */
      }
    }
    tempdir = tempdir2;
  }

  mm_Curl_safefree(char, dirbuildup);
  mm_Curl_safefree(char, outdup);

  return result;
}
