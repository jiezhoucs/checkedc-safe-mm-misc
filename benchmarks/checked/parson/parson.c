/*
 SPDX-License-Identifier: MIT

 Parson 1.1.3 ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 - 2021 Krzysztof Gabis

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/
#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#include "parson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

/* Apparently sscanf is not implemented in some "standard" libraries, so don't use it, if you
 * don't have to. */
#define sscanf THINK_TWICE_ABOUT_USING_SSCANF

#define STARTING_CAPACITY 16
#define MAX_NESTING       2048

#define FLOAT_FORMAT "%1.17g" /* do not increase precision without incresing NUM_BUF_SIZE */
#define NUM_BUF_SIZE 64 /* double printed with "%1.17g" shouldn't be longer than 25 bytes so let's be paranoid and use 64 */

#define SIZEOF_TOKEN(a)       (sizeof(a) - 1)
#define SKIP_CHAR(str)        ((*str)++)
#define SKIP_WHITESPACES(str) while (isspace((unsigned char)(**str))) { SKIP_CHAR(str); }
#define MAX(a, b)             ((a) > (b) ? (a) : (b))

#undef malloc
#undef free

#if defined(isnan) && defined(isinf)
#define IS_NUMBER_INVALID(x) (isnan((x)) || isinf((x)))
#else
#define IS_NUMBER_INVALID(x) (((x) * 0.0) != 0.0)
#endif

static JSON_Malloc_Function parson_malloc = malloc;
static JSON_Free_Function parson_free = free;

static int parson_escape_slashes = 1;

#define IS_CONT(b) (((unsigned char)(b) & 0xC0) == 0x80) /* is utf-8 continuation byte */

typedef struct json_string {
    mm_array_ptr<char> chars;
    size_t length;
} JSON_String;

/* Type definitions */
typedef union json_value_value {
    JSON_String  string;
    double       number;
    mm_ptr<JSON_Object> object;
    mm_ptr<JSON_Array> array;
    int          boolean;
    int          null;
} JSON_Value_Value;

struct json_value_t {
    mm_ptr<JSON_Value> parent;
    JSON_Value_Type  type;
    JSON_Value_Value value;
};

struct json_object_t {
    mm_ptr<JSON_Value> wrapping_value;
    mm_array_ptr<mm_array_ptr<char>>  names;
    mm_array_ptr<mm_ptr<JSON_Value>> values;
    size_t       count;
    size_t       capacity;
};

struct json_array_t {
    mm_ptr<JSON_Value> wrapping_value;
    mm_array_ptr<mm_ptr<JSON_Value>> items;
    size_t       count;
    size_t       capacity;
};

/* Various */
static mm_array_ptr<char> read_file(const char *filename);
static void   remove_comments(char *string, const char *start_token, const char *end_token);
static mm_array_ptr<char> parson_strndup(mm_array_ptr<const char> string, size_t n);
static mm_array_ptr<char> parson_strdup(mm_array_ptr<const char> string);
static int    hex_char_to_int(char c);
static int    parse_utf16_hex(const char *string, unsigned int *result);
static int    num_bytes_in_utf8_sequence(unsigned char c);
static int    verify_utf8_sequence(const unsigned char *string, int *len);
static int    is_valid_utf8(const char *string, size_t string_len);
static int    is_decimal(const char *string, size_t length);

/* JSON Object */
static mm_ptr<JSON_Object> json_object_init(mm_ptr<JSON_Value> wrapping_value);
static JSON_Status   json_object_add(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
                                     mm_ptr<JSON_Value> value);
static JSON_Status   json_object_addn(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
                                      size_t name_len, mm_ptr<JSON_Value> value);
static JSON_Status   json_object_resize(mm_ptr<JSON_Object> object, size_t new_capacity);
static mm_ptr<JSON_Value> json_object_getn_value(mm_ptr<const JSON_Object> object,
        mm_array_ptr<const char> name, size_t name_len);
static JSON_Status   json_object_remove_internal(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, int free_value);
static JSON_Status   json_object_dotremove_internal(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, int free_value);
static void          json_object_free(mm_ptr<JSON_Object> object);

/* JSON Array */
static mm_ptr<JSON_Array> json_array_init(mm_ptr<JSON_Value> wrapping_value);
static JSON_Status  json_array_add(mm_ptr<JSON_Array> array, mm_ptr<JSON_Value> value);
static JSON_Status  json_array_resize(mm_ptr<JSON_Array> array, size_t new_capacity);
static void         json_array_free(mm_ptr<JSON_Array> array);

/* JSON Value */
static mm_ptr<JSON_Value> json_value_init_string_no_copy(mm_array_ptr<char> string, size_t length);
static mm_ptr<const JSON_String> json_value_get_string_desc(mm_ptr<const JSON_Value> value);

/* Parser */
static JSON_Status  skip_quotes(mm_array_ptr<const char> *string);
static int          parse_utf16(mm_array_ptr<const char> *unprocessed, mm_array_ptr<char> *processed);
static mm_array_ptr<char> process_string(mm_array_ptr<const char> input, size_t input_len, size_t *output_len);
static mm_array_ptr<char> get_quoted_string(mm_array_ptr<const char> *string, size_t *output_string_len);
static mm_ptr<JSON_Value> parse_object_value(mm_array_ptr<const char> *string, size_t nesting);
static mm_ptr<JSON_Value> parse_array_value(mm_array_ptr<const char> *string, size_t nesting);
static mm_ptr<JSON_Value> parse_string_value(mm_array_ptr<const char> *string);
static mm_ptr<JSON_Value> parse_boolean_value(mm_array_ptr<const char> *string);
static mm_ptr<JSON_Value> parse_number_value(mm_array_ptr<const char> *string);
static mm_ptr<JSON_Value> parse_null_value(mm_array_ptr<const char> *string);
static mm_ptr<JSON_Value> parse_value(mm_array_ptr<const char> *string, size_t nesting);

/* Serialization */
static int    json_serialize_to_buffer_r(mm_ptr<const JSON_Value> value,
        mm_array_ptr<char> buf, int level, int is_pretty, mm_array_ptr<char> num_buf);
static int    json_serialize_string(mm_array_ptr<const char> string, size_t len, mm_array_ptr<char> buf);
static int    append_indent(mm_array_ptr<char> buf, int level);
static int    append_string(char *buf, const char *string);

/* Various */
static mm_array_ptr<char> parson_strndup(mm_array_ptr<const char> string, size_t n) {
    /* We expect the caller has validated that 'n' fits within the input buffer. */
    mm_array_ptr<char> output_string = MM_ARRAY_ALLOC(char, n + 1);
    if (!output_string) {
        return NULL;
    }
    output_string[n] = '\0';
    memcpy(_GETARRAYPTR(char, output_string), _GETARRAYPTR(char, string), n);
    return output_string;
}

static mm_array_ptr<char> parson_strdup(mm_array_ptr<const char> string) {
    return parson_strndup(string, strlen(_GETARRAYPTR(char, string)));
}

