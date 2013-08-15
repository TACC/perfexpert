#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define SIZE 2000
#define NRA SIZE /* number of rows in matrix A */
#define NCA SIZE /* number of columns in matrix A */
#define NCB SIZE /* number of columns in matrix B */
int i;
int j;
int k;
/* matrix A to be multiplied */
double a[2000UL][2000UL];
double b[2000UL][2000UL]
/* matrix B to be multiplied */
;
double c[2000UL][2000UL]
/* result matrix C */
;

void compute()
{
  
#pragma omp for
  for (i = 0; i < 2000; i++) 
    for (k = 0; k < 2000; k++) 
      for (j = 0; j < 2000; j++) 
        c[i][j] += (a[i][k] * b[k][j]);
}

int main(int argc,char *argv[])
{
/*** Spawn a parallel region explicitly scoping all variables ***/
  
#pragma omp parallel shared ( a, b, c ) private ( i, j, k )
{
/*** Initialize matrices ***/
    
#pragma omp for
    for (i = 0; i < 2000; i++) 
      for (j = 0; j < 2000; j++) 
        a[i][j] = (i + j);
    
#pragma omp for
    for (i = 0; i < 2000; i++) 
      for (j = 0; j < 2000; j++) 
        b[i][j] = (i * j);
    
#pragma omp for
    for (i = 0; i < 2000; i++) 
      for (j = 0; j < 2000; j++) 
        c[i][j] = 0;
/*** Do matrix multiply sharing iterations on outer loop ***/
    compute();
/*** End of parallel region ***/
  }
  printf("%6.2f\n",c[1][1]);
  return 0;
}
