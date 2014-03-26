#if 0
[macpo-integration-test]:init:1:1:
[macpo-integration-test]:write_idx:c:1:
[macpo-integration-test]:write_idx:a:1:
[macpo-integration-test]:write_idx:b:1:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
[macpo-integration-test]:record:3:42:?:0:8:
[macpo-integration-test]:record:1:42:?:1:8:
[macpo-integration-test]:record:1:42:?:2:8:
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
