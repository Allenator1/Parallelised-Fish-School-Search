#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"

unsigned int randomState;
#pragma omp threadprivate(randomState)

int main(int argc, char *argv[]) {
    int seed = SEED;
    float lake_w = LAKE_SIZE;
    struct Args args = {6, NUM_FISH, NUM_ITERATIONS, 4, 10, false, 20};
    parse_args(argc, argv, &args);

    fish *school = (fish*)malloc(args.nfish * sizeof(fish));

    srand(seed);
    omp_set_num_threads(args.nthreads);
    omp_set_schedule(args.schedule, args.chunk_size);    // sets the schedule and chunk_size for the parallel for regions

    for (int i = 0; i < args.nfish; i++) {
        fish f = {
            .x = rand_range(-lake_w/2, lake_w/2),
            .y = rand_range(-lake_w/2, lake_w/2),
            .wt = INITIAL_WT,
            .delta_f = 0
        };
        f.fitness = fitness_function(f.x, f.y);
        school[i] = f;
    }

    if (args.verbose) print_lake(school, args.gui_grid_size, lake_w, args.nfish);
    double start_time = omp_get_wtime();
    
#pragma omp parallel shared(seed)
{
    randomState = seed + omp_get_thread_num();  // initialise random number generator for each thread
}

    for (int i = 0; i < args.nrounds; i++) {
        float max_delta_f = 0;
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

#pragma omp parallel shared(school, max_delta_f, sum_wt, sum_xwt, sum_ywt)
{
        // Random swimming by fish
#pragma omp for schedule(runtime) reduction(max:max_delta_f)
        for (int j = 0; j < args.nfish; j++) {
            float rand_x = ((float)rand_r(&randomState) / RAND_MAX * 2 - 1); // [-1, 1]
            float rand_y = ((float)rand_r(&randomState) / RAND_MAX * 2 - 1); // [-1, 1]
            swimfish(&school[j], rand_x, rand_y, STEP_IND, lake_w);
            if (school[j].delta_f > max_delta_f) {
                max_delta_f = school[j].delta_f;
            }
        }

        // Feeding 
#pragma omp for schedule(runtime) nowait
        for (int j = 0; j < args.nfish; j++) {
            feedfish(&school[j], max_delta_f, INITIAL_WT);
        }

        // Calculate the barycenter as the weighed average of fish positions
#pragma omp for schedule(runtime) reduction(+: sum_wt, sum_xwt, sum_ywt)
        for (int j = 0; j < args.nfish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x + school[j].wt;
            sum_ywt += school[j].y + school[j].wt;
        }
}
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y); // numerical placeholder for barycenter
    }

    if (args.verbose) print_lake(school, args.gui_grid_size, lake_w, args.nfish);
    double delta_time = omp_get_wtime() - start_time;
    printf("\nTime taken: %f seconds\n", delta_time);
    free(school);
}