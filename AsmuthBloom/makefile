all: benchmark

benchmark: benchmark.o asmuthbloom.o
	gcc -std=c11 -g benchmark.o asmuthbloom.o -o benchmark -lgmp

benchmark.o: benchmark.c
	gcc -std=c11 -g benchmark.c -c

asmuthbloom.o: asmuthbloom.c
	gcc -std=c11 -g asmuthbloom.c -c

clean:
	rm benchmark.o asmuthbloom.o benchmark
