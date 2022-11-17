all: benchmark

benchmark: benchmark.o shamir.o
	gcc -std=c11 -g benchmark.o shamir.o -o benchmark -lgmp

benchmark.o: benchmark.c
	gcc -std=c11 -g benchmark.c -c

shamir.o: shamir.c
	gcc -std=c11 -g shamir.c -c

clean:
	rm benchmark.o shamir.o benchmark
