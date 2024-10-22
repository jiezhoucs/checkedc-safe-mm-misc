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
#include "tool_getparam.h"
#include "tool_getpass.h"
#include "tool_homedir.h"
#include "tool_msgs.h"
#include "tool_paramhlp.h"
#include "tool_version.h"
#include "dynbuf.h"

#include "memdebug.h" /* keep this as LAST include */

mm_ptr<struct getout> new_getout(mm_ptr<struct OperationConfig> config)
{
  mm_ptr<struct getout> node = MM_SINGLE_CALLOC(struct getout);
  mm_ptr<struct getout> last = config->url_last;
  if(node) {
    static int outnum = 0;

    /* append this new node last in the list */
    if(last)
      last->next = node;
    else
      config->url_list = node; /* first node */

    /* move the last pointer */
    config->url_last = node;

    node->flags = config->default_node_flags;
    node->num = outnum++;
  }
  return node;
}

#define MAX_FILE2STRING (256*1024*1024) /* big enough ? */

ParameterError file2string(mm_ptr<mm_array_ptr<char>> bufp, FILE *file)
{
  struct curlx_dynbuf dyn;
  curlx_dyn_init(&dyn, MAX_FILE2STRING);
  if(file) {
    char buffer[256];

    while(fgets(buffer, sizeof(buffer), file)) {
      char *ptr = strchr(buffer, '\r');
      if(ptr)
        *ptr = '\0';
      ptr = strchr(buffer, '\n');
      if(ptr)
        *ptr = '\0';
      if(curlx_dyn_add(&dyn, buffer))
        return PARAM_NO_MEM;
    }
  }
  *bufp = curlx_dyn_ptr(&dyn);
  return PARAM_OK;
}

#define MAX_FILE2MEMORY (1024*1024*1024) /* big enough ? */

ParameterError file2memory(mm_array_ptr<char> *bufp, size_t *size, FILE *file)
{
  if(file) {
    size_t nread;
    struct curlx_dynbuf dyn;
    curlx_dyn_init(&dyn, MAX_FILE2MEMORY);
    do {
      char buffer[4096];
      nread = fread(buffer, 1, sizeof(buffer), file);
      if(nread)
        if(curlx_dyn_addn(&dyn, buffer, nread))
          return PARAM_NO_MEM;
    } while(nread);
    *size = curlx_dyn_len(&dyn);
    *bufp = curlx_dyn_ptr(&dyn);
  }
  else {
    *size = 0;
    *bufp = NULL;
  }
  return PARAM_OK;
}

void cleanarg(mm_array_ptr<char> str)
{
#ifdef HAVE_WRITABLE_ARGV
  /* now that GetStr has copied the contents of nextarg, wipe the next
   * argument out so that the username:password isn't displayed in the
   * system process list */
  if(str) {
    size_t len = mm_strlen(str);
    mm_memset(str, ' ', len);
  }
#else
  (void)str;
#endif
}

/*
 * Parse the string and write the long in the given address. Return PARAM_OK
 * on success, otherwise a parameter specific error enum.
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */
/* No need to convert long *val to mm_ptr type. */
static ParameterError getnum(mm_ptr<long> val, mm_array_ptr<const char> str, int base)
{
  if(str) {
    mm_array_ptr<char> endptr = NULL;
    long num;
    errno = 0;
    num = mm_strtol(str, &endptr, base);
    if(errno == ERANGE)
      return PARAM_NUMBER_TOO_LARGE;
    if((endptr != str) && (endptr == str + mm_strlen(str))) {
      *val = num;
      return PARAM_OK;  /* Ok */
    }
  }
  return PARAM_BAD_NUMERIC; /* badness */
}

