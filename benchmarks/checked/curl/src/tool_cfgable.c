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

#include "tool_cfgable.h"
#include "tool_main.h"

#include "memdebug.h" /* keep this as LAST include */

void config_init(mm_ptr<struct OperationConfig> config)
{
  memset(_GETPTR(struct OperationConfig, config), 0, sizeof(struct OperationConfig));

  config->postfieldsize = -1;
  config->use_httpget = FALSE;
  config->create_dirs = FALSE;
  config->maxredirs = DEFAULT_MAXREDIRS;
  config->proto = CURLPROTO_ALL;
  config->proto_present = FALSE;
  config->proto_redir = CURLPROTO_ALL & /* All except FILE, SCP and SMB */
    ~(CURLPROTO_FILE | CURLPROTO_SCP | CURLPROTO_SMB |
      CURLPROTO_SMBS);
  config->proto_redir_present = FALSE;
  config->proto_default = NULL;
  config->tcp_nodelay = TRUE; /* enabled by default */
  config->happy_eyeballs_timeout_ms = CURL_HET_DEFAULT;
  config->http09_allowed = FALSE;
  config->ftp_skip_ip = TRUE;
}

static void free_config_fields(mm_ptr<struct OperationConfig> config)
{
  mm_ptr<struct getout> urlnode = NULL;

  mm_Curl_safefree(char, config->random_file);
  mm_Curl_safefree(char, config->egd_file);
  mm_Curl_safefree(char, config->useragent);
  mm_Curl_safefree(char, config->altsvc);
  mm_Curl_safefree(char, config->hsts);
  curl_slist_free_all(config->cookies);
  mm_Curl_safefree(char, config->cookiejar);
  curl_slist_free_all(config->cookiefiles);

  mm_Curl_safefree(char, config->postfields);
  mm_Curl_safefree(char, config->referer);

  mm_Curl_safefree(char, config->headerfile);
  mm_Curl_safefree(char, config->ftpport);
  mm_Curl_safefree(char, config->iface);

  mm_Curl_safefree(char, config->range);

  mm_Curl_safefree(char, config->userpwd);
  mm_Curl_safefree(char, config->tls_username);
  mm_Curl_safefree(char, config->tls_password);
  mm_Curl_safefree(char, config->tls_authtype);
  mm_Curl_safefree(char, config->proxy_tls_username);
  mm_Curl_safefree(char, config->proxy_tls_password);
  mm_Curl_safefree(char, config->proxy_tls_authtype);
  mm_Curl_safefree(char, config->proxyuserpwd);
  mm_Curl_safefree(char, config->proxy);

  mm_Curl_safefree(char, config->dns_ipv6_addr);
  mm_Curl_safefree(char, config->dns_ipv4_addr);
  mm_Curl_safefree(char, config->dns_interface);
  mm_Curl_safefree(char, config->dns_servers);

  mm_Curl_safefree(char, config->noproxy);

  mm_Curl_safefree(char, config->mail_from);
  curl_slist_free_all(config->mail_rcpt);
  mm_Curl_safefree(char, config->mail_auth);

  mm_Curl_safefree(char, config->netrc_file);
  mm_Curl_safefree(char, config->output_dir);

  urlnode = config->url_list;
  while(urlnode) {
    mm_ptr<struct getout> next = urlnode->next;
    mm_Curl_safefree(char, urlnode->url);
    mm_Curl_safefree(char, urlnode->outfile);
    mm_Curl_safefree(char, urlnode->infile);
    mm_Curl_safefree(struct getout, urlnode);
    urlnode = next;
  }
  config->url_list = NULL;
  config->url_last = NULL;
  config->url_get = NULL;
  config->url_out = NULL;

  mm_Curl_safefree(char, config->doh_url);
  mm_Curl_safefree(char, config->cipher_list);
  mm_Curl_safefree(char, config->proxy_cipher_list);
  mm_Curl_safefree(char, config->cert);
  mm_Curl_safefree(char, config->proxy_cert);
  mm_Curl_safefree(char, config->cert_type);
  mm_Curl_safefree(char, config->proxy_cert_type);
  mm_Curl_safefree(char, config->cacert);
  mm_Curl_safefree(char, config->login_options);
  mm_Curl_safefree(char, config->proxy_cacert);
  mm_Curl_safefree(char, config->capath);
  mm_Curl_safefree(char, config->proxy_capath);
  mm_Curl_safefree(char, config->crlfile);
  mm_Curl_safefree(char, config->pinnedpubkey);
  mm_Curl_safefree(char, config->proxy_pinnedpubkey);
  mm_Curl_safefree(char, config->proxy_crlfile);
  mm_Curl_safefree(char, config->key);
  mm_Curl_safefree(char, config->proxy_key);
  mm_Curl_safefree(char, config->key_type);
  mm_Curl_safefree(char, config->proxy_key_type);
  mm_Curl_safefree(char, config->key_passwd);
  mm_Curl_safefree(char, config->proxy_key_passwd);
  mm_Curl_safefree(char, config->pubkey);
  mm_Curl_safefree(char, config->hostpubmd5);
  mm_Curl_safefree(char, config->engine);
  mm_Curl_safefree(char, config->etag_save_file);
  mm_Curl_safefree(char, config->etag_compare_file);
  mm_Curl_safefree(char, config->request_target);
  mm_Curl_safefree(char, config->customrequest);
  mm_Curl_safefree(char, config->krblevel);

  mm_Curl_safefree(char, config->oauth_bearer);
  mm_Curl_safefree(char, config->sasl_authzid);

  mm_Curl_safefree(char, config->unix_socket_path);
  mm_Curl_safefree(char, config->writeout);
  mm_Curl_safefree(char, config->proto_default);

  curl_slist_free_all(config->quote);
  curl_slist_free_all(config->postquote);
  curl_slist_free_all(config->prequote);

  curl_slist_free_all(config->headers);
  curl_slist_free_all(config->proxyheaders);

  curl_mime_free(config->mimepost);
  config->mimepost = NULL;
  tool_mime_free(config->mimeroot);
  config->mimeroot = NULL;
  config->mimecurrent = NULL;

  curl_slist_free_all(config->telnet_options);
  curl_slist_free_all(config->resolve);
  curl_slist_free_all(config->connect_to);

  mm_Curl_safefree(char, config->preproxy);
  mm_Curl_safefree(char, config->proxy_service_name);
  mm_Curl_safefree(char, config->service_name);

  mm_Curl_safefree(char, config->ftp_account);
  mm_Curl_safefree(char, config->ftp_alternative_to_user);

  mm_Curl_safefree(char, config->aws_sigv4);
}

void config_free(mm_ptr<struct OperationConfig> config)
{
  mm_ptr<struct OperationConfig> last = config;

  /* Free each of the structures in reverse order */
  while(last) {
    mm_ptr<struct OperationConfig> prev = last->prev;

    free_config_fields(last);
    MM_FREE(struct OperationConfig, last);

    last = prev;
  }
}
