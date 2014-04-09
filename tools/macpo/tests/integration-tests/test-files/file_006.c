#if 0
[macpo-integration-test]:init:0:1:
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:1
[macpo-integration-test]:overlap_check:42:3:?:?:?:?:?:?
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:1
[macpo-integration-test]:overlap_check:42:3:?:?:?:?:?:?
[macpo-integration-test]:stride_check:41:1
[macpo-integration-test]:stride_check:41:1
[macpo-integration-test]:stride_check:41:0
[macpo-integration-test]:overlap_check:41:3:?:?:?:?:?:?
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:1
[macpo-integration-test]:overlap_check:42:3:?:?:?:?:?:?
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:0
[macpo-integration-test]:stride_check:42:1
[macpo-integration-test]:overlap_check:42:3:?:?:?:?:?:?
[macpo-integration-test]:stride_check:41:1
[macpo-integration-test]:stride_check:41:1
[macpo-integration-test]:stride_check:41:0
[macpo-integration-test]:overlap_check:41:3:?:?:?:?:?:?
[macpo-integration-test]:stride_check:40:0
[macpo-integration-test]:stride_check:40:0
[macpo-integration-test]:stride_check:40:0
[macpo-integration-test]:overlap_check:40:3:?:?:?:?:?:?
#endif

#include <stdio.h>

#define n 2
double a[n][n], b[n][n], c[n][n];

void compute() {
  register int i, j, k;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      for (k = 0; k < n; k++) {
        c[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

int main(int argc, char *argv[]) {
  register int i, j;

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      a[i][j] = i+j;
      b[i][j] = i-j;
      c[i][j] = 0;
    }
  }

  compute();
  printf("%.1lf\n", c[1][1]);

  return 0;
}
