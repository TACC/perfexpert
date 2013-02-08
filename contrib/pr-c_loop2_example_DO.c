do {
    do {
        c[i][j] += (a[i][j] * b[i][j]);
        j++;
    } while (j < M);
    i++;
} while (i < N);