static ParameterError getnum_raw_val(long *val, mm_array_ptr<const char> str, int base) {
  if(str) {
    mm_array_ptr<char> endptr = NULL;
    long num;
    errno = 0;
    num = mm_strtol(str, &endptr, base);
    if(errno == ERANGE)
      return PARAM_NUMBER_TOO_LARGE;
    if((endptr != str) && (endptr == str + mm_strlen(str))) {
      *val = num;
      return PARAM_OK;  /* Ok */
    }
  }
  return PARAM_BAD_NUMERIC; /* badness */
}

/* No need to convert long *val to mm_ptr type. */
ParameterError str2num(mm_ptr<long> val, mm_array_ptr<const char> str)
{
  return getnum(val, str, 10);
}

ParameterError oct2nummax(mm_ptr<long> val, mm_array_ptr<const char> str, long max)
{
  ParameterError result = getnum(val, str, 8);
  if(result != PARAM_OK)
    return result;
  else if(*val > max)
    return PARAM_NUMBER_TOO_LARGE;
  else if(*val < 0)
    return PARAM_NEGATIVE_NUMERIC;

  return PARAM_OK;
}

/*
 * Parse the string and write the long in the given address. Return PARAM_OK
 * on success, otherwise a parameter error enum. ONLY ACCEPTS POSITIVE NUMBERS!
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */

/* No need to convert long *val to mm_ptr type. */
ParameterError str2unum(mm_ptr<long> val, mm_array_ptr<const char> str)
{
  ParameterError result = getnum(val, str, 10);
  if(result != PARAM_OK)
    return result;
  if(*val < 0)
    return PARAM_NEGATIVE_NUMERIC;

  return PARAM_OK;
}

ParameterError str2unum_raw_val(long *val, mm_array_ptr<const char> str) {
  ParameterError result = getnum_raw_val(val, str, 10);
  if(result != PARAM_OK)
    return result;
  if(*val < 0)
    return PARAM_NEGATIVE_NUMERIC;

  return PARAM_OK;
}

/*
 * Parse the string and write the long in the given address if it is below the
 * maximum allowed value. Return PARAM_OK on success, otherwise a parameter
 * error enum. ONLY ACCEPTS POSITIVE NUMBERS!
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */

/* No need to convert long *val to mm_ptr type. */
ParameterError str2unummax(mm_ptr<long> val, mm_array_ptr<const char> str, long max)
{
  ParameterError result = str2unum(val, str);
  if(result != PARAM_OK)
    return result;
  if(*val > max)
    return PARAM_NUMBER_TOO_LARGE;

  return PARAM_OK;
}


/*
 * Parse the string and write the double in the given address. Return PARAM_OK
 * on success, otherwise a parameter specific error enum.
 *
 * The 'max' argument is the maximum value allowed, as the numbers are often
 * multiplied when later used.
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */

static ParameterError str2double(mm_ptr<double> val, mm_array_ptr<const char> str, long max)
{
  if(str) {
    mm_array_ptr<char> endptr = NULL;
    double num;
    errno = 0;
    num = mm_strtod(str, &endptr);
    if(errno == ERANGE)
      return PARAM_NUMBER_TOO_LARGE;
    if(num > max) {
      /* too large */
      return PARAM_NUMBER_TOO_LARGE;
    }
    if((endptr != str) && (endptr == str + mm_strlen(str))) {
      *val = num;
      return PARAM_OK;  /* Ok */
    }
  }
  return PARAM_BAD_NUMERIC; /* badness */
}

/*
 * Parse the string and write the double in the given address. Return PARAM_OK
 * on success, otherwise a parameter error enum. ONLY ACCEPTS POSITIVE NUMBERS!
 *
 * The 'max' argument is the maximum value allowed, as the numbers are often
 * multiplied when later used.
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */

ParameterError str2udouble(mm_ptr<double> valp, mm_array_ptr<const char> str, long max)
{
  double value;
  ParameterError result = str2double(&value, str, max);
  if(result != PARAM_OK)
    return result;
  if(value < 0)
    return PARAM_NEGATIVE_NUMERIC;

  *valp = value;
  return PARAM_OK;
}

