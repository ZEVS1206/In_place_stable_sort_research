#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "logsort.h"

typedef struct 
{
    int key;
    int original_index;
} Item;

int cmp_item(const void *pa, const void *pb) 
{
    Item a = *(const Item *)pa;
    Item b = *(const Item *)pb;
    if (a.key < b.key) return -1;
    if (a.key > b.key) return +1;
    return 0;
}

int cmp_item_stable(const void *pa, const void *pb) 
{
    Item a = *(const Item *)pa;
    Item b = *(const Item *)pb;
    if (a.key < b.key) return -1;
    if (a.key > b.key) return +1;
    if (a.original_index < b.original_index) return -1;
    if (a.original_index > b.original_index) return +1;
    return 0;
}

void copy_array(Item *dst, const Item *src, size_t n) 
{
    memcpy(dst, src, n * sizeof(Item));
}

int is_sorted_and_stable(const Item *a, size_t n) 
{
    for (size_t i = 1; i < n; i++) 
    {
        if (a[i-1].key > a[i].key) 
        {
            return 0;
        }
        if (a[i-1].key == a[i].key) 
        {
            if (a[i-1].original_index > a[i].original_index) 
            {
                return 0;
            }
        }
    }
    return 1;
}

void fill_random(Item *a, size_t n, int max_key) {
    for (size_t i = 0; i < n; i++) {
        a[i].key = rand() % max_key;
        a[i].original_index = (int)i;
    }
}

void test_random(size_t n, int max_key) {
    Item *a = (Item *) calloc(n, sizeof(Item));
    Item *b = (Item *) calloc(n, sizeof(Item));
    if (!a || !b) { perror("malloc"); exit(1); }

    fill_random(a, n, max_key);
    copy_array(b, a, n);
    // printf("array_a_before_sort = ");
    // for (size_t index = 0; index < n; ++index)
    // {
    //     printf("%d ", a[index].key);
    // }
    // printf("\n");
    

    logsort(a, n, sizeof(Item), cmp_item);
    qsort(b, n, sizeof(Item), cmp_item);
    // printf("array_a = ");
    // for (size_t index = 0; index < n; ++index)
    // {
    //     printf("%d ", a[index].key);
    // }
    // printf("\narray_b = ");
    // for (size_t index = 0; index < n; ++index)
    // {
    //     printf("%d ", b[index].key);
    // }
    // printf("\n\n");

    //check and comparsion with quicksort
    for (size_t i = 0; i < n; i++) 
    {
        if (a[i].key != b[i].key) 
        {
            fprintf(stderr, "ERROR: values differ at i=%zu: log=%d, qsort=%d\n", i, a[i].key, b[i].key);
            exit(1);
        }
    }

    //check stable
    if (!is_sorted_and_stable(a, n)) 
    {
        fprintf(stderr, "ERROR: not stable or not sorted for n=%zu\n", n);
        exit(1);
    }

    free(a);
    free(b);
}

// Test: with repeated keys
void test_sorted(size_t n) 
{
    Item *a = (Item *) calloc(n, sizeof(Item));
    for (size_t i = 0; i < n; i++) 
    {
        a[i].key = (int)(i / 5);
        a[i].original_index = (int)i;
    }
    logsort(a, n, sizeof(Item), cmp_item);

    if (!is_sorted_and_stable(a, n)) 
    {
        fprintf(stderr, "ERROR: sorted input – failed stability for n=%zu\n", n);
        exit(1);
    }
    free(a);
}

// Test: reverse case
void test_reversed(size_t n) 
{
    Item *a = (Item *) calloc(n, sizeof(Item));
    for (size_t i = 0; i < n; i++) 
    {
        a[i].key = (int)(n - i);  
        a[i].original_index = (int)i;
    }
    logsort(a, n, sizeof(Item), cmp_item);

    if (!is_sorted_and_stable(a, n)) 
    {
        fprintf(stderr, "ERROR: reversed input – failed for n=%zu\n", n);
        exit(1);
    }
    free(a);
}

int main(void) 
{
    srand((unsigned)time(NULL));

    printf("Testing Logsort...\n");

    test_random(0, 10);
    test_random(1, 10);
    test_random(10, 5);
    test_random(50, 50);
    test_random(1000, 200);
    test_random(5000, 1000);
    printf("Random tests passed\n");

    test_sorted(1000);
    printf("Already sorted test passed\n");

    test_reversed(1000);
    printf("Reversed-order test passed\n");

    printf("All tests passed ✅\n");
    return 0;
}
