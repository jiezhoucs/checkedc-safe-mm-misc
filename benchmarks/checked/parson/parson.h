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

#ifndef parson_parson_h
#define parson_parson_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>   /* size_t */

#include "safe_mm_checked.h"

/* Types and enums */
typedef struct json_object_t JSON_Object;
typedef struct json_array_t  JSON_Array;
typedef struct json_value_t  JSON_Value;

enum json_value_type {
    JSONError   = -1,
    JSONNull    = 1,
    JSONString  = 2,
    JSONNumber  = 3,
    JSONObject  = 4,
    JSONArray   = 5,
    JSONBoolean = 6
};
typedef int JSON_Value_Type;

enum json_result_t {
    JSONSuccess = 0,
    JSONFailure = -1
};
typedef int JSON_Status;

typedef void * (*JSON_Malloc_Function)(size_t);
typedef void   (*JSON_Free_Function)(void *);

/* Call only once, before calling any other function from parson API. If not called, malloc and free
   from stdlib will be used for all allocations */
void json_set_allocation_functions(JSON_Malloc_Function malloc_fun, JSON_Free_Function free_fun);

/* Sets if slashes should be escaped or not when serializing JSON. By default slashes are escaped.
 This function sets a global setting and is not thread safe. */
void json_set_escape_slashes(int escape_slashes);

/* Parses first JSON value in a file, returns NULL in case of error */
mm_ptr<JSON_Value>  json_parse_file(const char *filename);

/* Parses first JSON value in a file and ignores comments (/ * * / and //),
   returns NULL in case of error */
mm_ptr<JSON_Value>  json_parse_file_with_comments(const char *filename);

/*  Parses first JSON value in a string, returns NULL in case of error */
mm_ptr<JSON_Value>  json_parse_string(mm_array_ptr<const char> string);

/*  Parses first JSON value in a string and ignores comments (/ * * / and //),
    returns NULL in case of error */
mm_ptr<JSON_Value>  json_parse_string_with_comments(mm_array_ptr<const char> string);

/* Serialization */
size_t      json_serialization_size(mm_ptr<const JSON_Value> value); /* returns 0 on fail */
JSON_Status json_serialize_to_buffer(mm_ptr<const JSON_Value> value,
    mm_array_ptr<char> buf, size_t buf_size_in_bytes);
JSON_Status json_serialize_to_file(mm_ptr<const JSON_Value> value, const char *filename);
mm_array_ptr<char> json_serialize_to_string(mm_ptr<const JSON_Value> value);

/* Pretty serialization */
size_t      json_serialization_size_pretty(mm_ptr<const JSON_Value> value); /* returns 0 on fail */
JSON_Status json_serialize_to_buffer_pretty(mm_ptr<const JSON_Value> value,
    mm_array_ptr<char> buf, size_t buf_size_in_bytes);
JSON_Status json_serialize_to_file_pretty(mm_ptr<const JSON_Value> value, const char *filename);
mm_array_ptr<char>  json_serialize_to_string_pretty(mm_ptr<const JSON_Value> value);

void        json_free_serialized_string(char *string); /* frees string from json_serialize_to_string and json_serialize_to_string_pretty */
void mm_json_free_serialized_string(mm_array_ptr<char> string);

/* Comparing */
int  json_value_equals(mm_ptr<const JSON_Value> a, mm_ptr<const JSON_Value> b);

/* Validation
   This is *NOT* JSON Schema. It validates json by checking if object have identically
   named fields with matching types.
   For example schema {"name":"", "age":0} will validate
   {"name":"Joe", "age":25} and {"name":"Joe", "age":25, "gender":"m"},
   but not {"name":"Joe"} or {"name":"Joe", "age":"Cucumber"}.
   In case of arrays, only first value in schema is checked against all values in tested array.
   Empty objects ({}) validate all objects, empty arrays ([]) validate all arrays,
   null validates values of every type.
 */
JSON_Status json_validate(mm_ptr<const JSON_Value> schema, mm_ptr<const JSON_Value> value);

/*
 * JSON Object
 */