/*
 * Parse the string and modify the long in the given address. Return
 * non-zero on failure, zero on success.
 *
 * The string is a list of protocols
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */

long proto2num(mm_ptr<struct OperationConfig> config, mm_ptr<long> val, mm_array_ptr<const char> str)
{
  mm_array_ptr<char> buffer = NULL;
  const char *sep = ",";
  mm_array_ptr<char> token = NULL;

  static struct sprotos {
    const char *name;
    long bit;
  } const protos[] = {
    { "all", CURLPROTO_ALL },
    { "http", CURLPROTO_HTTP },
    { "https", CURLPROTO_HTTPS },
    { "ftp", CURLPROTO_FTP },
    { "ftps", CURLPROTO_FTPS },
    { "scp", CURLPROTO_SCP },
    { "sftp", CURLPROTO_SFTP },
    { "telnet", CURLPROTO_TELNET },
    { "ldap", CURLPROTO_LDAP },
    { "ldaps", CURLPROTO_LDAPS },
    { "dict", CURLPROTO_DICT },
    { "file", CURLPROTO_FILE },
    { "tftp", CURLPROTO_TFTP },
    { "imap", CURLPROTO_IMAP },
    { "imaps", CURLPROTO_IMAPS },
    { "pop3", CURLPROTO_POP3 },
    { "pop3s", CURLPROTO_POP3S },
    { "smtp", CURLPROTO_SMTP },
    { "smtps", CURLPROTO_SMTPS },
    { "rtsp", CURLPROTO_RTSP },
    { "gopher", CURLPROTO_GOPHER },
    { "smb", CURLPROTO_SMB },
    { "smbs", CURLPROTO_SMBS },
    { NULL, 0 }
  };

  if(!str)
    return 1;

  buffer = mm_strdup(str); /* because strtok corrupts it */
  if(!buffer)
    return 1;

  /* Allow strtok() here since this isn't used threaded */
  /* !checksrc! disable BANNEDFUNC 2 */
  for(token = mm_strtok(buffer, sep, buffer);
      token;
      token = mm_strtok(NULL, sep, buffer)) {
    enum e_action { allow, deny, set } action = allow;

    struct sprotos const *pp;

    /* Process token modifiers */
    while(!ISALNUM(*token)) { /* may be NULL if token is all modifiers */
      switch (*token++) {
      case '=':
        action = set;
        break;
      case '-':
        action = deny;
        break;
      case '+':
        action = allow;
        break;
      default: /* Includes case of terminating NULL */
        mm_Curl_safefree(char, buffer);
        return 1;
      }
    }

    for(pp = protos; pp->name; pp++) {
      if(mm_strcasecompare_0(token, pp->name)) {
        switch(action) {
        case deny:
          *val &= ~(pp->bit);
          break;
        case allow:
          *val |= pp->bit;
          break;
        case set:
          *val = pp->bit;
          break;
        }
        break;
      }
    }

    if(!(pp->name)) { /* unknown protocol */
      /* If they have specified only this protocol, we say treat it as
         if no protocols are allowed */
      if(action == set)
        *val = 0;
      warnf(config->global, "unrecognized protocol '%s'\n", _GETCHARPTR(token));
    }
  }
  mm_Curl_safefree(char, buffer);
  return 0;
}

/**
 * Check if the given string is a protocol supported by libcurl
 *
 * @param str  the protocol name
 * @return PARAM_OK  protocol supported
 * @return PARAM_LIBCURL_UNSUPPORTED_PROTOCOL  protocol not supported
 * @return PARAM_REQUIRES_PARAMETER   missing parameter
 */
