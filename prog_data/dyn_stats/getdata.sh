#!/usr/bin/env bash

#
# This script gets dynamic data from a dynamic statistic file.
#
# $1 : "heap" -- size of the largest heap object
#      "total" -- total size of all shared arrays of pointers
#      others -- size of the largest shared array of pointers
#

if [[ $2 == "heap" ]]; then
    grep "heap" $1 | cut -d ':' -f2 | cut -d ' ' -f2 | sort -h | uniq
elif [[ $2 == "total" ]] ; then
    grep "Total" $1 | cut -d ':' -f2 | cut -d ' ' -f2 | sort -h | uniq
else
    grep "Largest shared" $1 | cut -d ':' -f2 | cut -d ' ' -f2 | sort -h | uniq
fi
