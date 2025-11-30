#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logsort.h"

void intersection_sort(char *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp)
{
    if (array == NULL)
    {
        fprintf(stderr, "Error of intersection_sort\n");
        return;
    }
    for (size_t index = 1; index < size_of_array; ++index)
    {
        char *elem = array + index * size_of_element;
        char *tmp = (char *) calloc(size_of_element, sizeof(char));
        if (tmp == NULL)
        {
            fprintf(stderr, "Error of intersection_sort\n");
            return;
        }
        memcpy(tmp, elem, size_of_element);

        size_t index_for_search = index;
        while (index_for_search > 0 && cmp (array + (index_for_search - 1) * size_of_element, tmp) > 0)
        {
            memcpy(array + index_for_search * size_of_element, array + (index_for_search - 1) * size_of_element, size_of_element);
            index_for_search--;
        }
        memcpy(array + index_for_search * size_of_element, tmp, size_of_element);
        free(tmp);
        tmp = NULL;
    }
    return;
}

size_t stable_partition(void *array, size_t size_of_array, size_t size_of_element, void *pivot, cmp_func_t cmp, void *buffer)
{
    if (array == NULL || buffer == NULL || pivot == NULL)
    {
        fprintf(stderr, "Error in stable_partition\n");
        return 0;
    }
    char *A = (char *)array;
    char *B = (char *)buffer;
    size_t count_of_small_blocks = 0;
    size_t count_of_huge_blocks = 0;
    
    for (size_t index = 0; index < size_of_array; ++index)
    {
        char *elem = A + index * size_of_element;
        if (cmp(elem, pivot) < 0)
        {
            memcpy(B + count_of_small_blocks * size_of_element, elem, size_of_element);
            count_of_small_blocks++;
        }
    }

    for (size_t index = 0; index < size_of_array; ++index)
    {
        char *elem = A + index * size_of_element;
        if (cmp(elem, pivot) >= 0)
        {
            memcpy(B + count_of_huge_blocks * size_of_element, elem, size_of_element);
            count_of_huge_blocks++;
        }
    }
    memcpy(A, B, size_of_array * size_of_element);
    return count_of_small_blocks;
}

void logsort_recursive(void *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp, void *buffer)
{
    if (array == NULL || buffer == NULL)
    {
        fprintf(stderr, "Error in logsort_recursive\n");
        return;
    }
    
    if (size_of_array <= 16)
    {
        intersection_sort((char *)array, size_of_array, size_of_element, cmp);
        return;
    }

    void *pivot = (char *)array + (size_of_array / 2) * size_of_element;//get pivot as mediana
    char *pivot_copy = (char *) calloc(size_of_element, sizeof(char));
    if (pivot_copy == NULL)
    {
        fprintf(stderr, "Memory allocation error in logsort_recursive\n");
        return;
    }
    memcpy(pivot_copy, pivot, size_of_element);
    size_t count_of_small_blocks = stable_partition(array, size_of_array, size_of_element, pivot_copy, cmp, buffer);
    free(pivot_copy);

    if (count_of_small_blocks == 0)
    {
        int all_equal = 1;
        for (size_t index = 1; index < size_of_array; index++)
        {
            if (cmp((char *)array, (char *)array + index * size_of_element) != 0)
            {
                all_equal = 0;
                break;
            }
        }
        if (all_equal)
        {
            return;
        }
        count_of_small_blocks = 1;
    }
    else if (count_of_small_blocks == size_of_array)
    {
        int all_equal = 1;
        for (size_t index = 1; index < size_of_array; index++)
        {
            if (cmp((char *)array, (char *)array + index * size_of_element) != 0)
            {
                all_equal = 0;
                break;
            }
        }
        if (all_equal)
        {
            return;
        }
        count_of_small_blocks = size_of_array - 1;
    }

    logsort_recursive(array, count_of_small_blocks, size_of_element, cmp, buffer);
    logsort_recursive((char *)array + count_of_small_blocks * size_of_element, 
                     size_of_array - count_of_small_blocks, size_of_element, cmp, buffer);
}

void logsort(void *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp)
{
    if (array == NULL)
    {
        fprintf(stderr, "Error in logsort\n");
        return;
    }
    if (size_of_array <= 1)
    {
        return;
    }

    void *buffer = calloc(size_of_array, size_of_element);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error of getting memory for buffer\n");
        return;
    }

    logsort_recursive(array, size_of_array, size_of_element, cmp, buffer);
    free(buffer);
    return;
}