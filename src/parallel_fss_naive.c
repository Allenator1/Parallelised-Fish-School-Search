#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"


int main(int argc, char *argv[]) {
    int seed = 0;
    int grid_size = 20;
    int nt = 2;
    int lake_size = LAKE_SIZE;

    fish *school = (fish*)malloc(NUM_FISH * sizeof(fish));

    srand(seed);
    for (int i = 0; i < NUM_FISH; i++) {
        fish f = {
            .x = rand_range(-lake_size/2, lake_size/2),
            .y = rand_range(-lake_size/2, lake_size/2),
            .wt = INITIAL_WT,
            .delta_f = 0
        };
        f.fitness = fitness_function(f.x, f.y);
        school[i] = f;
    }

    print_lake(school, grid_size, lake_size, NUM_FISH);

    double start_time = omp_get_wtime();

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Random swimming by fish
        float max_delta_f = 0;
#pragma omp parallel for num_threads(nt) reduction(max:max_delta_f)
        for (int j = 0; j < NUM_FISH; j++) {
            unsigned int randomState = seed + i + omp_get_thread_num();
            float rand_x = ((float)rand_r(&randomState) / RAND_MAX * 2 - 1); // [-1, 1]
            float rand_y = ((float)rand_r(&randomState) / RAND_MAX * 2 - 1); // [-1, 1]
            swimfish(&school[j], rand_x, rand_y, STEP_IND, lake_size);
            if (school[j].delta_f > max_delta_f) {
                max_delta_f = school[j].delta_f;
            }
        }

        // Feeding 
#pragma omp parallel for num_threads(nt)
        for (int j = 0; j < NUM_FISH; j++) {
            feedfish(&school[j], max_delta_f, INITIAL_WT);
        }

        // Calculate the barycenter as the weighed average of fish positions
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

#pragma omp parallel for num_threads(nt) reduction(+:sum_wt, sum_xwt, sum_ywt) 
        for (int j = 0; j < NUM_FISH; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x + school[j].wt;
            sum_ywt += school[j].y + school[j].wt;
        }
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari); // numerical placeholder for barycenter
    }

    print_lake(school, grid_size, lake_size, NUM_FISH);

    double delta_time = omp_get_wtime() - start_time;
    printf("\nTime taken: %f seconds\n", delta_time);

    free(school);
}