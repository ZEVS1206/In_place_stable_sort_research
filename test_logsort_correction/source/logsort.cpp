#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logsort.h"

#define THRESHOLD_INSERTION 32
#define MERGE_BUFFER_SIZE 256

static void optimized_insertion_sort(char* array, size_t n, size_t elem_size, cmp_func_t cmp) 
{
    if (n <= 1) 
    {
        return;
    }
    
    if (elem_size <= MERGE_BUFFER_SIZE) 
    {
        char temp[MERGE_BUFFER_SIZE];
        for (size_t i = 1; i < n; i++) 
        {
            char* current = array + i * elem_size;
            memcpy(temp, current, elem_size);
            
            size_t j = i;
            while (j > 0 && cmp(array + (j-1) * elem_size, temp) > 0) 
            {
                memcpy(array + j * elem_size, array + (j-1) * elem_size, elem_size);
                j--;
            }
            
            if (j != i) 
            {
                memcpy(array + j * elem_size, temp, elem_size);
            }
        }
    } 
    else 
    {
        char* temp = (char*)calloc(elem_size, sizeof(char));
        if (!temp) 
        {
            return;
        }
        
        for (size_t i = 1; i < n; i++) 
        {
            char* current = array + i * elem_size;
            memcpy(temp, current, elem_size);
            
            size_t j = i;
            while (j > 0 && cmp(array + (j-1) * elem_size, temp) > 0) 
            {
                memcpy(array + j * elem_size, array + (j-1) * elem_size, elem_size);
                j--;
            }
            
            if (j != i) 
            {
                memcpy(array + j * elem_size, temp, elem_size);
            }
        }
        free(temp);
    }
}

void intersection_sort(char* array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp) 
{
    optimized_insertion_sort(array, size_of_array, size_of_element, cmp);
}

size_t stable_partition(void* array, size_t n, size_t elem_size, 
                       void* pivot, cmp_func_t cmp, void* buffer) 
{
    char* src = (char*)array;
    char* dst = (char*)buffer;
    
    size_t less_cnt = 0, equal_cnt = 0;
    
    for (size_t i = 0; i < n; i++) 
    {
        int res = cmp(src + i * elem_size, pivot);
        if (res < 0) 
        {
            less_cnt++;
        } 
        else if (res == 0) 
        {
            equal_cnt++;
        }
    }
    
    size_t less_idx = 0;
    size_t equal_idx = less_cnt;
    size_t greater_idx = less_cnt + equal_cnt;
    
    for (size_t i = 0; i < n; i++) 
    {
        char* elem = src + i * elem_size;
        int res = cmp(elem, pivot);
        
        if (res < 0) 
        {
            memcpy(dst + less_idx * elem_size, elem, elem_size);
            less_idx++;
        } 
        else if (res == 0) 
        {
            memcpy(dst + equal_idx * elem_size, elem, elem_size);
            equal_idx++;
        } 
        else 
        {
            memcpy(dst + greater_idx * elem_size, elem, elem_size);
            greater_idx++;
        }
    }
    
    memcpy(src, dst, n * elem_size);
    return less_cnt;
}

static void* select_pivot(void* array, size_t n, size_t elem_size, cmp_func_t cmp) 
{
    char* arr = (char*)array;
    
    if (n <= 3) 
    {
        return arr;
    }
    
    char* a = arr;
    char* b = arr + (n / 2) * elem_size;
    char* c = arr + (n - 1) * elem_size;
    
    if (cmp(a, b) < 0) 
    {
        if (cmp(b, c) < 0) return b;
        if (cmp(a, c) < 0) return c;
        return a;
    } else {
        if (cmp(a, c) < 0) return a;
        if (cmp(b, c) < 0) return c;
        return b;
    }
}

#define MAX_STACK_SIZE 128
typedef struct 
{
    void* arr;
    size_t n;
} SortFrame;

static void iterative_stable_sort(void* array, size_t n, size_t elem_size, 
                                  cmp_func_t cmp, void* buffer)
{
    SortFrame stack[MAX_STACK_SIZE];
    int top = 0;
    
    stack[top].arr = array;
    stack[top].n = n;
    
    char* temp_buffer = (char*)buffer;
    
    while (top >= 0) 
    {
        void* curr_arr = stack[top].arr;
        size_t curr_n = stack[top].n;
        top--;
        
        if (curr_n <= THRESHOLD_INSERTION) 
        {
            optimized_insertion_sort((char*)curr_arr, curr_n, elem_size, cmp);
            continue;
        }
        
        void* pivot_ptr = select_pivot(curr_arr, curr_n, elem_size, cmp);
        
        char* pivot_buf = temp_buffer;
        memcpy(pivot_buf, pivot_ptr, elem_size);
        
        char* partition_buf = temp_buffer + elem_size;
        
        size_t left_size = stable_partition(curr_arr, curr_n, elem_size, 
                                           pivot_buf, cmp, partition_buf);
        char* curr_char = (char*)curr_arr;
        size_t equal_cnt = 0;
        for (size_t i = left_size; i < curr_n; i++) 
        {
            if (cmp(curr_char + i * elem_size, pivot_buf) == 0) 
            {
                equal_cnt++;
            } 
            else 
            {
                break;
            }
        }
        
        size_t right_start = left_size + equal_cnt;
        size_t right_size = curr_n - right_start;
        
        if (right_size > left_size) 
        {
            if (right_size > 1) 
            {
                if (top + 1 < MAX_STACK_SIZE) 
                {
                    top++;
                    stack[top].arr = (char*)curr_arr + right_start * elem_size;
                    stack[top].n = right_size;
                }
            }
            if (left_size > 1) 
            {
                if (top + 1 < MAX_STACK_SIZE) 
                {
                    top++;
                    stack[top].arr = curr_arr;
                    stack[top].n = left_size;
                }
            }
        } 
        else 
        {
            if (left_size > 1) 
            {
                if (top + 1 < MAX_STACK_SIZE) 
                {
                    top++;
                    stack[top].arr = curr_arr;
                    stack[top].n = left_size;
                }
            }
            if (right_size > 1) 
            {
                if (top + 1 < MAX_STACK_SIZE) 
                {
                    top++;
                    stack[top].arr = (char*)curr_arr + right_start * elem_size;
                    stack[top].n = right_size;
                }
            }
        }
    }
}

void logsort_recursive(void* array, size_t size_of_array, size_t size_of_element, 
                      cmp_func_t cmp, void* buffer) 
{
    if (!array || !buffer || size_of_array <= 1) 
    {
        return;
    }
    
    if (size_of_array <= THRESHOLD_INSERTION) 
    {
        optimized_insertion_sort((char*)array, size_of_array, size_of_element, cmp);
        return;
    }
    
    iterative_stable_sort(array, size_of_array, size_of_element, cmp, buffer);
}

void logsort(void* array, size_t size_of_array, size_t size_of_element, cmp_func_t cmp) 
{
    if (!array || size_of_array <= 1) 
    {
        return;
    }
    
    if (size_of_array <= THRESHOLD_INSERTION) 
    {
        optimized_insertion_sort((char*)array, size_of_array, size_of_element, cmp);
        return;
    }
    
    size_t buffer_size = (size_of_array + 1) * size_of_element;
    void* buffer = calloc(buffer_size, sizeof(void));
    if (!buffer) 
    {
        optimized_insertion_sort((char*)array, size_of_array, size_of_element, cmp);
        return;
    }
    
    logsort_recursive(array, size_of_array, size_of_element, cmp, buffer);
    free(buffer);
}