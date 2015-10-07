../algo/BalanceSpan ../data/sequences-TZ-30.bin 0.4 62 ../data/dico-TZ-30.txt
sort -t$'\t' -k 2,2 -r -s result.txt > ../results/TvZ_0.4.txt
rm result.txt
mv error.tmp ../results/TvZ_0.4_Error.tmp