static int hex_char_to_int(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/* There is no need to refactor parse_utf16_hex() to use mm_array_ptr */
static int parse_utf16_hex(const char *s, unsigned int *result) {
    int x1, x2, x3, x4;
    if (s[0] == '\0' || s[1] == '\0' || s[2] == '\0' || s[3] == '\0') {
        return 0;
    }
    x1 = hex_char_to_int(s[0]);
    x2 = hex_char_to_int(s[1]);
    x3 = hex_char_to_int(s[2]);
    x4 = hex_char_to_int(s[3]);
    if (x1 == -1 || x2 == -1 || x3 == -1 || x4 == -1) {
        return 0;
    }
    *result = (unsigned int)((x1 << 12) | (x2 << 8) | (x3 << 4) | x4);
    return 1;
}

static int num_bytes_in_utf8_sequence(unsigned char c) {
    if (c == 0xC0 || c == 0xC1 || c > 0xF4 || IS_CONT(c)) {
        return 0;
    } else if ((c & 0x80) == 0) {    /* 0xxxxxxx */
        return 1;
    } else if ((c & 0xE0) == 0xC0) { /* 110xxxxx */
        return 2;
    } else if ((c & 0xF0) == 0xE0) { /* 1110xxxx */
        return 3;
    } else if ((c & 0xF8) == 0xF0) { /* 11110xxx */
        return 4;
    }
    return 0; /* won't happen */
}

static int verify_utf8_sequence(const unsigned char *string, int *len) {
    unsigned int cp = 0;
    *len = num_bytes_in_utf8_sequence(string[0]);

    if (*len == 1) {
        cp = string[0];
    } else if (*len == 2 && IS_CONT(string[1])) {
        cp = string[0] & 0x1F;
        cp = (cp << 6) | (string[1] & 0x3F);
    } else if (*len == 3 && IS_CONT(string[1]) && IS_CONT(string[2])) {
        cp = ((unsigned char)string[0]) & 0xF;
        cp = (cp << 6) | (string[1] & 0x3F);
        cp = (cp << 6) | (string[2] & 0x3F);
    } else if (*len == 4 && IS_CONT(string[1]) && IS_CONT(string[2]) && IS_CONT(string[3])) {
        cp = string[0] & 0x7;
        cp = (cp << 6) | (string[1] & 0x3F);
        cp = (cp << 6) | (string[2] & 0x3F);
        cp = (cp << 6) | (string[3] & 0x3F);
    } else {
        return 0;
    }

    /* overlong encodings */
    if ((cp < 0x80    && *len > 1) ||
        (cp < 0x800   && *len > 2) ||
        (cp < 0x10000 && *len > 3)) {
        return 0;
    }

    /* invalid unicode */
    if (cp > 0x10FFFF) {
        return 0;
    }

    /* surrogate halves */
    if (cp >= 0xD800 && cp <= 0xDFFF) {
        return 0;
    }

    return 1;
}

static int is_valid_utf8(const char *string, size_t string_len) {
    int len = 0;
    const char *string_end =  string + string_len;
    while (string < string_end) {
        if (!verify_utf8_sequence((const unsigned char*)string, &len)) {
            return 0;
        }
        string += len;
    }
    return 1;
}

/* No need to refactor is_decimal() as its argument never escapes. */
static int is_decimal(const char *string, size_t length) {
    if (length > 1 && string[0] == '0' && string[1] != '.') {
        return 0;
    }
    if (length > 2 && !strncmp(string, "-0", 2) && string[2] != '.') {
        return 0;
    }
    while (length--) {
        if (strchr("xX", string[length])) {
            return 0;
        }
    }
    return 1;
}

static mm_array_ptr<char> read_file(const char * filename) {
    FILE *fp = fopen(filename, "r");
    size_t size_to_read = 0;
    size_t size_read = 0;
    long pos;
    mm_array_ptr<char> file_contents = NULL;
    if (!fp) {
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        return NULL;
    }
    size_to_read = pos;
    rewind(fp);
    file_contents = MM_ARRAY_ALLOC(char, size_to_read + 1);
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    size_read = fread(_GETARRAYPTR(char, file_contents), 1, size_to_read, fp);
    if (size_read == 0 || ferror(fp)) {
        fclose(fp);
        MM_ARRAY_FREE(char, file_contents);
        return NULL;
    }
    fclose(fp);
    file_contents[size_read] = '\0';
    return file_contents;
}

/* There is no need to refactor remove_comments() */
static void remove_comments(char *string, const char *start_token, const char *end_token) {
    int in_string = 0, escaped = 0;
    size_t i;
    char *ptr = NULL, current_char;
    size_t start_token_len = strlen(start_token);
    size_t end_token_len = strlen(end_token);
    if (start_token_len == 0 || end_token_len == 0) {
        return;
    }
    while ((current_char = *string) != '\0') {
        if (current_char == '\\' && !escaped) {
            escaped = 1;
            string++;
            continue;
        } else if (current_char == '\"' && !escaped) {
            in_string = !in_string;
        } else if (!in_string && strncmp(string, start_token, start_token_len) == 0) {
            for(i = 0; i < start_token_len; i++) {
                string[i] = ' ';
            }
            string = string + start_token_len;
            ptr = strstr(string, end_token);
            if (!ptr) {
                return;
            }
            for (i = 0; i < (ptr - string) + end_token_len; i++) {
                string[i] = ' ';
            }
            string = ptr + end_token_len - 1;
        }
        escaped = 0;
        string++;
    }
}

/* JSON Object */
static mm_ptr<JSON_Object> json_object_init(mm_ptr<JSON_Value> wrapping_value) {
    mm_ptr<JSON_Object> new_obj = MM_ALLOC(JSON_Object);
    if (new_obj == NULL) {
        return NULL;
    }
    new_obj->wrapping_value = wrapping_value;
    new_obj->names = NULL;
    new_obj->values = NULL;
    new_obj->capacity = 0;
    new_obj->count = 0;
    return new_obj;
}

static JSON_Status json_object_add(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
                                   mm_ptr<JSON_Value> value) {
    if (name == NULL) {
        return JSONFailure;
    }
    return json_object_addn(object, name, strlen(_GETARRAYPTR(char, name)), value);
}

static JSON_Status json_object_addn(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
        size_t name_len, mm_ptr<JSON_Value> value) {
    size_t index = 0;
    if (object == NULL || name == NULL || value == NULL) {
        return JSONFailure;
    }
    if (json_object_getn_value(object, name, name_len) != NULL) {
        return JSONFailure;
    }
    if (object->count >= object->capacity) {
        size_t new_capacity = MAX(object->capacity * 2, STARTING_CAPACITY);
        if (json_object_resize(object, new_capacity) == JSONFailure) {
            return JSONFailure;
        }
    }
    index = object->count;
    object->names[index] = parson_strndup(name, name_len);
    if (object->names[index] == NULL) {
        return JSONFailure;
    }
    value->parent = json_object_get_wrapping_value(object);
    object->values[index] = value;
    object->count++;
    return JSONSuccess;
}

static JSON_Status json_object_resize(mm_ptr<JSON_Object> object, size_t new_capacity) {
    mm_array_ptr<mm_array_ptr<char>> temp_names = NULL;
    mm_array_ptr<mm_ptr<JSON_Value>> temp_values = NULL;

    if ((object->names == NULL && object->values != NULL) ||
        (object->names != NULL && object->values == NULL) ||
        new_capacity == 0) {
            return JSONFailure; /* Shouldn't happen */
    }
    temp_names = MM_ARRAY_ALLOC(mm_array_ptr<char>, new_capacity);
    if (temp_names == NULL) {
        return JSONFailure;
    }
    temp_values = MM_ARRAY_ALLOC(mm_ptr<JSON_Value>, new_capacity);
    if (temp_values == NULL) {
        MM_ARRAY_FREE(mm_array_ptr<char>, temp_names);
        return JSONFailure;
    }
    if (object->names != NULL && object->values != NULL && object->count > 0) {
        memcpy(_GETARRAYPTR(mm_array_ptr<char>, temp_names),
               _GETARRAYPTR(mm_array_ptr<char>, object->names),
               object->count * sizeof(mm_array_ptr<char>));
        memcpy(_GETARRAYPTR(mm_ptr<JSON_Value>, temp_values),
                _GETARRAYPTR(mm_ptr<JSON_Value>, object->values),
                object->count * sizeof(mm_ptr<JSON_Value>));
    }
    MM_ARRAY_FREE(mm_array_ptr<char>, object->names);
    MM_ARRAY_FREE(mm_ptr<JSON_Value>, object->values);
    object->names = temp_names;
    object->values = temp_values;
    object->capacity = new_capacity;
    return JSONSuccess;
}

static mm_ptr<JSON_Value> json_object_getn_value(mm_ptr<const JSON_Object> object,
        mm_array_ptr<const char> name, size_t name_len) {
    size_t i, name_length;
    for (i = 0; i < json_object_get_count(object); i++) {
        name_length = strlen(_GETARRAYPTR(char, object->names[i]));
        if (name_length != name_len) {
            continue;
        }
        if (strncmp(_GETARRAYPTR(char, object->names[i]), _GETARRAYPTR(char, name), name_len) == 0) {
            return object->values[i];
        }
    }
    return NULL;
}

static JSON_Status json_object_remove_internal(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, int free_value) {
    size_t i = 0, last_item_index = 0;
    if (object == NULL || json_object_get_value(object, name) == NULL) {
        return JSONFailure;
    }
    last_item_index = json_object_get_count(object) - 1;
    for (i = 0; i < json_object_get_count(object); i++) {
        if (strcmp(_GETARRAYPTR(char, object->names[i]), _GETARRAYPTR(char, name)) == 0) {
            MM_ARRAY_FREE(char, object->names[i]);
            if (free_value) {
                json_value_free(object->values[i]);
            }
            if (i != last_item_index) { /* Replace key value pair with one from the end */
                object->names[i] = object->names[last_item_index];
                object->values[i] = object->values[last_item_index];
            }
            object->count -= 1;
            return JSONSuccess;
        }
    }
    return JSONFailure; /* No execution path should end here */
}

static JSON_Status json_object_dotremove_internal(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, int free_value) {
    mm_ptr<JSON_Value> temp_value = NULL;
    mm_ptr<JSON_Object> temp_object = NULL;
    const char *dot_pos_raw = strchr(_GETARRAYPTR(char, name), '.');
    if (dot_pos_raw == NULL) {
        return json_object_remove_internal(object, name, free_value);
    }
    mm_array_ptr<const char> dot_pos = name + (dot_pos_raw - (char *)(_GETARRAYPTR(char, name)));
    temp_value = json_object_getn_value(object, name, dot_pos - name);
    if (json_value_get_type(temp_value) != JSONObject) {
        return JSONFailure;
    }
    temp_object = json_value_get_object(temp_value);
    return json_object_dotremove_internal(temp_object, dot_pos + 1, free_value);
}

static void json_object_free(mm_ptr<JSON_Object> object) {
    size_t i;
    for (i = 0; i < object->count; i++) {
        MM_ARRAY_FREE(char, object->names[i]);
        json_value_free(object->values[i]);
    }
    MM_ARRAY_FREE(mm_array_ptr<char>, object->names);
    MM_ARRAY_FREE(mm_ptr<JSON_Value>, object->values);
    MM_FREE(JSON_Object, object);
}

/* JSON Array */
static mm_ptr<JSON_Array> json_array_init(mm_ptr<JSON_Value> wrapping_value) {
    mm_ptr<JSON_Array> new_array = MM_ALLOC(JSON_Array);
    if (new_array == NULL) {
        return NULL;
    }
    new_array->wrapping_value = wrapping_value;
    new_array->items = NULL;
    new_array->capacity = 0;
    new_array->count = 0;
    return new_array;
}

static JSON_Status json_array_add(mm_ptr<JSON_Array> array, mm_ptr<JSON_Value> value) {
    if (array->count >= array->capacity) {
        size_t new_capacity = MAX(array->capacity * 2, STARTING_CAPACITY);
        if (json_array_resize(array, new_capacity) == JSONFailure) {
            return JSONFailure;
        }
    }
    value->parent = json_array_get_wrapping_value(array);
    array->items[array->count] = value;
    array->count++;
    return JSONSuccess;
}

static JSON_Status json_array_resize(mm_ptr<JSON_Array> array, size_t new_capacity) {
    mm_array_ptr<mm_ptr<JSON_Value>> new_items = NULL;
    if (new_capacity == 0) {
        return JSONFailure;
    }
    new_items = MM_ARRAY_ALLOC(mm_ptr<JSON_Value>, new_capacity);
    if (new_items == NULL) {
        return JSONFailure;
    }
    if (array->items != NULL && array->count > 0) {
        memcpy(_GETARRAYPTR(mm_ptr<JSON_Value>, new_items),
               _GETARRAYPTR(mm_ptr<JSON_Value>, array->items),
               array->count * sizeof(mm_ptr<JSON_Value>));
    }
    MM_ARRAY_FREE(mm_ptr<JSON_Value>, array->items);
    array->items = new_items;
    array->capacity = new_capacity;
    return JSONSuccess;
}

static void json_array_free(mm_ptr<JSON_Array> array) {
    size_t i;
    for (i = 0; i < array->count; i++) {
        json_value_free(array->items[i]);
    }
    MM_ARRAY_FREE(mm_ptr<JSON_Value>, array->items);
    MM_FREE(JSON_Array, array);
}

/* JSON Value */
static mm_ptr<JSON_Value> json_value_init_string_no_copy(mm_array_ptr<char> string, size_t length) {
    mm_ptr<JSON_Value> new_value = MM_ALLOC(JSON_Value);
    if (!new_value) {
        return NULL;
    }
    new_value->parent = NULL;
    new_value->type = JSONString;
    new_value->value.string.chars = string;
    new_value->value.string.length = length;
    return new_value;
}

/* Parser */
static JSON_Status skip_quotes(mm_array_ptr<const char> *string) {
    if (**string != '\"') {
        return JSONFailure;
    }
    SKIP_CHAR(string);
    while (**string != '\"') {
        if (**string == '\0') {
            return JSONFailure;
        } else if (**string == '\\') {
            SKIP_CHAR(string);
            if (**string == '\0') {
                return JSONFailure;
            }
        }
        SKIP_CHAR(string);
    }
    SKIP_CHAR(string);
    return JSONSuccess;
}

static int parse_utf16(mm_array_ptr<const char> *unprocessed, mm_array_ptr<char> *processed) {
    unsigned int cp, lead, trail;
    int parse_succeeded = 0;
    mm_array_ptr<char> processed_ptr = *processed;
    mm_array_ptr<const char> unprocessed_ptr = *unprocessed;
    unprocessed_ptr++; /* skips u */
    parse_succeeded = parse_utf16_hex(_GETARRAYPTR(char, unprocessed_ptr), &cp);
    if (!parse_succeeded) {
        return JSONFailure;
    }
    if (cp < 0x80) {
        processed_ptr[0] = (char)cp; /* 0xxxxxxx */
    } else if (cp < 0x800) {
        processed_ptr[0] = ((cp >> 6) & 0x1F) | 0xC0; /* 110xxxxx */
        processed_ptr[1] = ((cp)      & 0x3F) | 0x80; /* 10xxxxxx */
        processed_ptr += 1;
    } else if (cp < 0xD800 || cp > 0xDFFF) {
        processed_ptr[0] = ((cp >> 12) & 0x0F) | 0xE0; /* 1110xxxx */
        processed_ptr[1] = ((cp >> 6)  & 0x3F) | 0x80; /* 10xxxxxx */
        processed_ptr[2] = ((cp)       & 0x3F) | 0x80; /* 10xxxxxx */
        processed_ptr += 2;
    } else if (cp >= 0xD800 && cp <= 0xDBFF) { /* lead surrogate (0xD800..0xDBFF) */
        lead = cp;
        unprocessed_ptr += 4; /* should always be within the buffer, otherwise previous sscanf would fail */
        if (*unprocessed_ptr++ != '\\' || *unprocessed_ptr++ != 'u') {
            return JSONFailure;
        }
        parse_succeeded = parse_utf16_hex(_GETARRAYPTR(char, unprocessed_ptr), &trail);
        if (!parse_succeeded || trail < 0xDC00 || trail > 0xDFFF) { /* valid trail surrogate? (0xDC00..0xDFFF) */
            return JSONFailure;
        }
        cp = ((((lead - 0xD800) & 0x3FF) << 10) | ((trail - 0xDC00) & 0x3FF)) + 0x010000;
        processed_ptr[0] = (((cp >> 18) & 0x07) | 0xF0); /* 11110xxx */
        processed_ptr[1] = (((cp >> 12) & 0x3F) | 0x80); /* 10xxxxxx */
        processed_ptr[2] = (((cp >> 6)  & 0x3F) | 0x80); /* 10xxxxxx */
        processed_ptr[3] = (((cp)       & 0x3F) | 0x80); /* 10xxxxxx */
        processed_ptr += 3;
    } else { /* trail surrogate before lead surrogate */
        return JSONFailure;
    }
    unprocessed_ptr += 3;
    *processed = processed_ptr;
    *unprocessed = unprocessed_ptr;
    return JSONSuccess;
}


/* Copies and processes passed string up to supplied length.
Example: "\u006Corem ipsum" -> lorem ipsum */
static mm_array_ptr<char> process_string(mm_array_ptr<const char> input, size_t input_len, size_t *output_len) {
    mm_array_ptr<const char> input_ptr = input;
    size_t initial_size = (input_len + 1) * sizeof(char);
    size_t final_size = 0;
    mm_array_ptr<char> output = NULL, output_ptr = NULL, resized_output = NULL;
    output = MM_ARRAY_ALLOC(char, initial_size);
    if (output == NULL) {
        goto error;
    }
    output_ptr = output;
    while ((*input_ptr != '\0') && (size_t)(input_ptr - input) < input_len) {
        if (*input_ptr == '\\') {
            input_ptr++;
            switch (*input_ptr) {
                case '\"': *output_ptr = '\"'; break;
                case '\\': *output_ptr = '\\'; break;
                case '/':  *output_ptr = '/';  break;
                case 'b':  *output_ptr = '\b'; break;
                case 'f':  *output_ptr = '\f'; break;
                case 'n':  *output_ptr = '\n'; break;
                case 'r':  *output_ptr = '\r'; break;
                case 't':  *output_ptr = '\t'; break;
                case 'u':
                    if (parse_utf16(&input_ptr, &output_ptr) == JSONFailure) {
                        goto error;
                    }
                    break;
                default:
                    goto error;
            }
        } else if ((unsigned char)*input_ptr < 0x20) {
            goto error; /* 0x00-0x19 are invalid characters for json string (http://www.ietf.org/rfc/rfc4627.txt) */
        } else {
            *output_ptr = *input_ptr;
        }
        output_ptr++;
        input_ptr++;
    }
    *output_ptr = '\0';
    /* resize to new length */
    final_size = (size_t)(output_ptr-output) + 1;
    /* todo: don't resize if final_size == initial_size */
    resized_output = MM_ARRAY_ALLOC(char, final_size);
    if (resized_output == NULL) {
        goto error;
    }
    memcpy(_GETARRAYPTR(char, resized_output), _GETARRAYPTR(char, output), final_size);
    *output_len = final_size - 1;
    MM_ARRAY_FREE(char, output);
    return resized_output;
error:
    MM_ARRAY_FREE(char, output);
    return NULL;
}

/* Return processed contents of a string between quotes and
   skips passed argument to a matching quote. */
static mm_array_ptr<char> get_quoted_string(mm_array_ptr<const char> *string, size_t *output_string_len) {
    mm_array_ptr<const char> string_start = *string;
    size_t input_string_len = 0;
    JSON_Status status = skip_quotes(string);
    if (status != JSONSuccess) {
        return NULL;
    }
    input_string_len = *string - string_start - 2; /* length without quotes */
    return process_string(string_start + 1, input_string_len, output_string_len);
}

static mm_ptr<JSON_Value> parse_value(mm_array_ptr<const char> *string, size_t nesting) {
    if (nesting > MAX_NESTING) {
        return NULL;
    }
    SKIP_WHITESPACES(string);
    switch (**string) {
        case '{':
            return parse_object_value(string, nesting + 1);
        case '[':
            return parse_array_value(string, nesting + 1);
        case '\"':
            return parse_string_value(string);
        case 'f': case 't':
            return parse_boolean_value(string);
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parse_number_value(string);
        case 'n':
            return parse_null_value(string);
        default:
            return NULL;
    }
}

static mm_ptr<JSON_Value> parse_object_value(mm_array_ptr<const char> *string, size_t nesting) {
    mm_ptr<JSON_Value> output_value = NULL, new_value = NULL;
    mm_ptr<JSON_Object> output_object = NULL;
    mm_array_ptr<char> new_key = NULL;
    output_value = json_value_init_object();
    if (output_value == NULL) {
        return NULL;
    }
    if (**string != '{') {
        json_value_free(output_value);
        return NULL;
    }
    output_object = json_value_get_object(output_value);
    SKIP_CHAR(string);
    SKIP_WHITESPACES(string);
    if (**string == '}') { /* empty object */
        SKIP_CHAR(string);
        return output_value;
    }
    while (**string != '\0') {
        size_t key_len = 0;
        new_key = get_quoted_string(string, &key_len);
        /* We do not support key names with embedded \0 chars */
        if (new_key == NULL || key_len != strlen(_GETARRAYPTR(char, new_key))) {
            if (new_key) {
                MM_ARRAY_FREE(char, new_key);
            }
            json_value_free(output_value);
            return NULL;
        }
        SKIP_WHITESPACES(string);
        if (**string != ':') {
            MM_ARRAY_FREE(char, new_key);
            json_value_free(output_value);
            return NULL;
        }
        SKIP_CHAR(string);
        new_value = parse_value(string, nesting);
        if (new_value == NULL) {
            MM_ARRAY_FREE(char, new_key);
            json_value_free(output_value);
            return NULL;
        }
        if (json_object_add(output_object, new_key, new_value) == JSONFailure) {
            MM_ARRAY_FREE(char, new_key);
            json_value_free(new_value);
            json_value_free(output_value);
            return NULL;
        }
        MM_ARRAY_FREE(char, new_key);
        SKIP_WHITESPACES(string);
        if (**string != ',') {
            break;
        }
        SKIP_CHAR(string);
        SKIP_WHITESPACES(string);
    }
    SKIP_WHITESPACES(string);
    if (**string != '}' || /* Trim object after parsing is over */
        json_object_resize(output_object, json_object_get_count(output_object)) == JSONFailure) {
            json_value_free(output_value);
            return NULL;
    }
    SKIP_CHAR(string);
    return output_value;
}

static mm_ptr<JSON_Value> parse_array_value(mm_array_ptr<const char> *string, size_t nesting) {
    mm_ptr<JSON_Value> output_value = NULL, new_array_value = NULL;
    mm_ptr<JSON_Array> output_array = NULL;
    output_value = json_value_init_array();
    if (output_value == NULL) {
        return NULL;
    }
    if (**string != '[') {
        json_value_free(output_value);
        return NULL;
    }
    output_array = json_value_get_array(output_value);
    SKIP_CHAR(string);
    SKIP_WHITESPACES(string);
    if (**string == ']') { /* empty array */
        SKIP_CHAR(string);
        return output_value;
    }
    while (**string != '\0') {
        new_array_value = parse_value(string, nesting);
        if (new_array_value == NULL) {
            json_value_free(output_value);
            return NULL;
        }
        if (json_array_add(output_array, new_array_value) == JSONFailure) {
            json_value_free(new_array_value);
            json_value_free(output_value);
            return NULL;
        }
        SKIP_WHITESPACES(string);
        if (**string != ',') {
            break;
        }
        SKIP_CHAR(string);
        SKIP_WHITESPACES(string);
    }
    SKIP_WHITESPACES(string);
    if (**string != ']' || /* Trim array after parsing is over */
        json_array_resize(output_array, json_array_get_count(output_array)) == JSONFailure) {
            json_value_free(output_value);
            return NULL;
    }
    SKIP_CHAR(string);
    return output_value;
}

static mm_ptr<JSON_Value> parse_string_value(mm_array_ptr<const char> *string) {
    mm_ptr<JSON_Value> value = NULL;
    size_t new_string_len = 0;
    mm_array_ptr<char> new_string = get_quoted_string(string, &new_string_len);
    if (new_string == NULL) {
        return NULL;
    }
    value = json_value_init_string_no_copy(new_string, new_string_len);
    if (value == NULL) {
        MM_ARRAY_FREE(char, new_string);
        return NULL;
    }
    return value;
}

static mm_ptr<JSON_Value> parse_boolean_value(mm_array_ptr<const char> *string) {
    size_t true_token_size = SIZEOF_TOKEN("true");
    size_t false_token_size = SIZEOF_TOKEN("false");
    if (strncmp("true", _GETARRAYPTR(char, *string), true_token_size) == 0) {
        *string += true_token_size;
        return json_value_init_boolean(1);
    } else if (strncmp("false", _GETARRAYPTR(char, *string), false_token_size) == 0) {
        *string += false_token_size;
        return json_value_init_boolean(0);
    }
    return NULL;
}

static mm_ptr<JSON_Value> parse_number_value(mm_array_ptr<const char> *string) {
    char *end;
    double number = 0;
    errno = 0;
    number = strtod(_GETARRAYPTR(char, *string), &end);
    if (errno == ERANGE && (number == -HUGE_VAL || number == HUGE_VAL)) {
        return NULL;
    }
    if ((errno && errno != ERANGE) ||
         !is_decimal((char *)(_GETARRAYPTR(char, *string)),
             end - (char *)(_GETARRAYPTR(char, *string)))) {
        return NULL;
    }
    _setptr_mm_array<char>(string, end);
    return json_value_init_number(number);
}

static mm_ptr<JSON_Value> parse_null_value(mm_array_ptr<const char> *string) {
    size_t token_size = SIZEOF_TOKEN("null");
    if (strncmp("null", _GETARRAYPTR(char, *string), token_size) == 0) {
        *string += token_size;
        return json_value_init_null();
    }
    return NULL;
}

/* Serialization */
#define APPEND_STRING(str) do { written = append_string(_GETARRAYPTR(char, buf), (str));\
                                if (written < 0) { return -1; }\
                                if (buf != NULL) { buf += written; }\
                                written_total += written; } while(0)

