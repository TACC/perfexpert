# Multiple files

CC=gcc
CFLAGS=-O3 -g -fopenmp

all: test

test: omp_mm.o omp_mm_compute.o
	$(CC) $(CFLAGS) omp_mm.o omp_mm_compute.o -o test

omp_mm.o: omp_mm.c
	$(CC) -c $(CFLAGS) omp_mm.c

omp_mm_compute.o: omp_mm_compute.c
	$(CC) -c $(CFLAGS) omp_mm_compute.c

clean:
	rm -rf *o test
