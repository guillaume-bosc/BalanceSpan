#!/bin/bash
############ #

# Generates data
python sc2replay_converter.py ../../replays/ 600 30 > data.csv

#Selects only PvT matchup for example
cat data.csv | grep PT | cut -f1 > PT.log

#prepare the binarizator and bin your data
g++ -m32 -O3 -o binit *.cpp
./binit PT.log

#compile the algo
cd  ../../algo/BalanceSpan_Mac/
make
cd ../../tests/example

nbItemsMax=`cat dico.txt | wc -l`
#run the algo
../../algo/BalanceSpan PT.log.bin 0.1 $nbItemsMax dico.txt

#see the result
sort -t$'\t' -k 2,2 -r -s result.txt