#define APPEND_INDENT(level) do { written = append_indent(buf, (level));\
                                  if (written < 0) { return -1; }\
                                  if (buf != NULL) { buf += written; }\
                                  written_total += written; } while(0)

static int json_serialize_to_buffer_r(mm_ptr<const JSON_Value> value,
        mm_array_ptr<char> buf, int level, int is_pretty, mm_array_ptr<char> num_buf)
{
    mm_array_ptr<const char> key = NULL, string = NULL;
    mm_ptr<JSON_Value> temp_value = NULL;
    mm_ptr<JSON_Array> array = NULL;
    mm_ptr<JSON_Object> object = NULL;
    size_t i = 0, count = 0;
    double num = 0.0;
    int written = -1, written_total = 0;
    size_t len = 0;

    switch (json_value_get_type(value)) {
        case JSONArray:
            array = json_value_get_array(value);
            count = json_array_get_count(array);
            APPEND_STRING("[");
            if (count > 0 && is_pretty) {
                APPEND_STRING("\n");
            }
            for (i = 0; i < count; i++) {
                if (is_pretty) {
                    APPEND_INDENT(level+1);
                }
                temp_value = json_array_get_value(array, i);
                written = json_serialize_to_buffer_r(temp_value, buf, level+1, is_pretty, num_buf);
                if (written < 0) {
                    return -1;
                }
                if (buf != NULL) {
                    buf += written;
                }
                written_total += written;
                if (i < (count - 1)) {
                    APPEND_STRING(",");
                }
                if (is_pretty) {
                    APPEND_STRING("\n");
                }
            }
            if (count > 0 && is_pretty) {
                APPEND_INDENT(level);
            }
            APPEND_STRING("]");
            return written_total;
        case JSONObject:
            object = json_value_get_object(value);
            count  = json_object_get_count(object);
            APPEND_STRING("{");
            if (count > 0 && is_pretty) {
                APPEND_STRING("\n");
            }
            for (i = 0; i < count; i++) {
                key = json_object_get_name(object, i);
                if (key == NULL) {
                    return -1;
                }
                if (is_pretty) {
                    APPEND_INDENT(level+1);
                }
                /* We do not support key names with embedded \0 chars */
                written = json_serialize_string(key, strlen(_GETARRAYPTR(char, key)), buf);
                if (written < 0) {
                    return -1;
                }
                if (buf != NULL) {
                    buf += written;
                }
                written_total += written;
                APPEND_STRING(":");
                if (is_pretty) {
                    APPEND_STRING(" ");
                }
                temp_value = json_object_get_value_at(object, i);
                written = json_serialize_to_buffer_r(temp_value, buf, level+1, is_pretty, num_buf);
                if (written < 0) {
                    return -1;
                }
                if (buf != NULL) {
                    buf += written;
                }
                written_total += written;
                if (i < (count - 1)) {
                    APPEND_STRING(",");
                }
                if (is_pretty) {
                    APPEND_STRING("\n");
                }
            }
            if (count > 0 && is_pretty) {
                APPEND_INDENT(level);
            }
            APPEND_STRING("}");
            return written_total;
        case JSONString:
            string = json_value_get_string(value);
            if (string == NULL) {
                return -1;
            }
            len = json_value_get_string_len(value);
            written = json_serialize_string(string, len, buf);
            if (written < 0) {
                return -1;
            }
            if (buf != NULL) {
                buf += written;
            }
            written_total += written;
            return written_total;
        case JSONBoolean:
            if (json_value_get_boolean(value)) {
                APPEND_STRING("true");
            } else {
                APPEND_STRING("false");
            }
            return written_total;
        case JSONNumber:
            num = json_value_get_number(value);
            if (buf != NULL) {
                num_buf = buf;
            }
            written = sprintf(_GETARRAYPTR(char, num_buf), FLOAT_FORMAT, num);
            if (written < 0) {
                return -1;
            }
            if (buf != NULL) {
                buf += written;
            }
            written_total += written;
            return written_total;
        case JSONNull:
            APPEND_STRING("null");
            return written_total;
        case JSONError:
            return -1;
        default:
            return -1;
    }
}

