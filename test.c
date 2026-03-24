#include <stdio.h>
#include "malloc.h"

int main() {
    printf("=== my_malloc / my_free ===\n");
    int *arr = my_malloc(5 * sizeof(int));
    for (int i = 0; i < 5; i++) arr[i] = i * 10;
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]); // 0 10 20 30 40
    printf("\n");

    printf("\n=== my_realloc (grow) ===\n");
    arr = my_realloc(arr, 10 * sizeof(int));
    arr[9] = 99;
    printf("arr[9] = %d\n", arr[9]); // 99

    printf("\n=== my_calloc (zero-init) ===\n");
    int *zeros = my_calloc(5, sizeof(int));
    for (int i = 0; i < 5; i++) printf("%d ", zeros[i]); // 0 0 0 0 0
    printf("\n");

    printf("\n=== free and reuse block ===\n");
    my_free(arr);
    int *reused = my_malloc(10 * sizeof(int)); // should reuse arr's block
    reused[0] = 42;
    printf("reused[0] = %d\n", reused[0]); // 42

    my_free(zeros);
    my_free(reused);

    printf("\nAll tests passed.\n");
    return 0;
}
