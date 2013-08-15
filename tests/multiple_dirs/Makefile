# Multiple files

CC=gcc
CFLAGS=-O3 -g -fopenmp

all: test

test: omp_mm.o compute.o
	$(CC) $(CFLAGS) omp_mm.o compute.o -o test

omp_mm.o: omp_mm.c
	$(CC) -c $(CFLAGS) omp_mm.c

compute.o: compute/compute.c
	$(CC) -c $(CFLAGS) compute/compute.c

clean:
	rm -rf compute/*.o *o test
