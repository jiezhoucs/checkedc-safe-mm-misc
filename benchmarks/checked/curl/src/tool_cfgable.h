#ifndef HEADER_CURL_TOOL_CFGABLE_H
#define HEADER_CURL_TOOL_CFGABLE_H
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
#include "tool_sdecls.h"
#include "tool_urlglob.h"
#include "tool_formparse.h"

#include <safe_mm_checked.h>

typedef enum {
  ERR_NONE,
  ERR_BINARY_TERMINAL = 1, /* binary to terminal detected */
  ERR_LAST
} curl_error;

struct GlobalConfig;

struct State {
  mm_ptr<struct getout> urlnode;
  mm_ptr<struct URLGlob> inglob;
  mm_ptr<struct URLGlob> urls;
  mm_array_ptr<char> outfiles;
  mm_array_ptr<char> httpgetfields;
  mm_array_ptr<char> uploadfile;
  unsigned long infilenum; /* number of files to upload */
  unsigned long up;  /* upload file counter within a single upload glob */
  unsigned long urlnum; /* how many iterations this single URL has with ranges
                           etc */
  unsigned long li;
};

struct OperationConfig {
  bool remote_time;
  mm_array_ptr<char> random_file;
  mm_array_ptr<char> egd_file;
  mm_array_ptr<char> useragent;
  struct curl_slist *cookies;  /* cookies to serialize into a single line */
  mm_array_ptr<char> cookiejar;          /* write to this file */
  struct curl_slist *cookiefiles;  /* file(s) to load cookies from */
  mm_array_ptr<char> altsvc;             /* alt-svc cache file name */
  mm_array_ptr<char> hsts;               /* HSTS cache file name */
  bool cookiesession;       /* new session? */
  bool encoding;            /* Accept-Encoding please */
  bool tr_encoding;         /* Transfer-Encoding please */
  unsigned long authtype;   /* auth bitmask */
  bool use_resume;
  bool resume_from_current;
  bool disable_epsv;
  bool disable_eprt;
  bool ftp_pret;
  long proto;
  bool proto_present;
  long proto_redir;
  bool proto_redir_present;
  mm_array_ptr<char> proto_default;
  curl_off_t resume_from;
  mm_array_ptr<char> postfields;
  curl_off_t postfieldsize;
  mm_array_ptr<char> referer;
  double timeout;
  double connecttimeout;
  long maxredirs;
  curl_off_t max_filesize;
  mm_array_ptr<char> output_dir;
  mm_array_ptr<char> headerfile;
  mm_array_ptr<char> ftpport;
  mm_array_ptr<char> iface;
  long localport;
  long localportrange;
  unsigned short porttouse;
  mm_array_ptr<char> range;
  long low_speed_limit;
  long low_speed_time;
  mm_array_ptr<char> dns_servers;   /* dot notation: 1.1.1.1;2.2.2.2 */
  mm_array_ptr<char> dns_interface; /* interface name */
  mm_array_ptr<char> dns_ipv4_addr; /* dot notation */
  mm_array_ptr<char> dns_ipv6_addr; /* dot notation */
  mm_array_ptr<char> userpwd;
  mm_array_ptr<char> login_options;
  mm_array_ptr<char> tls_username;
  mm_array_ptr<char> tls_password;
  mm_array_ptr<char> tls_authtype;
  mm_array_ptr<char> proxy_tls_username;
  mm_array_ptr<char> proxy_tls_password;
  mm_array_ptr<char> proxy_tls_authtype;
  mm_array_ptr<char> proxyuserpwd;
  mm_array_ptr<char> proxy;
  int proxyver;             /* set to CURLPROXY_HTTP* define */
  mm_array_ptr<char> noproxy;
  mm_array_ptr<char> mail_from;
  struct curl_slist *mail_rcpt;
  mm_array_ptr<char> mail_auth;
  bool mail_rcpt_allowfails; /* --mail-rcpt-allowfails */
  mm_array_ptr<char> sasl_authzid;       /* Authorisation identity (identity to use) */
  bool sasl_ir;             /* Enable/disable SASL initial response */
  bool proxytunnel;
  bool ftp_append;          /* APPE on ftp */
  bool use_ascii;           /* select ascii or text transfer */
  bool autoreferer;         /* automatically set referer */
  bool failonerror;         /* fail on (HTTP) errors */
  bool failwithbody;        /* fail on (HTTP) errors but still store body */
  bool show_headers;        /* show headers to data output */
  bool no_body;             /* don't get the body */
  bool dirlistonly;         /* only get the FTP dir list */
  bool followlocation;      /* follow http redirects */
  bool unrestricted_auth;   /* Continue to send authentication (user+password)
                               when following ocations, even when hostname
                               changed */
  bool netrc_opt;
  bool netrc;
  mm_array_ptr<char> netrc_file;
  mm_ptr<struct getout> url_list;  /* point to the first node */
  mm_ptr<struct getout> url_last;  /* point to the last/current node */
  mm_ptr<struct getout> url_get;   /* point to the node to fill in URL */
  mm_ptr<struct getout> url_out;   /* point to the node to fill in outfile */
  mm_ptr<struct getout> url_ul;    /* point to the node to fill in upload */
  mm_array_ptr<char> doh_url;
  mm_array_ptr<char> cipher_list;
  mm_array_ptr<char> proxy_cipher_list;
  mm_array_ptr<char> cipher13_list;
  mm_array_ptr<char> proxy_cipher13_list;
  mm_array_ptr<char> cert;
  mm_array_ptr<char> proxy_cert;
  mm_array_ptr<char> cert_type;
  mm_array_ptr<char> proxy_cert_type;
  mm_array_ptr<char> cacert;
  mm_array_ptr<char> proxy_cacert;
  mm_array_ptr<char> capath;
  mm_array_ptr<char> proxy_capath;
  mm_array_ptr<char> crlfile;
  mm_array_ptr<char> proxy_crlfile;
  mm_array_ptr<char> pinnedpubkey;
  mm_array_ptr<char> proxy_pinnedpubkey;
  mm_array_ptr<char> key;
  mm_array_ptr<char> proxy_key;
  mm_array_ptr<char> key_type;
  mm_array_ptr<char> proxy_key_type;
  mm_array_ptr<char> key_passwd;
  mm_array_ptr<char> proxy_key_passwd;
  mm_array_ptr<char> pubkey;
  mm_array_ptr<char> hostpubmd5;
  mm_array_ptr<char> engine;
  mm_array_ptr<char> etag_save_file;
  mm_array_ptr<char> etag_compare_file;
  bool crlf;
  mm_array_ptr<char> customrequest;
  mm_array_ptr<char> ssl_ec_curves;
  mm_array_ptr<char> krblevel;
  mm_array_ptr<char> request_target;
  long httpversion;
  bool http09_allowed;
  bool nobuffer;
  bool readbusy;            /* set when reading input returns EAGAIN */
  bool globoff;
  bool use_httpget;
  bool insecure_ok;         /* set TRUE to allow insecure SSL connects */
  bool doh_insecure_ok;     /* set TRUE to allow insecure SSL connects
                               for DoH */
  bool proxy_insecure_ok;   /* set TRUE to allow insecure SSL connects
                               for proxy */
  bool terminal_binary_ok;
  bool verifystatus;
  bool doh_verifystatus;
  bool create_dirs;
  bool ftp_create_dirs;
  bool ftp_skip_ip;
  bool proxynegotiate;
  bool proxyntlm;
  bool proxydigest;
  bool proxybasic;
  bool proxyanyauth;
  mm_array_ptr<char> writeout;           /* %-styled format string to output */
  struct curl_slist *quote;
  struct curl_slist *postquote;
  struct curl_slist *prequote;
  long ssl_version;
  long ssl_version_max;
  long proxy_ssl_version;
  long ip_version;
  long create_file_mode; /* CURLOPT_NEW_FILE_PERMS */
  curl_TimeCond timecond;
  curl_off_t condtime;
  struct curl_slist *headers;
  struct curl_slist *proxyheaders;
  mm_ptr<struct tool_mime> mimeroot;
  mm_ptr<struct tool_mime> mimecurrent;
  curl_mime *mimepost;
  struct curl_slist *telnet_options;
  struct curl_slist *resolve;
  struct curl_slist *connect_to;
  HttpReq httpreq;

