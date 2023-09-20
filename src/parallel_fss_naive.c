#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"

unsigned int randState;
#pragma omp threadprivate(randState)

int main(int argc, char *argv[]) {
    struct Args args = {.nthreads=6, .nfish=NUM_FISH, .nrounds=NUM_ITERATIONS, 
        .verbose=false, .gui_grid_size=20, .fitness_fn=EUCLIDEAN};
    float lake_w = EUCLIDEAN_DOMAIN_WIDTH;
    if (args.fitness_fn == SHUBERT) {
        lake_w = SHUBERT_DOMAIN_WIDTH;
    } else if (args.fitness_fn == RASTRIGIN) {
        lake_w = RASTRIGIN_DOMAIN_WIDTH;
    }

    parse_args(argc, argv, &args);
    fish *school = (fish*)malloc(args.nfish * sizeof(fish));
    omp_set_num_threads(args.nthreads);

#pragma omp parallel
{
    randState = SEED + omp_get_thread_num();  // initialise random number generator for each thread
}   
    // Initialise the fish school
    for (int i = 0; i < args.nfish; i++) {
        fish f;
        init_fish(&f, &randState, lake_w, args.fitness_fn);
        school[i] = f;
    }
    if (args.verbose) print_lake(school, args.gui_grid_size, lake_w, args.nfish);
    double start_time = omp_get_wtime();

    // Run the simulation
    for (int i = 0; i < args.nrounds; i++) {
        float max_delta_f = 0;

#pragma omp parallel for reduction(max:max_delta_f)
        // Random swimming by fish
        for (int j = 0; j < args.nfish; j++) {
            swimfish(&school[j], &randState, lake_w, args.fitness_fn);
            if (abs(school[j].delta_f) > max_delta_f) {
                max_delta_f = abs(school[j].delta_f);
            }
        }

#pragma omp parallel for
        // Feeding 
        for (int j = 0; j < args.nfish; j++) {
            feedfish(&school[j], max_delta_f);
        }

        // Calculate the barycenter as the weighed average of fish positions
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

#pragma omp parallel for reduction(+:sum_wt, sum_xwt, sum_ywt) 
        for (int j = 0; j < args.nfish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x + school[j].wt;
            sum_ywt += school[j].y + school[j].wt;
        }
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari); // numerical placeholder for barycenter
    }

    if (args.verbose) print_lake(school, args.gui_grid_size, lake_w, args.nfish);
    double delta_time = omp_get_wtime() - start_time;
    printf("\nTime taken: %f seconds\n", delta_time);
    free(school);
}