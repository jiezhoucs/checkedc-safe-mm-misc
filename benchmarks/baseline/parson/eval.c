/**
 * This file takes as inputs JSON data files and uses parson to parse them
 * for evaluation.
 * */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "parson.h"

#define TEST(A) printf("%d %-72s-", __LINE__, #A);\
                if(A){puts(" OK");tests_passed++;}\
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define MM_STREQ(A, B) ((A) && (B) ? strcmp((_GETARRAYPTR(char, A)), \
      (_GETARRAYPTR(char, B))) == 0 : 0)
#define EPSILON 0.000001

#define JSON_FILE_NUM 10

/* JSON data files */
static char *twitter_json = "twitter";
static char *canada_json = "canada";
static char *sf_json = "citylots";
const char *data_files[JSON_FILE_NUM];

static char * read_file(const char * filename);
const char* get_file_path(const char *filename);

static int tests_passed;
static int tests_failed;

/*
 * Put all data file names in an array.
 * */
void prepare_file_paths() {
    for (unsigned i = 0; i < JSON_FILE_NUM; i++) data_files[i] = NULL;

    data_files[0] = twitter_json;
    data_files[1] = canada_json;
#if 1
    data_files[2] = sf_json;   /* A large file */
#endif
}

const char* get_file_path(const char *filename) {
    const char *tests_path = "../../../eval/json_dataset";
    static char path_buf[2048] = { 0 };
    memset(path_buf, 0, sizeof(path_buf));
    sprintf(path_buf, "%s/%s%s", tests_path, filename, ".json");
    return path_buf;
}

static char * read_file(const char * file_path) {
    FILE *fp = NULL;
    size_t size_to_read = 0;
    size_t size_read = 0;
    long pos;
    char *file_contents;
    fp = fopen(file_path, "r");
    if (!fp) {
        assert(0);
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        assert(0);
        return NULL;
    }
    size_to_read = pos;
    rewind(fp);
    file_contents = (char*)malloc(sizeof(char) * (size_to_read + 1));
    if (!file_contents) {
        fclose(fp);
        assert(0);
        return NULL;
    }
    size_read = fread(file_contents, 1, size_to_read, fp);
    if (size_read == 0 || ferror(fp)) {
        fclose(fp);
        free(file_contents);
        assert(0);
        return NULL;
    }
    fclose(fp);
    file_contents[size_read] = '\0';
    return file_contents;
}

/*
 * The main body of the evaluation.
 * */
void eval() {
    for (unsigned i = 0; i < JSON_FILE_NUM; i++) {
        if (data_files[i] == NULL) break;
        const char *file_path = get_file_path(data_files[i]);
        if (!file_path) {
            printf("Cannot find data file %s.\n", data_files[i]);
            exit(1);
        }

        /* Basic parsing */
        JSON_Value *val = json_parse_file(file_path);
        TEST(val != NULL);
        TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
        TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
        if (val) { json_value_free(val); }

        /* Parsing file with comments */
        TEST((val = json_parse_file_with_comments(file_path)) != NULL);
        TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
        TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
        if (val) { json_value_free(val); }
    }
}

int main(int argc, char *argv[]) {
    prepare_file_paths();

    eval();

    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}
