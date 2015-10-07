all   : bin/BalanceSpan

bin/BalanceSpan : *.cpp SeqTree/*.cpp *.h *.h
	g++ -Wno-write-strings -Wno-deprecated -m32 -O3 -o ../BalanceSpan *.cpp SeqTree/*.cpp

clean :
	rm -Rf bin/* 

