#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"


int main(int argc, char *argv[]) {
    srand(0);
    int grid_size = 20;
    int lake_size = 20;

    fish *school = (fish*)malloc(NUM_FISH * sizeof(fish));

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

    clock_t t;
    t = clock();

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Random swimming by fish
        float max_delta_f = 0;
        for (int j = 0; j < NUM_FISH; j++) {
            swimfish(&school[j], STEP_IND, lake_size);
            if (school[j].delta_f > max_delta_f) {
                max_delta_f = school[j].delta_f;
            }
        }

        // Feeding fish
        for (int j = 0; j < NUM_FISH; j++) {
            feedfish(&school[j], max_delta_f, INITIAL_WT);
        }

        // Calculate the barycenter as the weighed average of fish positions
        float sum_f = 0; 
        float sum_fwt = 0;  // sum of fitness * weight

        for (int j = 0; j < NUM_FISH; j++) {
            sum_f += school[j].fitness;
            sum_fwt += school[j].wt * school[j].fitness;
        }
        float bari = sum_fwt / sum_f;
    }

    print_lake(school, grid_size, lake_size, NUM_FISH);

    float delta_t =  (float)(clock() - t) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", delta_t);

    free(school);
}