static int json_serialize_string(mm_array_ptr<const char> string, size_t len, mm_array_ptr<char> buf) {
    size_t i = 0;
    char c = '\0';
    int written = -1, written_total = 0;
    APPEND_STRING("\"");
    for (i = 0; i < len; i++) {
        c = string[i];
        switch (c) {
            case '\"': APPEND_STRING("\\\""); break;
            case '\\': APPEND_STRING("\\\\"); break;
            case '\b': APPEND_STRING("\\b"); break;
            case '\f': APPEND_STRING("\\f"); break;
            case '\n': APPEND_STRING("\\n"); break;
            case '\r': APPEND_STRING("\\r"); break;
            case '\t': APPEND_STRING("\\t"); break;
            case '\x00': APPEND_STRING("\\u0000"); break;
            case '\x01': APPEND_STRING("\\u0001"); break;
            case '\x02': APPEND_STRING("\\u0002"); break;
            case '\x03': APPEND_STRING("\\u0003"); break;
            case '\x04': APPEND_STRING("\\u0004"); break;
            case '\x05': APPEND_STRING("\\u0005"); break;
            case '\x06': APPEND_STRING("\\u0006"); break;
            case '\x07': APPEND_STRING("\\u0007"); break;
            /* '\x08' duplicate: '\b' */
            /* '\x09' duplicate: '\t' */
            /* '\x0a' duplicate: '\n' */
            case '\x0b': APPEND_STRING("\\u000b"); break;
            /* '\x0c' duplicate: '\f' */
            /* '\x0d' duplicate: '\r' */
            case '\x0e': APPEND_STRING("\\u000e"); break;
            case '\x0f': APPEND_STRING("\\u000f"); break;
            case '\x10': APPEND_STRING("\\u0010"); break;
            case '\x11': APPEND_STRING("\\u0011"); break;
            case '\x12': APPEND_STRING("\\u0012"); break;
            case '\x13': APPEND_STRING("\\u0013"); break;
            case '\x14': APPEND_STRING("\\u0014"); break;
            case '\x15': APPEND_STRING("\\u0015"); break;
            case '\x16': APPEND_STRING("\\u0016"); break;
            case '\x17': APPEND_STRING("\\u0017"); break;
            case '\x18': APPEND_STRING("\\u0018"); break;
            case '\x19': APPEND_STRING("\\u0019"); break;
            case '\x1a': APPEND_STRING("\\u001a"); break;
            case '\x1b': APPEND_STRING("\\u001b"); break;
            case '\x1c': APPEND_STRING("\\u001c"); break;
            case '\x1d': APPEND_STRING("\\u001d"); break;
            case '\x1e': APPEND_STRING("\\u001e"); break;
            case '\x1f': APPEND_STRING("\\u001f"); break;
            case '/':
                if (parson_escape_slashes) {
                    APPEND_STRING("\\/");  /* to make json embeddable in xml\/html */
                } else {
                    APPEND_STRING("/");
                }
                break;
            default:
                if (buf != NULL) {
                    buf[0] = c;
                    buf += 1;
                }
                written_total += 1;
                break;
        }
    }
    APPEND_STRING("\"");
    return written_total;
}

