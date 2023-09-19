#ifndef UTIL_H_
#define UTIL_H_

struct Args {
    int nthreads;
    int nfish;
    int nrounds;
    int schedule;
    int chunk_size;
    bool verbose;
    int gui_grid_size;
};

float rand_range(float min_n, float max_n);

float fitness_function(float x, float y);

int median(int *data, int n);

void quantiles(int *data, int n, int *qs);

float shubert_function(float x, float y);

void check_bounds(float *x, float *y, float lake_size);

size_t get_cache_line_size(int cache);

void parse_args(int argc, char **argv, struct Args* args);

#endif