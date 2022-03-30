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

#include "curl_setup.h"
#ifndef CURL_DISABLE_NETRC

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#include <curl/curl.h>
#include "netrc.h"
#include "strtok.h"
#include "strcase.h"

/* The last 3 #include files should be in this order */
#include "curl_printf.h"
#include "curl_memory.h"
#include "memdebug.h"

/* Get user and password from .netrc when given a machine name */

enum host_lookup_state {
  NOTHING,
  HOSTFOUND,    /* the 'machine' keyword was found */
  HOSTVALID,    /* this is "our" machine! */
  MACDEF
};

#define NETRC_FILE_MISSING 1
#define NETRC_FAILED -1
#define NETRC_SUCCESS 0

/*
 * Returns zero on success.
 */
static int parsenetrc(mm_array_ptr<const char> host,
                      mm_array_ptr<char> *loginp,
                      mm_array_ptr<char> *passwordp,
                      bool *login_changed,
                      bool *password_changed,
                      mm_array_ptr<char> netrcfile)
{
  FILE *file;
  int retcode = NETRC_FILE_MISSING;
  mm_array_ptr<char> login = *loginp;
  mm_array_ptr<char> password = *passwordp;
  bool specific_login = (login && *login != 0);
  bool login_alloc = FALSE;
  bool password_alloc = FALSE;
  enum host_lookup_state state = NOTHING;

  char state_login = 0;      /* Found a login keyword */
  char state_password = 0;   /* Found a password keyword */
  int state_our_login = FALSE;  /* With specific_login, found *our* login
                                   name */

  DEBUGASSERT(netrcfile);

  file = mm_fopen(netrcfile, FOPEN_READTEXT);
  if(file) {
    char *tok;
    char *tok_buf;
    bool done = FALSE;
    char netrcbuffer[4096];
    int  netrcbuffsize = (int)sizeof(netrcbuffer);

    while(!done && fgets(netrcbuffer, netrcbuffsize, file)) {
      if(state == MACDEF) {
        if((netrcbuffer[0] == '\n') || (netrcbuffer[0] == '\r'))
          state = NOTHING;
        else
          continue;
      }
      tok = strtok_r(netrcbuffer, " \t\n", &tok_buf);
      if(tok && *tok == '#')
        /* treat an initial hash as a comment line */
        continue;
      while(tok) {
        if((login && *login) && (password && *password)) {
          done = TRUE;
          break;
        }

        switch(state) {
        case NOTHING:
          if(strcasecompare("macdef", tok)) {
            /* Define a macro. A macro is defined with the specified name; its
               contents begin with the next .netrc line and continue until a
               null line (consecutive new-line characters) is encountered. */
            state = MACDEF;
          }
          else if(strcasecompare("machine", tok)) {
            /* the next tok is the machine name, this is in itself the
               delimiter that starts the stuff entered for this machine,
               after this we need to search for 'login' and
               'password'. */
            state = HOSTFOUND;
          }
          else if(strcasecompare("default", tok)) {
            state = HOSTVALID;
            retcode = NETRC_SUCCESS; /* we did find our host */
          }
          break;
        case MACDEF:
          if(!strlen(tok)) {
            state = NOTHING;
          }
          break;
        case HOSTFOUND:
          if(mm_strcasecompare_0(host, tok)) {
            /* and yes, this is our host! */
            state = HOSTVALID;
            retcode = NETRC_SUCCESS; /* we did find our host */
          }
          else
            /* not our host */
            state = NOTHING;
          break;
        case HOSTVALID:
          /* we are now parsing sub-keywords concerning "our" host */
          if(state_login) {
            if(specific_login) {
              state_our_login = mm_strcasecompare_0(login, tok);
            }
            else if(!login || mm_strcmp(login, tok)) {
              if(login_alloc) {
                MM_FREE(char, login);
                login_alloc = FALSE;
              }
              login = mm_strdup_from_raw(tok);
              if(!login) {
                retcode = NETRC_FAILED; /* allocation failed */
                goto out;
              }
              login_alloc = TRUE;
            }
            state_login = 0;
          }
          else if(state_password) {
            if((state_our_login || !specific_login)
                && (!password || mm_strcmp(password, tok))) {
              if(password_alloc) {
                MM_FREE(char, password);
                password_alloc = FALSE;
              }
              password = mm_strdup_from_raw(tok);
              if(!password) {
                retcode = NETRC_FAILED; /* allocation failed */
                goto out;
              }
              password_alloc = TRUE;
            }
            state_password = 0;
          }
          else if(strcasecompare("login", tok))
            state_login = 1;
          else if(strcasecompare("password", tok))
            state_password = 1;
          else if(strcasecompare("machine", tok)) {
            /* ok, there's machine here go => */
            state = HOSTFOUND;
            state_our_login = FALSE;
          }
          break;
        } /* switch (state) */

        tok = strtok_r(NULL, " \t\n", &tok_buf);
      } /* while(tok) */
    } /* while fgets() */

    out:
    if(!retcode) {
      /* success */
      *login_changed = FALSE;
      *password_changed = FALSE;
      if(login_alloc) {
        if(*loginp)
          MM_FREE(char, *loginp);
        *loginp = login;
        *login_changed = TRUE;
      }
      if(password_alloc) {
        if(*passwordp)
          MM_FREE(char, *passwordp);
        *passwordp = password;
        *password_changed = TRUE;
      }
    }
    else {
      if(login_alloc)
        MM_FREE(char, login);
      if(password_alloc)
        MM_FREE(char, password);
    }
    fclose(file);
  }

  return retcode;
}

