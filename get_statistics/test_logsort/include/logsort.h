#ifndef LOGSORT_H
#define LOGSORT_H
#include <stdio.h>

typedef int (*cmp_func_t)(const void *a, const void *b);

//intersection sort for small arrays
void intersection_sort(char *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp);

//stable partition for blocks: at the left side -> elements < pivot, at the right - > pivot
// return count of elements from the begining
size_t stable_partition(void *array, size_t size_of_array, size_t size_of_element, void *pivot, cmp_func_t cmp, void *buffer);

// recursive sort: divide and analyze -> stable partition -> intersection sort
void logsort_recursive(void *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp, void *buffer);

// general function of logsort
void logsort(void *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp);

#endif