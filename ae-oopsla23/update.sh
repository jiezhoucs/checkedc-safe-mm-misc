#!/usr/bin/env bash

AE_SCRIPTS="misc/ae-oopsla23"

#
# Pull the latest changes in misc.
#
cd misc
git pull

#
# Update soft links
cd ..
rm -f setup.sh eval.sh eval.sh print_results.sh dryrun_mini.sh update.sh
ln -s "$AE_SCRIPTS/setup.sh" ./
ln -s "$AE_SCRIPTS/eval.sh" ./
ln -s "$AE_SCRIPTS/print_results.sh" ./
ln -s "$AE_SCRIPTS/dryrun_mini.sh" ./
ln -s "$AE_SCRIPTS/update.sh" ./
