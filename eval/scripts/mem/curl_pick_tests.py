#!/usr/bin/env python3

'''
This script reads the output of running baseline curl tests and pick the
longest run tests.
'''

import os

CURL_PERF_DATA = os.path.abspath(os.getcwd() + "/../../perf_data/curl/baseline/result.1")

def run():
    test_time = dict()
    lines = open(CURL_PERF_DATA).readlines()
    for i in range(0, len(lines)):
        line = lines[i]
        index = line.find("took")
        if  index > 0:
            test_num = lines[i - 1][5:9]
            if "Warning" in lines[i - 1]:
                # Skip the Warning line
                test_num = lines[i - 2][5:9]
            test_time[test_num] = float(line[index:].split()[1][:-2])

    for i in sorted(test_time.items(), key=lambda item: item[1]):
        print(i)

#
# Entrance
#
if __name__ == "__main__":
    run()
