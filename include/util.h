#ifndef UTIL_H_
#define UTIL_H_

# define M_PI        3.14159265358979323846

struct Args {
    int nthreads;
    int nfish;
    int nrounds;
    int schedule;
    int chunk_size;
    bool verbose;
    int gui_grid_size;
    int fitness_fn;
};

enum objective_fns {EUCLIDEAN = 1, SHUBERT = 2, RASTRIGIN = 3};

float fitness_function(float x, float y, int fn);

int median(int *data, int n);

void quantiles(int *data, int n, int *qs);

float dist(float x1, float y1, float x2, float y2);

float shubert_function(float x, float y);

float rastrigin_function(float x, float y);

void check_bounds(float *x, float *y, float lake_size);

size_t get_cache_line_size(int cache);

void parse_args(int argc, char **argv, struct Args* args);

#endif