int check_protocol(mm_array_ptr<const char> str)
{
  const char * const *pp;
  const curl_version_info_data *curlinfo = curl_version_info(CURLVERSION_NOW);
  if(!str)
    return PARAM_REQUIRES_PARAMETER;
  for(pp = curlinfo->protocols; *pp; pp++) {
    if(mm_strcasecompare_1(*pp, str))
      return PARAM_OK;
  }
  return PARAM_LIBCURL_UNSUPPORTED_PROTOCOL;
}

/**
 * Parses the given string looking for an offset (which may be a
 * larger-than-integer value). The offset CANNOT be negative!
 *
 * @param val  the offset to populate
 * @param str  the buffer containing the offset
 * @return PARAM_OK if successful, a parameter specific error enum if failure.
 */
ParameterError str2offset(mm_ptr<curl_off_t> val, mm_array_ptr<const char> str)
{
  mm_array_ptr<char> endptr = NULL;
  if(str[0] == '-')
    /* offsets aren't negative, this indicates weird input */
    return PARAM_NEGATIVE_NUMERIC;

#if(SIZEOF_CURL_OFF_T > SIZEOF_LONG)
  {
    CURLofft offt = curlx_strtoofft(_GETCHARPTR(str), &endptr, 0, val);
    if(CURL_OFFT_FLOW == offt)
      return PARAM_NUMBER_TOO_LARGE;
    else if(CURL_OFFT_INVAL == offt)
      return PARAM_BAD_NUMERIC;
  }
#else
  errno = 0;
  *val = mm_strtol(str, &endptr, 0);
  if((*val == LONG_MIN || *val == LONG_MAX) && errno == ERANGE)
    return PARAM_NUMBER_TOO_LARGE;
#endif
  if((endptr != str) && (endptr == str + mm_strlen(str)))
    return PARAM_OK;

  return PARAM_BAD_NUMERIC;
}

#define MAX_USERPWDLENGTH (100*1024)
static CURLcode checkpasswd(const char *kind, /* for what purpose */
                            const size_t i,   /* operation index */
                            const bool last,  /* TRUE if last operation */
                            mm_ptr<char *> userpwd)   /* pointer to allocated string */
{
  char *psep;
  char *osep;

  if(!*userpwd)
    return CURLE_OK;

  /* Attempt to find the password separator */
  psep = strchr(*userpwd, ':');

  /* Attempt to find the options separator */
  osep = strchr(*userpwd, ';');

  if(!psep && **userpwd != ';') {
    /* no password present, prompt for one */
    char passwd[2048] = "";
    char prompt[256];
    struct curlx_dynbuf dyn;

    curlx_dyn_init(&dyn, MAX_USERPWDLENGTH);
    if(osep)
      *osep = '\0';

    /* build a nice-looking prompt */
    if(!i && last)
      curlx_msnprintf(prompt, sizeof(prompt),
                      "Enter %s password for user '%s':",
                      kind, *userpwd);
    else
      curlx_msnprintf(prompt, sizeof(prompt),
                      "Enter %s password for user '%s' on URL #%zu:",
                      kind, *userpwd, i + 1);

    /* get password */
    getpass_r(prompt, passwd, sizeof(passwd));
    if(osep)
      *osep = ';';

    if(curlx_dyn_addf(&dyn, "%s:%s", *userpwd, passwd))
      return CURLE_OUT_OF_MEMORY;

    /* return the new string */
    free(*userpwd);
    *userpwd = _GETCHARPTR(curlx_dyn_ptr(&dyn));
  }

  return CURLE_OK;
}

ParameterError add2list(mm_ptr<struct curl_slist *> list, mm_array_ptr<const char> ptr)
{
  struct curl_slist *newlist = curl_slist_append(*list, _GETCHARPTR(ptr));
  if(newlist)
    *list = newlist;
  else
    return PARAM_NO_MEM;

  return PARAM_OK;
}

