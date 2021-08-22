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

#include "safe_mm_checked.h"
#include "parson.h"

#define LARGE_FILE

#define TEST(A) printf("%d %-72s-", __LINE__, #A);\
                if(A){puts(" OK");tests_passed++;}\
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define MM_STREQ(A, B) ((A) && (B) ? strcmp((_GETARRAYPTR(char, A)), \
      (_GETARRAYPTR(char, B))) == 0 : 0)
#define EPSILON 0.000001

#define JSON_FILE_NUM 16
#define BILLION 1000000000

/* JSON data files */
static char *products_json         = "products";           /* 2.8K */
static char *students_json         = "students";           /* 35K  */
static char *grades_json           = "grades";             /* 92K  */
static char *countries_small_json  = "countries-small";    /* 329K */
static char *profiles_json         = "profiles";           /* 454K */
static char *covers_json           = "covers";             /* 470K */
static char *books_json            = "books";              /* 525K */
static char *albums_json           = "albums";             /* 634K */
static char *restaurant_json       = "restaurant";         /* 666K */
static char *countries_big_json    = "countries-big";      /* 2.3M */
static char *zips_json             = "zips";               /* 3.1M */
static char *images_json           = "images";             /* 9.2M */
static char *city_inspections_json = "city_inspections";   /* 24M  */
static char *companies_json        = "companies";          /* 75M  */
static char *citylots_json         = "citylots";           /* 181M */
static char *trades_json           = "trades";             /* 232M */
const char *data_files[JSON_FILE_NUM];
static uint64_t exe_times[JSON_FILE_NUM];

const char* get_file_path(const char *filename);

static int tests_passed;
static int tests_failed;

/* Measure the execution time of parson library functions */
static struct timespec timer_start;
static struct timespec timer_end;

/*
 * 1. Put all data file names in an array.
 * 2. Initialize the array of execution time for each file.
 * */
void prepare_files() {
    for (unsigned i = 0; i < JSON_FILE_NUM; i++) {
        data_files[i] = NULL;
        exe_times[i] = 0;
    }

    data_files[0] = products_json;
    data_files[1] = students_json;
    data_files[2] = grades_json;
    data_files[3] = countries_small_json;
    data_files[4] = profiles_json;
    data_files[5] = covers_json;
    data_files[6] = books_json;
    data_files[7] = albums_json;
    data_files[8] = restaurant_json;
    data_files[9] = countries_big_json;
    data_files[10] = zips_json;
    data_files[11] = images_json;
    data_files[12] = city_inspections_json;
    data_files[13] = companies_json;
#ifdef LARGE_FILE
    data_files[14] = citylots_json;   /* A large file */
    data_files[15] = trades_json;     /* A large file */
#endif
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

/*
 * Write results to a file.
 * */
void write_results() {
    const char *result_file_path = "../../../eval/perf_data/parson/checked/result.csv";
    FILE *perf_file = fopen(result_file_path,  "w+");
    if (perf_file == NULL) {
        perror("failed to open result file");
        exit(errno);
    }

    fprintf(perf_file, "file,exe_time\n");
    for (unsigned i = 0; i < JSON_FILE_NUM; i++) {
        const char *file_name = data_files[i];
        if (!file_name) continue;
        fprintf(perf_file, "%s,%lu\n", file_name, exe_times[i]);
    }
}

/*
 * The main body of the evaluation.
 * */
void eval() {
    for (unsigned i = 0; i < JSON_FILE_NUM; i++) {
        if (data_files[i] == NULL) continue;

        const char *file_path = get_file_path(data_files[i]);
        if (!file_path) {
            printf("Cannot find data file %s.\n", data_files[i]);
            exit(1);
        }
        printf("JSONing %s...\n", data_files[i]);

        if (clock_gettime(CLOCK_MONOTONIC, &timer_start) == -1) {
            perror("clock_gettime() failed");
            exit(errno);
        }

        /* Basic parsing and serialization */
        mm_ptr<JSON_Value> val = json_parse_file(file_path);
        mm_array_ptr<char> serialized = NULL, serialized_pretty = NULL;
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

        /* Finished parsing; record time. */
        if (clock_gettime(CLOCK_MONOTONIC, &timer_end) == -1) {
            perror("clock_gettime() failed");
            exit(errno);
        }
        uint64_t elapsed = BILLION * (timer_end.tv_sec - timer_start.tv_sec) +
            (timer_end.tv_nsec - timer_start.tv_nsec);
        exe_times[i] = elapsed;
        printf("elapsed time = %ld ns\n", elapsed);
    }
}

int main(int argc, char *argv[]) {
    prepare_files();

    eval();

    write_results();

    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}