  /* for bandwidth limiting features: */
  curl_off_t sendpersecond; /* send to peer */
  curl_off_t recvpersecond; /* receive from peer */

  bool ftp_ssl;
  bool ftp_ssl_reqd;
  bool ftp_ssl_control;
  bool ftp_ssl_ccc;
  int ftp_ssl_ccc_mode;
  mm_array_ptr<char> preproxy;
  int socks5_gssapi_nec;    /* The NEC reference server does not protect the
                               encryption type exchange */
  unsigned long socks5_auth;/* auth bitmask for socks5 proxies */
  mm_array_ptr<char> proxy_service_name; /* set authentication service name for HTTP and
                               SOCKS5 proxies */
  mm_array_ptr<char> service_name;       /* set authentication service name for DIGEST-MD5,
                               Kerberos 5 and SPNEGO */

  bool tcp_nodelay;
  bool tcp_fastopen;
  long req_retry;           /* number of retries */
  bool retry_all_errors;    /* retry on any error */
  bool retry_connrefused;   /* set connection refused as a transient error */
  long retry_delay;         /* delay between retries (in seconds) */
  long retry_maxtime;       /* maximum time to keep retrying */

  mm_array_ptr<char> ftp_account;        /* for ACCT */
  mm_array_ptr<char> ftp_alternative_to_user;  /* send command if USER/PASS fails */
  int ftp_filemethod;
  long tftp_blksize;        /* TFTP BLKSIZE option */
  bool tftp_no_options;     /* do not send TFTP options requests */
  bool ignorecl;            /* --ignore-content-length */
  bool disable_sessionid;

