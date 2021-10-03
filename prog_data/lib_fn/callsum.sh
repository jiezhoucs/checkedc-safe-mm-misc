#!/usr/bin/env bash

#
# This script calculates the total number of callsites that call a
# library function with double-pointer argument(s).
#
# $1 - name of data file
#

grep "Total " $1 | cut -d: -f2 | cut -d ' ' -f2 | paste -sd+ - | bc
