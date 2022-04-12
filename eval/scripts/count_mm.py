#!/usr/bin/env python3

'''
This script counts the number of mmsafe pointers, calls to mmsafe heap allocators,
and frees of a ported program.
'''

import os
import sys
import subprocess as sp

#
# Paths
#
Root_dir = os.path.abspath(os.getcwd() + "/../../..")
Misc_dir = Root_dir + "/misc"
Benchmark_dir = Misc_dir + "/benchmarks/checked"
Olden_dir = Root_dir + "/tests/test-suite/MultiSource/Benchmarks/Olden"
thttpd_dir = Benchmark_dir + "/thttpd-2.29"
parson_dir = Benchmark_dir + "/parson"
lzfse_dir = Benchmark_dir + "/lzfse-1.0/src"
curl_dir = Benchmark_dir + "/curl"
mcf_dir = Root_dir + "/../common/spec-cpu2006/benchspec/CPU2006/429.mcf"

mmsafe_ptr = ["mm_ptr", "mm_array_ptr"]
mm_alloc = ["mm_alloc", "mm_array_alloc", "mm_array_realloc", "mm_strdup",\
        "mm_calloc", "mm_single_calloc", "mmize_str", "MM_ALLOC",
        "MM_ARRAY_ALLOC", "MM_CALLOC", "MM_SINGLE_CALLOC", "MM_REALLOC",\
        "MM_NEW", "MM_ARRAY_NEW", "MM_ARRAY_RENEW"]
mm_free = ["mm_free", "mm_array_free", "MM_FREE", "MM_ARRAY_FREE",\
        "mm_Curl_safefree", ]


#
# Function: count()
# The main body of this script.
#
def count(benchmark, target):
    grep = ["grep", "-r", "--include=*.c", "--include=*.h"]
    cwd = ""
    # Configure the command and working directory based on the target program.
    if benchmark == "olden":
        cwd = Olden_dir
    elif benchmark == "thttpd":
        grep += ["--exclude=safe_mm_checked.h", "--exclude=stdchecked.h"]
        cwd = thttpd_dir
    elif benchmark == "parson":
        grep += ["--exclude=tests.c", "--exclude=eval.c"]
        cwd = parson_dir
    elif benchmark == "lzfse":
        cwd = lzfse_dir
    elif benchmark == "curl":
        cwd = curl_dir
    elif benchmark == "mcf":
        cwd = mcf_dir
    else:
        print("Unknown benchmark name!")
        sys.exit()

    # Configure the target mm_ keyword.
    if target == "ptr":
        patterns = mmsafe_ptr
    elif target == "alloc":
        patterns = mm_alloc
    elif target == "free":
        patterns = mm_free
    else:
        print("Unknown pattern!")
        sys.exit()

    # cd to benchmark directory and grep.
    os.chdir(cwd)
    target_lines = []
    for pattern in patterns:
        target_lines += sp.run(grep + [pattern], stdout=sp.PIPE).\
                stdout.decode("utf-8").split('\n')[:-1]
    result = 0
    comment = 0
    for line in target_lines:
        line = line[line.find(':') + 1:].strip()
        # Skip comment.
        if line.startswith(("/*", "//", "* ", "__attribute")):
            comment += 1
            continue
        num = 0
        # print(line)
        for pattern in patterns:
            num += line.count(pattern)
        # if num > 1:
        #     print(line)
        result += num

    print("len[result] = " + str(len(target_lines)))

    if target == "ptr":
        print("# of mmsafe pointers: " + str(result))
    elif target == "alloc":
        print("# of safe allocator calls: " + str(result))
    else:
        print("# of safe free calls: " + str(result))



#
# Entrance of the script.
#
if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("Specify a benchmark")
        sys.exit()
    benchmark = sys.argv[1]
    count(benchmark, "ptr")
    count(benchmark, "alloc")
    count(benchmark, "free")
