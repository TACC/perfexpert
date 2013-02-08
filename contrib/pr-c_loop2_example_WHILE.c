while (i < N) {
    while (j < M) {
        c[i][j] += (a[i][j] * b[i][j]);
        j++;
    }
    i++;
}