static int append_indent(mm_array_ptr<char> buf, int level) {
    int i;
    int written = -1, written_total = 0;
    for (i = 0; i < level; i++) {
        APPEND_STRING("    ");
    }
    return written_total;
}

static int append_string(char *buf, const char *string) {
    if (buf == NULL) {
        return (int)strlen(string);
    }
    return sprintf(buf, "%s", string);
}

#undef APPEND_STRING
#undef APPEND_INDENT

/* Parser API */
mm_ptr<JSON_Value>  json_parse_file(const char *filename) {
    mm_array_ptr<char> file_contents = read_file(filename);
    mm_ptr<JSON_Value> output_value = NULL;
    if (file_contents == NULL) {
        return NULL;
    }
    output_value = json_parse_string(file_contents);
    MM_ARRAY_FREE(char, file_contents);
    return output_value;
}

mm_ptr<JSON_Value> json_parse_file_with_comments(const char *filename) {
    mm_array_ptr<char> file_contents = read_file(filename);
    mm_ptr<JSON_Value> output_value = NULL;
    if (file_contents == NULL) {
        return NULL;
    }
    output_value = json_parse_string_with_comments(file_contents);
    MM_ARRAY_FREE(char, file_contents);
    return output_value;
}

mm_ptr<JSON_Value> json_parse_string(mm_array_ptr<const char> string) {
    if (string == NULL) {
        return NULL;
    }
    if (string[0] == '\xEF' && string[1] == '\xBB' && string[2] == '\xBF') {
        string = string + 3; /* Support for UTF-8 BOM */
    }
    return parse_value(&string, 0);
}

mm_ptr<JSON_Value> json_parse_string_with_comments(mm_array_ptr<const char> string) {
    mm_ptr<JSON_Value> result = NULL;
    mm_array_ptr<const char> string_mutable_copy = NULL, string_mutable_copy_ptr = NULL;
    string_mutable_copy = parson_strdup(string);
    if (string_mutable_copy == NULL) {
        return NULL;
    }
    /* No need to refactor remove_comments */
    remove_comments(_GETARRAYPTR(char, string_mutable_copy), "/*", "*/");
    remove_comments(_GETARRAYPTR(char, string_mutable_copy), "//", "\n");
    string_mutable_copy_ptr = string_mutable_copy;
    result = parse_value(&string_mutable_copy_ptr, 0);
    MM_ARRAY_FREE(char, string_mutable_copy);
    return result;
}

/* JSON Object API */

mm_ptr<JSON_Value> json_object_get_value(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    if (object == NULL || name == NULL) {
        return NULL;
    }
    return json_object_getn_value(object, name, strlen(_GETARRAYPTR(char, name)));
}

mm_array_ptr<const char> json_object_get_string(mm_ptr<const JSON_Object> object,
                                                mm_array_ptr<const char> name) {
    return json_value_get_string(json_object_get_value(object, name));
}

size_t json_object_get_string_len(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_string_len(json_object_get_value(object, name));
}

double json_object_get_number(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_number(json_object_get_value(object, name));
}

mm_ptr<JSON_Object> json_object_get_object(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_object(json_object_get_value(object, name));
}

mm_ptr<JSON_Array> json_object_get_array(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_array(json_object_get_value(object, name));
}

int json_object_get_boolean(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_boolean(json_object_get_value(object, name));
}

mm_ptr<JSON_Value> json_object_dotget_value(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    /* FIXME: Implement an bounds-safe interface for strchr */
    const char *dot_position_raw = strchr(_GETARRAYPTR(char, name), '.');
    if (!dot_position_raw) {
        return json_object_get_value(object, name);
    }
    mm_array_ptr<const char> dot_position = name + (dot_position_raw - (char *)_GETARRAYPTR(char, name));
    object = json_value_get_object(json_object_getn_value(object, name, dot_position - name));
    return json_object_dotget_value(object, dot_position + 1);
}

mm_array_ptr<const char> json_object_dotget_string(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_string(json_object_dotget_value(object, name));
}

size_t json_object_dotget_string_len(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_string_len(json_object_dotget_value(object, name));
}

double json_object_dotget_number(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_number(json_object_dotget_value(object, name));
}

mm_ptr<JSON_Object> json_object_dotget_object(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_object(json_object_dotget_value(object, name));
}

mm_ptr<JSON_Array>  json_object_dotget_array(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_array(json_object_dotget_value(object, name));
}

int json_object_dotget_boolean(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_value_get_boolean(json_object_dotget_value(object, name));
}

size_t json_object_get_count(mm_ptr<const JSON_Object> object) {
    return object ? object->count : 0;
}

mm_array_ptr<const char> json_object_get_name(mm_ptr<const JSON_Object> object, size_t index) {
    if (object == NULL || index >= json_object_get_count(object)) {
        return NULL;
    }
    return object->names[index];
}

mm_ptr<JSON_Value> json_object_get_value_at(mm_ptr<const JSON_Object> object, size_t index) {
    if (object == NULL || index >= json_object_get_count(object)) {
        return NULL;
    }
    return object->values[index];
}

mm_ptr<JSON_Value> json_object_get_wrapping_value(mm_ptr<const JSON_Object> object) {
    return object->wrapping_value;
}

int json_object_has_value (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_object_get_value(object, name) != NULL;
}

int json_object_has_value_of_type(mm_ptr<const JSON_Object> object,
        mm_array_ptr<const char> name, JSON_Value_Type type) {
    mm_ptr<JSON_Value> val = json_object_get_value(object, name);
    return val != NULL && json_value_get_type(val) == type;
}

int json_object_dothas_value(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name) {
    return json_object_dotget_value(object, name) != NULL;
}

int json_object_dothas_value_of_type(mm_ptr<const JSON_Object> object,
        mm_array_ptr<const char> name, JSON_Value_Type type) {
    mm_ptr<JSON_Value> val = json_object_dotget_value(object, name);
    return val != NULL && json_value_get_type(val) == type;
}

/* JSON Array API */
mm_ptr<JSON_Value> json_array_get_value(mm_ptr<const JSON_Array> array, size_t index) {
    if (array == NULL || index >= json_array_get_count(array)) {
        return NULL;
    }
    return array->items[index];
}

mm_array_ptr<const char> json_array_get_string(mm_ptr<const JSON_Array> array, size_t index) {
    return json_value_get_string(json_array_get_value(array, index));
}

size_t json_array_get_string_len(mm_ptr<const JSON_Array> array, size_t index) {
    return json_value_get_string_len(json_array_get_value(array, index));
}

double json_array_get_number(mm_ptr<const JSON_Array> array, size_t index) {
    return json_value_get_number(json_array_get_value(array, index));
}