/*
 * @unittest: 1304
 *
 * *loginp and *passwordp MUST be allocated if they aren't NULL when passed
 * in.
 */
int Curl_parsenetrc(mm_array_ptr<const char> host,
                    mm_array_ptr<char> *loginp,
                    mm_array_ptr<char> *passwordp,
                    bool *login_changed,
                    bool *password_changed,
                    mm_array_ptr<char> netrcfile)
{
  int retcode = 1;
  mm_array_ptr<char> filealloc = NULL;

  if(!netrcfile) {
    char *home = NULL;
    mm_array_ptr<char> homea = curl_getenv("HOME"); /* portable environment reader */
    if(homea) {
      home = _GETCHARPTR(homea);
#if defined(HAVE_GETPWUID_R) && defined(HAVE_GETEUID)
    }
    else {
      struct passwd pw, *pw_res;
      char pwbuf[1024];
      if(!getpwuid_r(geteuid(), &pw, pwbuf, sizeof(pwbuf), &pw_res)
         && pw_res) {
        home = pw.pw_dir;
      }
#elif defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
    }
    else {
      struct passwd *pw;
      pw = getpwuid(geteuid());
      if(pw) {
        home = pw->pw_dir;
      }
#endif
    }

    if(!home)
      return retcode; /* no home directory found (or possibly out of
                         memory) */

    filealloc = mmize_str(curl_maprintf("%s%s.netrc", home, DIR_CHAR));
    if(!filealloc) {
      MM_FREE(char, homea);
      return -1;
    }
    retcode = parsenetrc(host, loginp, passwordp, login_changed,
                         password_changed, filealloc);
    MM_FREE(char, filealloc);
#ifdef WIN32
    if(retcode == NETRC_FILE_MISSING) {
      /* fallback to the old-style "_netrc" file */
      filealloc = curl_maprintf("%s%s_netrc", home, DIR_CHAR);
      if(!filealloc) {
        MM_FREE(char, homea);
        return -1;
      }
      retcode = parsenetrc(host, loginp, passwordp, login_changed,
                           password_changed, filealloc);
      free(filealloc);
    }
#endif
    MM_FREE(char, homea);
  }
  else
    retcode = parsenetrc(host, loginp, passwordp, login_changed,
                         password_changed, netrcfile);
  return retcode;
}

#endif
