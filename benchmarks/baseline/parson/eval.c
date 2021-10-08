/**
 * This file takes as inputs JSON data files and uses parson to parse them
 * for evaluation.
 * */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#include "parson.h"

#define LARGE_FILE
#define EVAL_ALL
#undef EVAL_ALL

#define TEST(A) printf("%d %-72s-", __LINE__, #A);\
                if(A){puts(" OK");tests_passed++;}\
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define MM_STREQ(A, B) ((A) && (B) ? strcmp((_GETARRAYPTR(char, A)), \
      (_GETARRAYPTR(char, B))) == 0 : 0)
#define EPSILON 0.000001

#define JSON_FILE_NUM 16
#define BILLION 1000000000

const char* get_file_path(const char *filename);

static int tests_passed;
static int tests_failed;

/* Measure the execution time of parson library functions */
static struct timespec timer_start;
static struct timespec timer_end;
static uint64_t exe_time;

/* Data files that only take very short time to be processed. */
const char *short_data[] = {
    "albums", "books", "covers", "countries-small", "profiles", "restaurant",
    "countries-big", "zips",
};
const int SHORT_DATA_NUM = 8;

/* Some data files only takes very short time to be processed. We need let
 * the program sleep for a while so that wss.pl would have the time to
 * measure its RSS before it exits. Currently the checking interval for wss
 * is 0.05 second. */
void sleep_for_mem_measure(const char *data_file) {
    for (int i = 0 ; i < SHORT_DATA_NUM; i++) {
        if (!strcmp(short_data[i], data_file)) {
            sleep(1);
            return;
        }
    }
}

/*
 * Construct the path of each data file.
 * */
const char* get_file_path(const char *filename) {
    const char *tests_path = "../../../eval/json_dataset";
    static char path_buf[2048] = { 0 };
    memset(path_buf, 0, sizeof(path_buf));
    sprintf(path_buf, "%s/%s%s", tests_path, filename, ".json");
    return path_buf;
}

void file_open_status(FILE *file) {
    if (file == NULL) {
        perror("failed to open result file");
        exit(errno);
    }
}

/*
 * Write results to a file.
 * */
void write_result(const char *file_name) {
    const char *file_path = "../../../eval/perf_data/parson/baseline/result.csv";
    FILE *perf_file = fopen(file_path, "r");
    if (perf_file == NULL) {
        perf_file = fopen(file_path, "w+");
        file_open_status(perf_file);
        fprintf(perf_file, "file_name,exe_time\n");
    }  else {
        file_open_status(perf_file = fopen(file_path, "a"));
    }
    fprintf(perf_file, "%s,%lu\n", file_name, exe_time);
}

/*
 * The main body of the evaluation.
 * */
void eval(const char *file_name) {
    const char *file_path = get_file_path(file_name);
    if (!file_path) {
        printf("Cannot find data file %s.\n", file_name);
        exit(1);
    }
    printf("Processing %s...\n", file_name);

    if (clock_gettime(CLOCK_MONOTONIC, &timer_start) == -1) {
        perror("clock_gettime() failed");
        exit(errno);
    }

    /* Basic parsing and serialization */
    JSON_Value *val = json_parse_file(file_path);
    char *serialized = NULL, *serialized_pretty = NULL;
    TEST(val != NULL);
    /* Test json_value_get_type() */
    val = json_parse_file(file_path);
    TEST(json_value_get_type(val) == JSONObject);
    serialized = json_serialize_to_string(val);
    TEST(json_value_equals(json_parse_string(serialized), val));
    serialized_pretty = json_serialize_to_string_pretty(val);
    TEST(json_value_equals(json_parse_string(serialized_pretty), val));
    json_free_serialized_string(serialized);
    json_free_serialized_string(serialized_pretty);
    if (val) { json_value_free(val); }

    /* Parsing files with comments */
    TEST((val = json_parse_file_with_comments(file_path)) != NULL);
    /* Test json_value_get_type() */
    val = json_parse_file(file_path);
    TEST(json_value_get_type(val) == JSONObject);
    TEST(json_value_equals(json_parse_string(
                    serialized = json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(
                    serialized_pretty = json_serialize_to_string_pretty(val)), val));
    json_free_serialized_string(serialized);
    json_free_serialized_string(serialized_pretty);
    if (val) { json_value_free(val); }

#ifdef MEM_OVERHEAD
    /* For memory consumption measurement. */
    sleep(1);
#endif

    /* Finished parsing; record time. */
    if (clock_gettime(CLOCK_MONOTONIC, &timer_end) == -1) {
        perror("clock_gettime() failed");
        exit(errno);
    }
    exe_time = BILLION * (timer_end.tv_sec - timer_start.tv_sec) +
        (timer_end.tv_nsec - timer_start.tv_nsec);
    printf("elapsed time = %ld ns\n", exe_time);
}

int main(int argc, char *argv[]) {
    eval(argv[1]);

    write_result(argv[1]);

    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}
