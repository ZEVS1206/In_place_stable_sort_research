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
        char *tmp = (char *)calloc(size_of_element, sizeof(char));
        if (tmp == NULL)
        {
            fprintf(stderr, "Error of intersection_sort\n");
            return;
        }
        memcpy(tmp, elem, size_of_element);

        size_t index_for_search = index;
        while (index_for_search > 0 && cmp(array + (index_for_search - 1) * size_of_element, tmp) > 0)
        {
            memcpy(array + index_for_search * size_of_element, array + (index_for_search - 1) * size_of_element, size_of_element);
            index_for_search--;
        }
        memcpy(array + index_for_search * size_of_element, tmp, size_of_element);
        free(tmp);
    }
}

// Improved stable partition that properly handles equal elements
size_t stable_partition(void *array, size_t size_of_array, size_t size_of_element, void *pivot, cmp_func_t cmp, void *buffer)
{
    if (array == NULL || buffer == NULL || pivot == NULL)
    {
        fprintf(stderr, "Error in stable_partition\n");
        return 0;
    }
    
    char *A = (char *)array;
    char *B = (char *)buffer;
    
    // Count elements in each category
    size_t less_count = 0;
    size_t equal_count = 0;
    
    for (size_t i = 0; i < size_of_array; ++i)
    {
        int cmp_result = cmp(A + i * size_of_element, pivot);
        if (cmp_result < 0) {
            less_count++;
        } else if (cmp_result == 0) {
            equal_count++;
        }
    }
    
    // Copy elements to buffer in stable order
    size_t less_idx = 0;
    size_t equal_idx = less_count;
    size_t greater_idx = less_count + equal_count;
    
    for (size_t i = 0; i < size_of_array; ++i)
    {
        char *current = A + i * size_of_element;
        int cmp_result = cmp(current, pivot);
        
        if (cmp_result < 0) {
            memcpy(B + less_idx * size_of_element, current, size_of_element);
            less_idx++;
        } else if (cmp_result == 0) {
            memcpy(B + equal_idx * size_of_element, current, size_of_element);
            equal_idx++;
        } else {
            memcpy(B + greater_idx * size_of_element, current, size_of_element);
            greater_idx++;
        }
    }
    
    // Copy back to original array
    memcpy(A, B, size_of_array * size_of_element);
    return less_count;
}

// Simple pivot selection without dynamic memory allocation
void *select_pivot(void *array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp)
{
    char *A = (char *)array;
    
    // For small arrays, just use the middle element
    if (size_of_array <= 5) 
    {
        return A + (size_of_array / 2) * size_of_element;
    }
    
    // Sample 3 elements: first, middle, last
    char *first = A;
    char *middle = A + (size_of_array / 2) * size_of_element;
    char *last = A + (size_of_array - 1) * size_of_element;
    
    // Find median without dynamic allocation
    if (cmp(first, middle) < 0) {
        if (cmp(middle, last) < 0) {
            return middle;  // first < middle < last
        } else if (cmp(first, last) < 0) {
            return last;    // first < last <= middle
        } else {
            return first;   // last <= first < middle
        }
    } else {
        if (cmp(first, last) < 0) {
            return first;   // middle <= first < last
        } else if (cmp(middle, last) < 0) {
            return last;    // middle < last <= first
        } else {
            return middle;  // last <= middle <= first
        }
    }
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

    // Select pivot without dynamic allocation
    void *pivot_ptr = select_pivot(array, size_of_array, size_of_element, cmp);
    char *pivot_copy = (char *)calloc(size_of_element, sizeof(char));
    if (pivot_copy == NULL)
    {
        fprintf(stderr, "Memory allocation error in logsort_recursive\n");
        return;
    }
    memcpy(pivot_copy, pivot_ptr, size_of_element);
    
    size_t count_of_small_blocks = stable_partition(array, size_of_array, size_of_element, pivot_copy, cmp, buffer);
    free(pivot_copy);

    // Check if partition created empty sections
    if (count_of_small_blocks == 0 || count_of_small_blocks == size_of_array) {
        // All elements are equal or partition failed, use insertion sort
        intersection_sort((char *)array, size_of_array, size_of_element, cmp);
        return;
    }

    // Recursively sort both parts
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
}