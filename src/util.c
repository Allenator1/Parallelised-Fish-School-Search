#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/util.h"
#include "../include/constants.h"


float fitness_function(float x, float y) 
{
    float ret_val = 0.0f;
    if (fitness_fn_type == EUCLIDEAN) {
        ret_val = dist(x, y, 0.0, 0.0);
    } 
    else if (fitness_fn_type == SHUBERT) {
        ret_val = shubert_function(x, y);
    }
    else if (fitness_fn_type == RASTRIGIN) {
        ret_val = rastrigin_function(x, y);
    }
    else {
        return -1;
    }
    return ret_val;
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

float dist(float x1, float y1, float x2, float y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
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


float rastrigin_function(float x, float y) {
    float x_com = x * x - 10 * cos(2 * M_PI * x);
    float y_com = y * y - 10 * cos(2 * M_PI * y);
    return 20 + x_com + y_com;
}


void check_bounds(float *x, float *y) 
{
    if (*x < -lake_width/2) *x = -lake_width/2;
    if (*y < -lake_width/2) *y = -lake_width/2;
    if (*x > lake_width/2) *x = lake_width/2;
    if (*y > lake_width/2) *y = lake_width/2;
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


void parse_args(int argc, char **argv, struct Args* args) {
    int opt;
    while ((opt = getopt(argc, argv, "t:n:r:s:c:vg:f:"))!= -1) {
        switch (opt) {
            case 't': 
                args->nthreads = atoi(optarg); break;
            case 'n': 
                args->nfish = atoi(optarg); break;
            case 'r': 
                args->nrounds = atoi(optarg); break;
            case 's': 
                args->schedule = atoi(optarg); break;       // 1: static, 2: dynamic, 3: dynamic, 4: auto
            case 'c': 
                args->chunk_size = atoi(optarg); break;
            case 'v': 
                args->verbose = true; break;
            case 'g': 
                args->gui_grid_size = atoi(optarg); break;
            case 'f':
                args->fitness_fn = atoi(optarg); break;
            default: break;
        }
    }
}