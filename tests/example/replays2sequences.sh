#!/bin/bash
############ #

# Generates data
python sc2replay_converter.py ../../replays/ 600 30 > data.csv

#Selects only PvT matchup
cat data.csv | grep PT | cut -f1 > PT.log

#prepare the binarizator and bin your data
g++ -m32 -O3 -o binit *.cpp
./binit PT.log

#run the algo
../../algo/BalanceSpan PT.log.bin 0.5 62 dico.txt

#see the result
sort -t$'\t' -k 2,2 -r -s result.txt

