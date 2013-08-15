#include <omp.h>
#include "../omp_mm.h"

void compute() {
    #pragma omp parallel for shared( a, b, c) private(i, j, k)
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            for (k = 0; k < SIZE; k++) {
                c[i][j] += (a[i][k] * b[k][j]);
            }
        }
    }
}
