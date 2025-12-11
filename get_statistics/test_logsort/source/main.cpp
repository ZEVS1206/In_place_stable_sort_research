#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

int main(int argc, char **argv) 
{
    if (argc < 3) 
    {
        fprintf(stderr, "Usage: %s input_file mode(logsort|qsort)\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *mode = argv[2];

    FILE *f = fopen(filename, "r");
    if (!f) 
    {
        fprintf(stderr, "Cannot open file '%s': %s\n", filename, strerror(errno));
        return 1;
    }

    size_t capacity = 0, n = 0;
    Item *arr = NULL;

    while (1) 
    {
        int x = 0;
        if (fscanf(f, "%d", &x) != 1) break;
        if (n >= capacity) 
        {
            size_t new_cap = capacity ? capacity * 2 : 128;
            Item *tmp = (Item *)realloc(arr, new_cap * sizeof(Item));
            if (!tmp) 
            {
                fprintf(stderr, "Memory error (realloc)\n");
                fclose(f);
                free(arr);
                return 1;
            }
            arr = tmp;
            capacity = new_cap;
        }
        arr[n].key = x;
        arr[n].original_index = (int)n;
        n++;
    }
    fclose(f);

    if (n == 0) 
    {
        return 0;
    }

    if (strcmp(mode, "logsort") == 0) 
    {
        logsort(arr, n, sizeof(Item), cmp_item);
    } 
    else if (strcmp(mode, "qsort") == 0) 
    {
        qsort(arr, n, sizeof(Item), cmp_item);
    } 
    else 
    {
        fprintf(stderr, "Unknown mode '%s'. Use logsort or qsort\n", mode);
        free(arr);
        return 1;
    }

    // for (size_t i = 0; i < n; i++) 
    // {
    //     printf("%d ", arr[i].key);
    // }
    // printf("\n");

    free(arr);
    return 0;
}