int ftpfilemethod(struct OperationConfig *config, const char *str)
{
  if(curl_strequal("singlecwd", str))
    return CURLFTPMETHOD_SINGLECWD;
  if(curl_strequal("nocwd", str))
    return CURLFTPMETHOD_NOCWD;
  if(curl_strequal("multicwd", str))
    return CURLFTPMETHOD_MULTICWD;

  warnf(config->global, "unrecognized ftp file method '%s', using default\n",
        str);

  return CURLFTPMETHOD_MULTICWD;
}

int ftpcccmethod(mm_ptr<struct OperationConfig> config, mm_array_ptr<const char> str)
{
  if(mm_strcasecompare("passive", str))
    return CURLFTPSSL_CCC_PASSIVE;
  if(mm_strcasecompare("active", str))
    return CURLFTPSSL_CCC_ACTIVE;

  warnf(config->global, "unrecognized ftp CCC method '%s', using default\n",
        _GETCHARPTR(str));

  return CURLFTPSSL_CCC_PASSIVE;
}

long delegation(mm_ptr<struct OperationConfig> config, mm_array_ptr<const char> str)
{
  if(mm_strcasecompare("none", str))
    return CURLGSSAPI_DELEGATION_NONE;
  if(mm_strcasecompare("policy", str))
    return CURLGSSAPI_DELEGATION_POLICY_FLAG;
  if(mm_strcasecompare("always", str))
    return CURLGSSAPI_DELEGATION_FLAG;

  warnf(config->global, "unrecognized delegation method '%s', using none\n",
        _GETCHARPTR(str));

  return CURLGSSAPI_DELEGATION_NONE;
}

/*
 * my_useragent: returns allocated string with default user agent
 */
static mm_array_ptr<char> my_useragent(void)
{
  return mm_strdup_from_raw(CURL_NAME "/" CURL_VERSION);
}

CURLcode get_args(mm_ptr<struct OperationConfig> config, const size_t i)
{
  CURLcode result = CURLE_OK;
  bool last = (config->next ? FALSE : TRUE);

  /* Check we have a password for the given host user */
  if(config->userpwd && !config->oauth_bearer) {
    result = checkpasswd("host", i, last, &config->userpwd);
    if(result)
      return result;
  }

  /* Check we have a password for the given proxy user */
  if(config->proxyuserpwd) {
    result = checkpasswd("proxy", i, last, &config->proxyuserpwd);
    if(result)
      return result;
  }

  /* Check we have a user agent */
  if(!config->useragent) {
    config->useragent = my_useragent();
    if(!config->useragent) {
      errorf(config->global, "out of memory\n");
      result = CURLE_OUT_OF_MEMORY;
    }
  }

  return result;
}

/*
 * Parse the string and modify ssl_version in the val argument. Return PARAM_OK
 * on success, otherwise a parameter error enum. ONLY ACCEPTS POSITIVE NUMBERS!
 *
 * Since this function gets called with the 'nextarg' pointer from within the
 * getparameter a lot, we must check it for NULL before accessing the str
 * data.
 */

ParameterError str2tls_max(mm_ptr<long> val, mm_array_ptr<const char> str)
{
   static struct s_tls_max {
    const char *tls_max_str;
    long tls_max;
  } const tls_max_array[] = {
    { "default", CURL_SSLVERSION_MAX_DEFAULT },
    { "1.0",     CURL_SSLVERSION_MAX_TLSv1_0 },
    { "1.1",     CURL_SSLVERSION_MAX_TLSv1_1 },
    { "1.2",     CURL_SSLVERSION_MAX_TLSv1_2 },
    { "1.3",     CURL_SSLVERSION_MAX_TLSv1_3 }
  };
  size_t i = 0;
  if(!str)
    return PARAM_REQUIRES_PARAMETER;
  for(i = 0; i < sizeof(tls_max_array)/sizeof(tls_max_array[0]); i++) {
    if(!mm_strcmp(str, tls_max_array[i].tls_max_str)) {
      *val = tls_max_array[i].tls_max;
      return PARAM_OK;
    }
  }
  return PARAM_BAD_USE;
}
