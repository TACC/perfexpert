#if 0
[macpo-integration-test]:init:1:1:
[macpo-integration-test]:write_idx:array:5:
[macpo-integration-test]:record:2:15:?:0:4:
[macpo-integration-test]:record:1:16:?:0:4:
[macpo-integration-test]:record:1:17:?:0:4:
#endif

int foo(int x) {
    return x;
}

int main() {
    int array[5] __attribute__((unused));
    array[4] = 5;
    foo(array[2]);
    int x = array[1];
    return x;
}