  bool raw;
  bool post301;
  bool post302;
  bool post303;
  bool nokeepalive;         /* for keepalive needs */
  long alivetime;
  bool content_disposition; /* use Content-disposition filename */

  int default_node_flags;   /* default flags to search for each 'node', which
                               is basically each given URL to transfer */

  bool xattr;               /* store metadata in extended attributes */
  long gssapi_delegation;
  bool ssl_allow_beast;     /* allow this SSL vulnerability */
  bool proxy_ssl_allow_beast; /* allow this SSL vulnerability for proxy*/

  bool ssl_no_revoke;       /* disable SSL certificate revocation checks */
  /*bool proxy_ssl_no_revoke; */

  bool ssl_revoke_best_effort; /* ignore SSL revocation offline/missing
                                  revocation list errors */

  bool native_ca_store;        /* use the native os ca store */
  bool ssl_auto_client_cert;   /* automatically locate and use a client
                                  certificate for authentication (Schannel) */
  bool proxy_ssl_auto_client_cert; /* proxy version of ssl_auto_client_cert */
  mm_array_ptr<char> oauth_bearer;             /* OAuth 2.0 bearer token */
  bool nonpn;                     /* enable/disable TLS NPN extension */
  bool noalpn;                    /* enable/disable TLS ALPN extension */
  mm_array_ptr<char> unix_socket_path;         /* path to Unix domain socket */
  bool abstract_unix_socket;      /* path to an abstract Unix domain socket */
  bool falsestart;
  bool path_as_is;
  double expect100timeout;
  bool suppress_connect_headers;  /* suppress proxy CONNECT response headers
                                     from user callbacks */
  curl_error synthetic_error;     /* if non-zero, it overrides any libcurl
                                     error */
  bool ssh_compression;           /* enable/disable SSH compression */
  long happy_eyeballs_timeout_ms; /* happy eyeballs timeout in milliseconds.
                                     0 is valid. default: CURL_HET_DEFAULT. */
  bool haproxy_protocol;          /* whether to send HAProxy protocol v1 */
  bool disallow_username_in_url;  /* disallow usernames in URLs */
  mm_array_ptr<char> aws_sigv4;
  struct GlobalConfig *global;
  mm_ptr<struct OperationConfig> prev;
  mm_ptr<struct OperationConfig> next;   /* Always last in the struct */
  struct State state;             /* for create_transfer() */
};

struct GlobalConfig {
  int showerror;                  /* -1 == unset, default => show errors
                                      0 => -s is used to NOT show errors
                                      1 => -S has been used to show errors */
  bool mute;                      /* don't show messages, --silent given */
  bool noprogress;                /* don't show progress bar --silent given */
  bool isatty;                    /* Updated internally if output is a tty */
  FILE *errors;                   /* Error stream, defaults to stderr */
  bool errors_fopened;            /* Whether error stream isn't stderr */
  mm_array_ptr<char> trace_dump;               /* file to dump the network trace to */
  FILE *trace_stream;
  bool trace_fopened;
  trace tracetype;
  bool tracetime;                 /* include timestamp? */
  int progressmode;               /* CURL_PROGRESS_BAR / CURL_PROGRESS_STATS */
  mm_array_ptr<char> libcurl;                  /* Output libcurl code to this file name */
  bool fail_early;                /* exit on first transfer error */
  bool styled_output;             /* enable fancy output style detection */
#ifdef CURLDEBUG
  bool test_event_based;
#endif
  bool parallel;
  long parallel_max;
  bool parallel_connect;
  mm_array_ptr<char> help_category;            /* The help category, if set */
  mm_ptr<struct OperationConfig> first;
  mm_ptr<struct OperationConfig> current;
  mm_ptr<struct OperationConfig> last;   /* Always last in the struct */
};

void config_init(mm_ptr<struct OperationConfig> config);
void config_free(mm_ptr<struct OperationConfig> config);

#endif /* HEADER_CURL_TOOL_CFGABLE_H */
