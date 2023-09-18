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
    int chunk_size = 4;
    int grid_w = 20;     
    int nt = 6;
    float lake_w = LAKE_SIZE;
    int num_fish = NUM_FISH;

    fish *school = (fish*)malloc(num_fish * sizeof(fish));

    srand(seed);
    for (int i = 0; i < num_fish; i++) {
        fish f = {
            .x = rand_range(-lake_w/2, lake_w/2),
            .y = rand_range(-lake_w/2, lake_w/2),
            .wt = INITIAL_WT,
            .delta_f = 0
        };
        f.fitness = fitness_function(f.x, f.y);
        school[i] = f;
    }

    print_lake(school, grid_w, lake_w, num_fish);
    double start_time = omp_get_wtime();

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        float max_delta_f = 0;
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

#pragma omp parallel num_threads(nt) shared(school, max_delta_f, sum_wt, sum_xwt, sum_ywt)
{
        unsigned int randomState = seed + i + omp_get_thread_num();

        // Random swimming by fish
#pragma omp for schedule(static, chunk_size) reduction(max:max_delta_f)
        for (int j = 0; j < num_fish; j++) {
            float rand_x = ((float)rand_r(&randomState) / RAND_MAX * 2 - 1); // [-1, 1]
            float rand_y = ((float)rand_r(&randomState) / RAND_MAX * 2 - 1); // [-1, 1]
            swimfish(&school[j], rand_x, rand_y, STEP_IND, lake_w);
            if (school[j].delta_f > max_delta_f) {
                max_delta_f = school[j].delta_f;
            }
        }

        // Feeding 
#pragma omp for schedule(static, chunk_size) nowait
        for (int j = 0; j < num_fish; j++) {
            feedfish(&school[j], max_delta_f, INITIAL_WT);
        }

        // Calculate the barycenter as the weighed average of fish positions
#pragma omp for schedule(static, chunk_size) reduction(+: sum_wt, sum_xwt, sum_ywt)
        for (int j = 0; j < num_fish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x + school[j].wt;
            sum_ywt += school[j].y + school[j].wt;
        }
}
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y); // numerical placeholder for barycenter
    }

    print_lake(school, grid_w, lake_w, num_fish);

    double delta_time = omp_get_wtime() - start_time;
    printf("\nTime taken: %f seconds\n", delta_time);

    free(school);
}