mm_ptr<JSON_Object> json_array_get_object(mm_ptr<const JSON_Array> array, size_t index) {
    return json_value_get_object(json_array_get_value(array, index));
}

mm_ptr<JSON_Array>  json_array_get_array(mm_ptr<const JSON_Array> array, size_t index) {
    return json_value_get_array(json_array_get_value(array, index));
}

int json_array_get_boolean(mm_ptr<const JSON_Array> array, size_t index) {
    return json_value_get_boolean(json_array_get_value(array, index));
}

size_t json_array_get_count(mm_ptr<const JSON_Array> array) {
    return array ? array->count : 0;
}

mm_ptr<JSON_Value> json_array_get_wrapping_value(mm_ptr<const JSON_Array> array) {
    return array->wrapping_value;
}

/* JSON Value API */
JSON_Value_Type json_value_get_type(mm_ptr<const JSON_Value> value) {
    return value ? value->type : JSONError;
}

mm_ptr<JSON_Object> json_value_get_object(mm_ptr<const JSON_Value> value) {
    return json_value_get_type(value) == JSONObject ? value->value.object : NULL;
}

mm_ptr<JSON_Array> json_value_get_array(mm_ptr<const JSON_Value> value) {
    return json_value_get_type(value) == JSONArray ? value->value.array : NULL;
}

static mm_ptr<const JSON_String> json_value_get_string_desc(mm_ptr<const JSON_Value> value) {
    return json_value_get_type(value) == JSONString ? &value->value.string : NULL;
}

mm_array_ptr<const char> json_value_get_string(mm_ptr<const JSON_Value> value) {
    mm_ptr<const JSON_String> str = json_value_get_string_desc(value);
    return str ? str->chars : NULL;
}

size_t json_value_get_string_len(mm_ptr<const JSON_Value> value) {
    mm_ptr<const JSON_String> str = json_value_get_string_desc(value);
    return str ? str->length : 0;
}

double json_value_get_number(mm_ptr<const JSON_Value> value) {
    return json_value_get_type(value) == JSONNumber ? value->value.number : 0;
}

int json_value_get_boolean(mm_ptr<const JSON_Value> value) {
    return json_value_get_type(value) == JSONBoolean ? value->value.boolean : -1;
}

mm_ptr<JSON_Value> json_value_get_parent (mm_ptr<const JSON_Value> value) {
    return value ? value->parent : NULL;
}

void json_value_free(mm_ptr<JSON_Value> value) {
    switch (json_value_get_type(value)) {
        case JSONObject:
            json_object_free(value->value.object);
            break;
        case JSONString:
            MM_ARRAY_FREE(char, value->value.string.chars);
            break;
        case JSONArray:
            json_array_free(value->value.array);
            break;
        default:
            break;
    }
    MM_FREE(JSON_Value, value);
}

mm_ptr<JSON_Value> json_value_init_object(void) {
    mm_ptr<JSON_Value> new_value = MM_ALLOC(JSON_Value);
    if (!new_value) {
        return NULL;
    }
    new_value->parent = NULL;
    new_value->type = JSONObject;
    new_value->value.object = json_object_init(new_value);
    if (!new_value->value.object) {
        MM_FREE(JSON_Value, new_value);
        return NULL;
    }
    return new_value;
}

mm_ptr<JSON_Value> json_value_init_array(void) {
    mm_ptr<JSON_Value> new_value = MM_ALLOC(JSON_Value);
    if (!new_value) {
        return NULL;
    }
    new_value->parent = NULL;
    new_value->type = JSONArray;
    new_value->value.array = json_array_init(new_value);
    if (!new_value->value.array) {
        MM_FREE(JSON_Value, new_value);
        return NULL;
    }
    return new_value;
}

mm_ptr<JSON_Value> json_value_init_string(mm_array_ptr<const char> string) {
    if (string == NULL) {
        return NULL;
    }
    return json_value_init_string_with_len(string, strlen(_GETARRAYPTR(char, string)));
}

mm_ptr<JSON_Value> json_value_init_string_with_len(mm_array_ptr<const char> string, size_t length) {
    mm_array_ptr<char> copy = NULL;
    mm_ptr<JSON_Value> value = NULL;
    if (string == NULL) {
        return NULL;
    }
    if (!is_valid_utf8(_GETARRAYPTR(char, string), length)) {
        return NULL;
    }
    copy = parson_strndup(string, length);
    if (copy == NULL) {
        return NULL;
    }
    value = json_value_init_string_no_copy(copy, length);
    if (value == NULL) {
        MM_ARRAY_FREE(char, copy);
    }
    return value;
}

mm_ptr<JSON_Value> json_value_init_number(double number) {
    mm_ptr<JSON_Value> new_value = NULL;
    if (IS_NUMBER_INVALID(number)) {
        return NULL;
    }
    new_value = MM_ALLOC(JSON_Value);
    if (new_value == NULL) {
        return NULL;
    }
    new_value->parent = NULL;
    new_value->type = JSONNumber;
    new_value->value.number = number;
    return new_value;
}

mm_ptr<JSON_Value> json_value_init_boolean(int boolean) {
    mm_ptr<JSON_Value> new_value = MM_ALLOC(JSON_Value);
    if (!new_value) {
        return NULL;
    }
    new_value->parent = NULL;
    new_value->type = JSONBoolean;
    new_value->value.boolean = boolean ? 1 : 0;
    return new_value;
}

mm_ptr<JSON_Value> json_value_init_null(void) {
    mm_ptr<JSON_Value> new_value = MM_ALLOC(JSON_Value);
    if (!new_value) {
        return NULL;
    }
    new_value->parent = NULL;
    new_value->type = JSONNull;
    return new_value;
}

mm_ptr<JSON_Value> json_value_deep_copy(mm_ptr<const JSON_Value> value) {
    size_t i = 0;
    mm_ptr<JSON_Value> return_value = NULL, temp_value_copy = NULL, temp_value = NULL;
    mm_ptr<const JSON_String> temp_string = NULL;
    mm_array_ptr<const char> temp_key = NULL;
    mm_array_ptr<char> temp_string_copy = NULL;
    mm_ptr<JSON_Array> temp_array = NULL, temp_array_copy = NULL;
    mm_ptr<JSON_Object> temp_object = NULL, temp_object_copy = NULL;

    switch (json_value_get_type(value)) {
        case JSONArray:
            temp_array = json_value_get_array(value);
            return_value = json_value_init_array();
            if (return_value == NULL) {
                return NULL;
            }
            temp_array_copy = json_value_get_array(return_value);
            for (i = 0; i < json_array_get_count(temp_array); i++) {
                temp_value = json_array_get_value(temp_array, i);
                temp_value_copy = json_value_deep_copy(temp_value);
                if (temp_value_copy == NULL) {
                    json_value_free(return_value);
                    return NULL;
                }
                if (json_array_add(temp_array_copy, temp_value_copy) == JSONFailure) {
                    json_value_free(return_value);
                    json_value_free(temp_value_copy);
                    return NULL;
                }
            }
            return return_value;
        case JSONObject:
            temp_object = json_value_get_object(value);
            return_value = json_value_init_object();
            if (return_value == NULL) {
                return NULL;
            }
            temp_object_copy = json_value_get_object(return_value);
            for (i = 0; i < json_object_get_count(temp_object); i++) {
                temp_key = json_object_get_name(temp_object, i);
                temp_value = json_object_get_value(temp_object, temp_key);
                temp_value_copy = json_value_deep_copy(temp_value);
                if (temp_value_copy == NULL) {
                    json_value_free(return_value);
                    return NULL;
                }
                if (json_object_add(temp_object_copy, temp_key, temp_value_copy) == JSONFailure) {
                    json_value_free(return_value);
                    json_value_free(temp_value_copy);
                    return NULL;
                }
            }
            return return_value;
        case JSONBoolean:
            return json_value_init_boolean(json_value_get_boolean(value));
        case JSONNumber:
            return json_value_init_number(json_value_get_number(value));
        case JSONString:
            temp_string = json_value_get_string_desc(value);
            if (temp_string == NULL) {
                return NULL;
            }
            temp_string_copy = parson_strndup(temp_string->chars, temp_string->length);
            if (temp_string_copy == NULL) {
                return NULL;
            }
            return_value = json_value_init_string_no_copy(temp_string_copy, temp_string->length);
            if (return_value == NULL) {
                MM_ARRAY_FREE(char, temp_string_copy);
            }
            return return_value;
        case JSONNull:
            return json_value_init_null();
        case JSONError:
            return NULL;
        default:
            return NULL;
    }
}

size_t json_serialization_size(mm_ptr<const JSON_Value> value) {
    /* recursively allocating buffer on stack is a bad idea, so let's do it only once */
    char num_buf[NUM_BUF_SIZE];
    int res = json_serialize_to_buffer_r(value, NULL, 0, 0, num_buf);
    return res < 0 ? 0 : (size_t)(res) + 1;
}

