#ifndef HEADER_CURL_TOOL_OPERHLP_H
#define HEADER_CURL_TOOL_OPERHLP_H
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

struct OperationConfig;

void clean_getout(mm_ptr<struct OperationConfig> config);

bool output_expected(mm_array_ptr<const char> url, mm_array_ptr<const char> uploadfile);

bool stdin_upload(mm_array_ptr<const char> uploadfile);

mm_array_ptr<char> add_file_name_to_url(mm_array_ptr<char> url, mm_array_ptr<const char> filename);

CURLcode get_url_file_name(mm_ptr<mm_array_ptr<char>> filename, mm_array_ptr<const char> url);

#endif /* HEADER_CURL_TOOL_OPERHLP_H */
