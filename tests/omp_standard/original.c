#include <omp.h>

int i, j, k;

double a[2000UL][2000UL];
double b[2000UL][2000UL];
double c[2000UL][2000UL];

void compute() {
    #pragma omp parallel for shared( a, b, c) private(i, j, k)
    for (i = 0; i < 2000; i++) {
        for (j = 0; j < 2000; j++) {
            for (k = 0; k < 2000; k++) {
                c[i][j] += (a[i][k] * b[k][j]);
            }
        }
    }
}

int main(int argc,char *argv[]) {
    #pragma omp parallel for shared(a) private(i, j)
    for (i = 0; i < 2000; i++) {
        for (j = 0; j < 2000; j++) { 
            a[i][j] = (i + j);
        }
    }
    
    #pragma omp parallel for shared(b) private(i, j)
    for (i = 0; i < 2000; i++) {
        for (j = 0; j < 2000; j++) {
            b[i][j] = (i * j);
        }
    }
    
    #pragma omp parallel for shared(c) private(i, j)
    for (i = 0; i < 2000; i++) {
        for (j = 0; j < 2000; j++) {
            c[i][j] = 0;
        }
    }

    compute();
    return 0;
}
