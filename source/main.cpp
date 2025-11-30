#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "logsort.h"

static double now_sec(void) 
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0.0;
    }
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

#define TIMER_START()  double _timer_start = now_sec()
#define TIMER_ELAPSED() (now_sec() - _timer_start)


static void fill_random(int *a, size_t size, int max_key) 
{
    for (size_t index = 0; index < size; index++) 
    {
        a[index] = rand() % max_key;
    }
    return;
}

int cmp(const void *pa, const void *pb)
{
    int a = *(const int *)pa;
    int b = *(const int *)pb;
    return a > b;
}

void test_random(size_t size, int max_elem)
{
    int *array = (int *) calloc(size, sizeof(int));
    if (array == NULL)
    {
        fprintf(stderr, "Error of getting memory for array\n");
        return;
    }
    fill_random(array, size, max_elem);

    TIMER_START();

    logsort(array, size, sizeof(int), cmp);

    double elapsed = TIMER_ELAPSED();
    printf("Elapsed: %.6f sec\n", elapsed);
    free(array);
    return;
}


int main(void) {
    srand((unsigned)time(NULL));

    test_random(10, 10);
    test_random(100, 50);
    test_random(100, 2000);
    test_random(1000, 1000);
    // int array[10] = {1, 8, 7, 6, 6, 3, 4, 6, 1, 2};
    // logsort(array, 10, sizeof(int), cmp);
    // for (size_t index = 0; index < 10; ++index)
    // {
    //     printf("%d ", array[index]);
    // }
    // printf("\n");
    return 0;
}
