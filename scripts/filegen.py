#!/usr/bin/env python3

'''
This script generates files that contain ramdom contents.
File sizes range from 16 KB to 128 MB.
'''

import random
import time
import sys
import os

file_dir = os.path.abspath("../../benchmark-build/thttpd")

#
# Generate files of different sizes
#
def gen_file(filename, size):
    random.seed(time.time())

    f = open(file_dir + filename, "w")
    for i in range(size):
        c = chr(int(random.random() * 127))
        f.write(c)
    f.close()

#
# Entrance of this script
#
def main():
    global file_dir
    if len(sys.argv) == 2 and sys.argv[1] == "baseline":
        file_dir += "/baseline/files/"
    else:
        file_dir += "/checked/files/"

    for i in range(14, 28):
        size = 2 ** i
        filename = "file-{}".format(size)
        print("Generating {}".format(filename))
        gen_file(filename, size)

if __name__ == "__main__":
    main()