JSON_Status json_serialize_to_buffer(mm_ptr<const JSON_Value> value,
        mm_array_ptr<char> buf, size_t buf_size_in_bytes) {
    int written = -1;
    size_t needed_size_in_bytes = json_serialization_size(value);
    if (needed_size_in_bytes == 0 || buf_size_in_bytes < needed_size_in_bytes) {
        return JSONFailure;
    }
    written = json_serialize_to_buffer_r(value, buf, 0, 0, NULL);
    if (written < 0) {
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_serialize_to_file(mm_ptr<const JSON_Value> value, const char *filename) {
    JSON_Status return_code = JSONSuccess;
    FILE *fp = NULL;
    mm_array_ptr<char> serialized_string = json_serialize_to_string(value);
    if (serialized_string == NULL) {
        return JSONFailure;
    }
    fp = fopen(filename, "w");
    if (fp == NULL) {
        mm_json_free_serialized_string(serialized_string);
        return JSONFailure;
    }
    if (fputs(_GETARRAYPTR(char, serialized_string), fp) == EOF) {
        return_code = JSONFailure;
    }
    if (fclose(fp) == EOF) {
        return_code = JSONFailure;
    }
    mm_json_free_serialized_string(serialized_string);
    return return_code;
}

mm_array_ptr<char> json_serialize_to_string(mm_ptr<const JSON_Value> value) {
    JSON_Status serialization_result = JSONFailure;
    size_t buf_size_bytes = json_serialization_size(value);
    mm_array_ptr<char> buf = NULL;
    if (buf_size_bytes == 0) {
        return NULL;
    }
    buf = MM_ARRAY_ALLOC(char, buf_size_bytes);
    if (buf == NULL) {
        return NULL;
    }
    serialization_result = json_serialize_to_buffer(value, buf, buf_size_bytes);
    if (serialization_result == JSONFailure) {
        mm_json_free_serialized_string(buf);
        return NULL;
    }
    return buf;
}

size_t json_serialization_size_pretty(mm_ptr<const JSON_Value> value) {
    /* recursively allocating buffer on stack is a bad idea, so let's do it only once */
    char num_buf[NUM_BUF_SIZE];
    int res = json_serialize_to_buffer_r(value, NULL, 0, 1, num_buf);
    return res < 0 ? 0 : (size_t)(res) + 1;
}

JSON_Status json_serialize_to_buffer_pretty(mm_ptr<const JSON_Value> value,
        mm_array_ptr<char> buf, size_t buf_size_in_bytes) {
    int written = -1;
    size_t needed_size_in_bytes = json_serialization_size_pretty(value);
    if (needed_size_in_bytes == 0 || buf_size_in_bytes < needed_size_in_bytes) {
        return JSONFailure;
    }
    written = json_serialize_to_buffer_r(value, buf, 0, 1, NULL);
    if (written < 0) {
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_serialize_to_file_pretty(mm_ptr<const JSON_Value> value, const char *filename) {
    JSON_Status return_code = JSONSuccess;
    FILE *fp = NULL;
    mm_array_ptr<char> serialized_string = json_serialize_to_string_pretty(value);
    if (serialized_string == NULL) {
        return JSONFailure;
    }
    fp = fopen(filename, "w");
    if (fp == NULL) {
        mm_json_free_serialized_string(serialized_string);
        return JSONFailure;
    }
    if (fputs(_GETARRAYPTR(char, serialized_string), fp) == EOF) {
        return_code = JSONFailure;
    }
    if (fclose(fp) == EOF) {
        return_code = JSONFailure;
    }
    mm_json_free_serialized_string(serialized_string);
    return return_code;
}

mm_array_ptr<char> json_serialize_to_string_pretty(mm_ptr<const JSON_Value> value) {
    JSON_Status serialization_result = JSONFailure;
    size_t buf_size_bytes = json_serialization_size_pretty(value);
    mm_array_ptr<char> buf = NULL;
    if (buf_size_bytes == 0) {
        return NULL;
    }
    buf = MM_ARRAY_ALLOC(char, buf_size_bytes);
    if (buf == NULL) {
        return NULL;
    }
    serialization_result = json_serialize_to_buffer_pretty(value, buf, buf_size_bytes);
    if (serialization_result == JSONFailure) {
        mm_json_free_serialized_string(buf);
        return NULL;
    }
    return buf;
}

void json_free_serialized_string(char *string) {
    parson_free(string);
}

void mm_json_free_serialized_string(mm_array_ptr<char> string) {
    MM_ARRAY_FREE(char, string);
}

JSON_Status json_array_remove(mm_ptr<JSON_Array> array, size_t ix) {
    size_t to_move_bytes = 0;
    if (array == NULL || ix >= json_array_get_count(array)) {
        return JSONFailure;
    }
    json_value_free(json_array_get_value(array, ix));
    to_move_bytes = (json_array_get_count(array) - 1 - ix) * sizeof(mm_ptr<JSON_Value>);
    memmove(_GETARRAYPTR(mm_ptr<JSON_Value>, array->items + ix),
            _GETARRAYPTR(mm_ptr<JSON_Value>, array->items + ix + 1),
            to_move_bytes);
    array->count -= 1;
    return JSONSuccess;
}

JSON_Status json_array_replace_value(mm_ptr<JSON_Array> array, size_t ix, mm_ptr<JSON_Value> value) {
    if (array == NULL || value == NULL || value->parent != NULL || ix >= json_array_get_count(array)) {
        return JSONFailure;
    }
    json_value_free(json_array_get_value(array, ix));
    value->parent = json_array_get_wrapping_value(array);
    array->items[ix] = value;
    return JSONSuccess;
}

JSON_Status json_array_replace_string(mm_ptr<JSON_Array> array, size_t i, mm_array_ptr<const char> string) {
    mm_ptr<JSON_Value> value = json_value_init_string(string);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_replace_value(array, i, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_replace_string_with_len(mm_ptr<JSON_Array> array, size_t i,
        mm_array_ptr<const char> string, size_t len) {
    mm_ptr<JSON_Value> value = json_value_init_string_with_len(string, len);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_replace_value(array, i, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_replace_number(mm_ptr<JSON_Array> array, size_t i, double number) {
    mm_ptr<JSON_Value> value = json_value_init_number(number);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_replace_value(array, i, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_replace_boolean(mm_ptr<JSON_Array> array, size_t i, int boolean) {
    mm_ptr<JSON_Value> value = json_value_init_boolean(boolean);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_replace_value(array, i, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_replace_null(mm_ptr<JSON_Array> array, size_t i) {
    mm_ptr<JSON_Value> value = json_value_init_null();
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_replace_value(array, i, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_clear(mm_ptr<JSON_Array> array) {
    size_t i = 0;
    if (array == NULL) {
        return JSONFailure;
    }
    for (i = 0; i < json_array_get_count(array); i++) {
        json_value_free(json_array_get_value(array, i));
    }
    array->count = 0;
    return JSONSuccess;
}

JSON_Status json_array_append_value(mm_ptr<JSON_Array> array, mm_ptr<JSON_Value> value) {
    if (array == NULL || value == NULL || value->parent != NULL) {
        return JSONFailure;
    }
    return json_array_add(array, value);
}

JSON_Status json_array_append_string(mm_ptr<JSON_Array> array, mm_array_ptr<const char> string) {
    mm_ptr<JSON_Value> value = json_value_init_string(string);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_append_value(array, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_append_string_with_len(mm_ptr<JSON_Array> array,
        mm_array_ptr<const char> string, size_t len) {
    mm_ptr<JSON_Value> value = json_value_init_string_with_len(string, len);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_append_value(array, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_append_number(mm_ptr<JSON_Array> array, double number) {
    mm_ptr<JSON_Value> value = json_value_init_number(number);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_append_value(array, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_append_boolean(mm_ptr<JSON_Array> array, int boolean) {
    mm_ptr<JSON_Value> value = json_value_init_boolean(boolean);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_append_value(array, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_array_append_null(mm_ptr<JSON_Array> array) {
    mm_ptr<JSON_Value> value = json_value_init_null();
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_array_append_value(array, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_set_value(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, mm_ptr<JSON_Value> value) {
    size_t i = 0;
    mm_ptr<JSON_Value> old_value = NULL;
    if (object == NULL || name == NULL || value == NULL || value->parent != NULL) {
        return JSONFailure;
    }
    old_value = json_object_get_value(object, name);
    if (old_value != NULL) { /* free and overwrite old value */
        json_value_free(old_value);
        for (i = 0; i < json_object_get_count(object); i++) {
            if (strcmp(_GETARRAYPTR(char, object->names[i]), _GETARRAYPTR(char, name)) == 0) {
                value->parent = json_object_get_wrapping_value(object);
                object->values[i] = value;
                return JSONSuccess;
            }
        }
    }
    /* add new key value pair */
    return json_object_add(object, name, value);
}

JSON_Status json_object_set_string(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
        mm_array_ptr<const char> string) {
    mm_ptr<JSON_Value> value = json_value_init_string(string);
    JSON_Status status = json_object_set_value(object, name, value);
    if (status == JSONFailure) {
        json_value_free(value);
    }
    return status;
}

JSON_Status json_object_set_string_with_len(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
        mm_array_ptr<const char> string, size_t len) {
    mm_ptr<JSON_Value> value = json_value_init_string_with_len(string, len);
    JSON_Status status = json_object_set_value(object, name, value);
    if (status == JSONFailure) {
        json_value_free(value);
    }
    return status;
}

JSON_Status json_object_set_number(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, double number) {
    mm_ptr<JSON_Value> value = json_value_init_number(number);
    JSON_Status status = json_object_set_value(object, name, value);
    if (status == JSONFailure) {
        json_value_free(value);
    }
    return status;
}

JSON_Status json_object_set_boolean(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, int boolean) {
    mm_ptr<JSON_Value> value = json_value_init_boolean(boolean);
    JSON_Status status = json_object_set_value(object, name, value);
    if (status == JSONFailure) {
        json_value_free(value);
    }
    return status;
}

JSON_Status json_object_set_null(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name) {
    mm_ptr<JSON_Value> value = json_value_init_null();
    JSON_Status status = json_object_set_value(object, name, value);
    if (status == JSONFailure) {
        json_value_free(value);
    }
    return status;
}

JSON_Status json_object_dotset_value(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, mm_ptr<JSON_Value> value) {
    const char *dot_pos_raw = NULL;
    mm_array_ptr<const char> dot_pos = NULL;
    mm_ptr<JSON_Value> temp_value = NULL, new_value = NULL;
    mm_ptr<JSON_Object> temp_object = NULL, new_object = NULL;
    JSON_Status status = JSONFailure;
    size_t name_len = 0;
    if (object == NULL || name == NULL || value == NULL) {
        return JSONFailure;
    }
    dot_pos_raw = strchr(_GETARRAYPTR(char, name), '.');
    if (dot_pos_raw == NULL) {
        return json_object_set_value(object, name, value);
    }
    name_len = dot_pos_raw - (char *)_GETARRAYPTR(char, name);
    dot_pos = name + name_len;
    temp_value = json_object_getn_value(object, name, name_len);
    if (temp_value) {
        /* Don't overwrite existing non-object (unlike json_object_set_value, but it shouldn't be changed at this point) */
        if (json_value_get_type(temp_value) != JSONObject) {
            return JSONFailure;
        }
        temp_object = json_value_get_object(temp_value);
        return json_object_dotset_value(temp_object, dot_pos + 1, value);
    }
    new_value = json_value_init_object();
    if (new_value == NULL) {
        return JSONFailure;
    }
    new_object = json_value_get_object(new_value);
    status = json_object_dotset_value(new_object, dot_pos + 1, value);
    if (status != JSONSuccess) {
        json_value_free(new_value);
        return JSONFailure;
    }
    status = json_object_addn(object, name, name_len, new_value);
    if (status != JSONSuccess) {
        json_object_dotremove_internal(new_object, dot_pos + 1, 0);
        json_value_free(new_value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_dotset_string(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, mm_array_ptr<const char> string) {
    mm_ptr<JSON_Value> value = json_value_init_string(string);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_object_dotset_value(object, name, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_dotset_string_with_len(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, mm_array_ptr<const char> string, size_t len) {
    mm_ptr<JSON_Value> value = json_value_init_string_with_len(string, len);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_object_dotset_value(object, name, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_dotset_number(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, double number) {
    mm_ptr<JSON_Value> value = json_value_init_number(number);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_object_dotset_value(object, name, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_dotset_boolean(mm_ptr<JSON_Object> object,
        mm_array_ptr<const char> name, int boolean) {
    mm_ptr<JSON_Value> value = json_value_init_boolean(boolean);
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_object_dotset_value(object, name, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_dotset_null(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name) {
    mm_ptr<JSON_Value> value = json_value_init_null();
    if (value == NULL) {
        return JSONFailure;
    }
    if (json_object_dotset_value(object, name, value) == JSONFailure) {
        json_value_free(value);
        return JSONFailure;
    }
    return JSONSuccess;
}

JSON_Status json_object_remove(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name) {
    return json_object_remove_internal(object, name, 1);
}

JSON_Status json_object_dotremove(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name) {
    return json_object_dotremove_internal(object, name, 1);
}

JSON_Status json_object_clear(mm_ptr<JSON_Object> object) {
    size_t i = 0;
    if (object == NULL) {
        return JSONFailure;
    }
    for (i = 0; i < json_object_get_count(object); i++) {
        MM_ARRAY_FREE(char, object->names[i]);
        json_value_free(object->values[i]);
    }
    object->count = 0;
    return JSONSuccess;
}

JSON_Status json_validate(mm_ptr<const JSON_Value> schema, mm_ptr<const JSON_Value> value) {
    mm_ptr<JSON_Value> temp_schema_value = NULL, temp_value = NULL;
    mm_ptr<JSON_Array> schema_array = NULL, value_array = NULL;
    mm_ptr<JSON_Object> schema_object = NULL, value_object = NULL;
    JSON_Value_Type schema_type = JSONError, value_type = JSONError;
    mm_array_ptr<const char> key = NULL;
    size_t i = 0, count = 0;
    if (schema == NULL || value == NULL) {
        return JSONFailure;
    }
    schema_type = json_value_get_type(schema);
    value_type = json_value_get_type(value);
    if (schema_type != value_type && schema_type != JSONNull) { /* null represents all values */
        return JSONFailure;
    }
    switch (schema_type) {
        case JSONArray:
            schema_array = json_value_get_array(schema);
            value_array = json_value_get_array(value);
            count = json_array_get_count(schema_array);
            if (count == 0) {
                return JSONSuccess; /* Empty array allows all types */
            }
            /* Get first value from array, rest is ignored */
            temp_schema_value = json_array_get_value(schema_array, 0);
            for (i = 0; i < json_array_get_count(value_array); i++) {
                temp_value = json_array_get_value(value_array, i);
                if (json_validate(temp_schema_value, temp_value) == JSONFailure) {
                    return JSONFailure;
                }
            }
            return JSONSuccess;
        case JSONObject:
            schema_object = json_value_get_object(schema);
            value_object = json_value_get_object(value);
            count = json_object_get_count(schema_object);
            if (count == 0) {
                return JSONSuccess; /* Empty object allows all objects */
            } else if (json_object_get_count(value_object) < count) {
                return JSONFailure; /* Tested object mustn't have less name-value pairs than schema */
            }
            for (i = 0; i < count; i++) {
                key = json_object_get_name(schema_object, i);
                temp_schema_value = json_object_get_value(schema_object, key);
                temp_value = json_object_get_value(value_object, key);
                if (temp_value == NULL) {
                    return JSONFailure;
                }
                if (json_validate(temp_schema_value, temp_value) == JSONFailure) {
                    return JSONFailure;
                }
            }
            return JSONSuccess;
        case JSONString: case JSONNumber: case JSONBoolean: case JSONNull:
            return JSONSuccess; /* equality already tested before switch */
        case JSONError: default:
            return JSONFailure;
    }
}

int json_value_equals(mm_ptr<const JSON_Value> a, mm_ptr<const JSON_Value> b) {
    mm_ptr<JSON_Object> a_object = NULL, b_object = NULL;
    mm_ptr<JSON_Array> a_array = NULL, b_array = NULL;
    mm_ptr<const JSON_String> a_string = NULL, b_string = NULL;
    mm_array_ptr<const char> key = NULL;
    size_t a_count = 0, b_count = 0, i = 0;
    JSON_Value_Type a_type, b_type;
    a_type = json_value_get_type(a);
    b_type = json_value_get_type(b);
    if (a_type != b_type) {
        return 0;
    }
    switch (a_type) {
        case JSONArray:
            a_array = json_value_get_array(a);
            b_array = json_value_get_array(b);
            a_count = json_array_get_count(a_array);
            b_count = json_array_get_count(b_array);
            if (a_count != b_count) {
                return 0;
            }
            for (i = 0; i < a_count; i++) {
                if (!json_value_equals(json_array_get_value(a_array, i),
                                       json_array_get_value(b_array, i))) {
                    return 0;
                }
            }
            return 1;
        case JSONObject:
            a_object = json_value_get_object(a);
            b_object = json_value_get_object(b);
            a_count = json_object_get_count(a_object);
            b_count = json_object_get_count(b_object);
            if (a_count != b_count) {
                return 0;
            }
            for (i = 0; i < a_count; i++) {
                key = json_object_get_name(a_object, i);
                if (!json_value_equals(json_object_get_value(a_object, key),
                                       json_object_get_value(b_object, key))) {
                    return 0;
                }
            }
            return 1;
        case JSONString:
            a_string = json_value_get_string_desc(a);
            b_string = json_value_get_string_desc(b);
            if (a_string == NULL || b_string == NULL) {
                return 0; /* shouldn't happen */
            }
            return a_string->length == b_string->length &&
                   memcmp(_GETARRAYPTR(char, a_string->chars),
                          _GETARRAYPTR(char, b_string->chars), a_string->length) == 0;
        case JSONBoolean:
            return json_value_get_boolean(a) == json_value_get_boolean(b);
        case JSONNumber:
            return fabs(json_value_get_number(a) - json_value_get_number(b)) < 0.000001; /* EPSILON */
        case JSONError:
            return 1;
        case JSONNull:
            return 1;
        default:
            return 1;
    }
}

JSON_Value_Type json_type(mm_ptr<const JSON_Value> value) {
    return json_value_get_type(value);
}

mm_ptr<JSON_Object> json_object (mm_ptr<const JSON_Value> value) {
    return json_value_get_object(value);
}

mm_ptr<JSON_Array> json_array  (mm_ptr<const JSON_Value> value) {
    return json_value_get_array(value);
}

mm_array_ptr<const char> json_string(mm_ptr<const JSON_Value> value) {
    return json_value_get_string(value);
}

size_t json_string_len(mm_ptr<const JSON_Value> value) {
    return json_value_get_string_len(value);
}

double json_number (mm_ptr<const JSON_Value> value) {
    return json_value_get_number(value);
}

int json_boolean(mm_ptr<const JSON_Value> value) {
    return json_value_get_boolean(value);
}

void json_set_allocation_functions(JSON_Malloc_Function malloc_fun, JSON_Free_Function free_fun) {
    parson_malloc = malloc_fun;
    parson_free = free_fun;
}

void json_set_escape_slashes(int escape_slashes) {
    parson_escape_slashes = escape_slashes;
}
