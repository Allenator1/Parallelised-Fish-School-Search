#ifndef UTIL_H_
#define UTIL_H_


float rand_range(float min_n, float max_n);

float fitness_function(float x, float y);

int median(int *data, int n);

void quantiles(int *data, int n, int *qs);

float shubert_function(float x, float y);

void check_bounds(float *x, float *y, float lake_size);

size_t get_cache_line_size(int cache);

#endif