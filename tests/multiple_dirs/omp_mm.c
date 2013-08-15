#include <omp.h>
#include "omp_mm.h"

int main(int argc,char *argv[]) {
    #pragma omp parallel for shared(a) private(i, j)
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) { 
            a[i][j] = (i + j);
        }
    }
    
    #pragma omp parallel for shared(b) private(i, j)
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            b[i][j] = (i * j);
        }
    }
    
    #pragma omp parallel for shared(c) private(i, j)
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            c[i][j] = 0;
        }
    }

    compute();
    return 0;
}
