#! /usr/bin/env bash

diff -d TvZ_0.4.txt ../results/TvZ_0.4.txt | tail -r | tail -n +7 | tail -r
