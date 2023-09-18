#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../include/util.h"


float rand_range(float min_n, float max_n) 
{
    return (float)rand() / RAND_MAX * (max_n - min_n) + min_n;
}


float fitness_function(float x, float y) 
{
    return (float)sqrt(x * x + y * y);
}


int int_compare(const void *a, const void *b) 
{
    int f = *((int*)a);
    int s = *((int*)b);
    return (f > s) - (f < s);
}


int median(int *data, int n) 
{
    if (n % 2 == 0) {
        return (data[n / 2] + data[n / 2 + 1]) / 2;
    } else {
        return data[n / 2 + 1];
    }
}


void quantiles(int *data, int n, int *qs) 
{
    int *sorted_data = malloc(n * sizeof(*data));
    memcpy(sorted_data, data, n * sizeof(*data));
    qsort(sorted_data, n, sizeof(*data), int_compare);

    qs[1] = median(sorted_data, n / 2);          // q1
    qs[2] = median(sorted_data + n / 2, n / 2);  // q3
    float iqr = qs[2] - qs[1];
    qs[0] = qs[1] - 1.5 * iqr;                  // lower tail
    qs[3] = qs[2] + 1.5 * iqr;                  // upper tail

    free(sorted_data);
}


float shubert_function(float x, float y) 
{
    float f_x1 = 0;
    float f_x2 = 0;
    for (int i = 1; i < 6; i++) {
        f_x1 += i * cos((i + 1) * x + i);
        f_x2 += i * cos((i + 1) * y + i);
    }
    return f_x1 * f_x2;
}


void check_bounds(float *x, float *y, float lake_size) 
{
    if (*x < -lake_size/2) *x = -lake_size/2;
    if (*y < -lake_size/2) *y = -lake_size/2;
    if (*x > lake_size/2) *x = lake_size/2;
    if (*y > lake_size/2) *y = lake_size/2;
}


size_t get_cache_line_size(int cache) {
    FILE * p = 0;
    char line[256];
    sprintf(line, "/sys/devices/system/cpu/cpu1/cache/index%d/coherency_line_size", cache);
    p = fopen(line, "r");
    unsigned int i = 0;
    if (p) {
        fscanf(p, "%d", &i);
        fclose(p);
    }
    return i;
}