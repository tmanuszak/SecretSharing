all: benchmark

benchmark: benchmark.o blakely.o
	gcc -std=c11 -g benchmark.o blakely.o -o benchmark -lgmp

benchmark.o: benchmark.c
	gcc -std=c11 -g benchmark.c -c

blakely.o: blakely.c
	gcc -std=c11 -g blakely.c -c

clean:
	rm benchmark.o blakely.o benchmark
