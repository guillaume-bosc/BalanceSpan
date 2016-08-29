#! /usr/bin/env bash

../../algo/BalanceSpan ../../data/sequences-TZ-30.bin 0.4 62 ../../data/dico-TZ-30.txt
sort -t$'\t' -k 2,2 -r -s result.txt > TvZ_0.4.txt
rm result.txt
mv error.tmp TvZ_0.4_Error.tmp