mm_ptr<JSON_Value> json_object_get_value(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
mm_array_ptr<const char> json_object_get_string (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
size_t        json_object_get_string_len(mm_ptr<const JSON_Object> object,
    mm_array_ptr<const char> name); /* doesn't account for last null character */
mm_ptr<JSON_Object> json_object_get_object (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
JSON_Array  * json_object_get_array  (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
double        json_object_get_number (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name); /* returns 0 on fail */
int           json_object_get_boolean(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name); /* returns -1 on fail */

/* dotget functions enable addressing values with dot notation in nested objects,
 just like in structs or c++/java/c# objects (e.g. objectA.objectB.value).
 Because valid names in JSON can contain dots, some values may be inaccessible
 this way. */
mm_ptr<JSON_Value> json_object_dotget_value  (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
mm_array_ptr<const char> json_object_dotget_string(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
size_t        json_object_dotget_string_len(mm_ptr<const JSON_Object> object,
    mm_array_ptr<const char> name); /* doesn't account for last null character */
mm_ptr<JSON_Object> json_object_dotget_object (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
JSON_Array  * json_object_dotget_array  (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
double        json_object_dotget_number (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name); /* returns 0 on fail */
int           json_object_dotget_boolean(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name); /* returns -1 on fail */

/* Functions to get available names */
size_t        json_object_get_count(mm_ptr<const JSON_Object> object);
mm_array_ptr<const char> json_object_get_name(mm_ptr<const JSON_Object> object, size_t index);
mm_ptr<JSON_Value> json_object_get_value_at(mm_ptr<const JSON_Object> object, size_t index);
mm_ptr<JSON_Value> json_object_get_wrapping_value(mm_ptr<const JSON_Object> object);

/* Functions to check if object has a value with a specific name. Returned value is 1 if object has
 * a value and 0 if it doesn't. dothas functions behave exactly like dotget functions. */
int json_object_has_value        (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
int json_object_has_value_of_type(mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name, JSON_Value_Type type);

int json_object_dothas_value     (mm_ptr<const JSON_Object> object, mm_array_ptr<const char> name);
int json_object_dothas_value_of_type(mm_ptr<const JSON_Object> object,
    mm_array_ptr<const char> name, JSON_Value_Type type);

/* Creates new name-value pair or frees and replaces old value with a new one.
 * json_object_set_value does not copy passed value so it shouldn't be freed afterwards. */
JSON_Status json_object_set_value(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, mm_ptr<JSON_Value> value);
JSON_Status json_object_set_string(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
    mm_array_ptr<const char> string);
JSON_Status json_object_set_string_with_len(mm_ptr<JSON_Object> bject, mm_array_ptr<const char> name,
    mm_array_ptr<const char> string, size_t len);  /* length shouldn't include last null character */
JSON_Status json_object_set_number(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, double number);
JSON_Status json_object_set_boolean(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, int boolean);
JSON_Status json_object_set_null(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name);

/* Works like dotget functions, but creates whole hierarchy if necessary.
 * json_object_dotset_value does not copy passed value so it shouldn't be freed afterwards. */
JSON_Status json_object_dotset_value(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, mm_ptr<JSON_Value> value);
JSON_Status json_object_dotset_string(mm_ptr<JSON_Object> object,
    mm_array_ptr<const char> name, mm_array_ptr<const char> string);
JSON_Status json_object_dotset_string_with_len(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name,
    mm_array_ptr<const char> string, size_t len); /* length shouldn't include last null character */
JSON_Status json_object_dotset_number(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, double number);
JSON_Status json_object_dotset_boolean(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name, int boolean);
JSON_Status json_object_dotset_null(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name);

/* Frees and removes name-value pair */
JSON_Status json_object_remove(mm_ptr<JSON_Object> object, mm_array_ptr<const char> name);

/* Works like dotget function, but removes name-value pair only on exact match. */
JSON_Status json_object_dotremove(mm_ptr<JSON_Object> object, mm_array_ptr<const char> key);

/* Removes all name-value pairs in object */
JSON_Status json_object_clear(mm_ptr<JSON_Object> object);

/*
 *JSON Array
 */
mm_ptr<JSON_Value> json_array_get_value  (const JSON_Array *array, size_t index);
mm_array_ptr<const char> json_array_get_string (const JSON_Array *array, size_t index);
size_t        json_array_get_string_len(const JSON_Array *array, size_t index); /* doesn't account for last null character */
mm_ptr<JSON_Object> json_array_get_object (const JSON_Array *array, size_t index);
JSON_Array  * json_array_get_array  (const JSON_Array *array, size_t index);
double        json_array_get_number (const JSON_Array *array, size_t index); /* returns 0 on fail */
int           json_array_get_boolean(const JSON_Array *array, size_t index); /* returns -1 on fail */
size_t        json_array_get_count  (const JSON_Array *array);
mm_ptr<JSON_Value> json_array_get_wrapping_value(const JSON_Array *array);

/* Frees and removes value at given index, does nothing and returns JSONFailure if index doesn't exist.
 * Order of values in array may change during execution.  */
JSON_Status json_array_remove(JSON_Array *array, size_t i);

/* Frees and removes from array value at given index and replaces it with given one.
 * Does nothing and returns JSONFailure if index doesn't exist.
 * json_array_replace_value does not copy passed value so it shouldn't be freed afterwards. */
JSON_Status json_array_replace_value(JSON_Array *array, size_t i, mm_ptr<JSON_Value> value);
JSON_Status json_array_replace_string(JSON_Array *array, size_t i, mm_array_ptr<const char> string);
JSON_Status json_array_replace_string_with_len(JSON_Array *array, size_t i,
    mm_array_ptr<const char> string, size_t len); /* length shouldn't include last null character */
JSON_Status json_array_replace_number(JSON_Array *array, size_t i, double number);
JSON_Status json_array_replace_boolean(JSON_Array *array, size_t i, int boolean);
JSON_Status json_array_replace_null(JSON_Array *array, size_t i);

/* Frees and removes all values from array */
JSON_Status json_array_clear(JSON_Array *array);

/* Appends new value at the end of array.
 * json_array_append_value does not copy passed value so it shouldn't be freed afterwards. */
JSON_Status json_array_append_value(JSON_Array *array, mm_ptr<JSON_Value> value);
JSON_Status json_array_append_string(JSON_Array *array, mm_array_ptr<const char> string);
JSON_Status json_array_append_string_with_len(JSON_Array *array,
    mm_array_ptr<const char> string, size_t len); /* length shouldn't include last null character */
JSON_Status json_array_append_number(JSON_Array *array, double number);
JSON_Status json_array_append_boolean(JSON_Array *array, int boolean);
JSON_Status json_array_append_null(JSON_Array *array);

/*
 *JSON Value
 */
mm_ptr<JSON_Value> json_value_init_object (void);
mm_ptr<JSON_Value> json_value_init_array  (void);
mm_ptr<JSON_Value> json_value_init_string (mm_array_ptr<const char> string); /* copies passed string */
mm_ptr<JSON_Value> json_value_init_string_with_len(mm_array_ptr<const char> string,
    size_t length); /* copies passed string, length shouldn't include last null character */
mm_ptr<JSON_Value> json_value_init_number (double number);
mm_ptr<JSON_Value> json_value_init_boolean(int boolean);
mm_ptr<JSON_Value> json_value_init_null   (void);
mm_ptr<JSON_Value> json_value_deep_copy   (mm_ptr<const JSON_Value> value);
void         json_value_free        (mm_ptr<JSON_Value> value);

JSON_Value_Type json_value_get_type   (mm_ptr<const JSON_Value> value);
mm_ptr<JSON_Object>   json_value_get_object (mm_ptr<const JSON_Value> value);
JSON_Array  *   json_value_get_array  (mm_ptr<const JSON_Value> value);
mm_array_ptr<const char> json_value_get_string (mm_ptr<const JSON_Value> value);
size_t          json_value_get_string_len(mm_ptr<const JSON_Value> value); /* doesn't account for last null character */
double          json_value_get_number (mm_ptr<const JSON_Value> value);
int             json_value_get_boolean(mm_ptr<const JSON_Value> value);
mm_ptr<JSON_Value> json_value_get_parent (mm_ptr<const JSON_Value> value);

/* Same as above, but shorter */
JSON_Value_Type json_type   (mm_ptr<const JSON_Value> value);
mm_ptr<JSON_Object>  json_object (mm_ptr<const JSON_Value> value);
JSON_Array  *   json_array  (mm_ptr<const JSON_Value> value);
mm_array_ptr<const char> json_string (mm_ptr<const JSON_Value> value);
size_t          json_string_len(mm_ptr<const JSON_Value> value); /* doesn't account for last null character */
double          json_number (mm_ptr<const JSON_Value> value);
int             json_boolean(mm_ptr<const JSON_Value> value);

#ifdef __cplusplus
}
#endif

#endif
