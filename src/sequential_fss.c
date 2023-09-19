#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"


int main(int argc, char *argv[]) {
    int seed = 0;
    int lake_w = LAKE_SIZE;
    struct Args args = {.nfish=NUM_FISH, .nrounds=NUM_ITERATIONS, 
        .verbose=false, .gui_grid_size=20};
    parse_args(argc, argv, &args);

    fish *school = (fish*)malloc(args.nfish * sizeof(fish));

    srand(seed);
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

    clock_t t;
    t = clock();

    for (int i = 0; i < args.nrounds; i++) {
        // Random swimming by fish
        float max_delta_f = 0;
        for (int j = 0; j < args.nfish; j++) {
            float rand_x = ((float)rand() / RAND_MAX * 2 - 1); // [-1, 1]
            float rand_y = ((float)rand() / RAND_MAX * 2 - 1); // [-1, 1]
            swimfish(&school[j], rand_x, rand_y, STEP_IND, lake_w);
            if (school[j].delta_f > max_delta_f) {
                max_delta_f = school[j].delta_f;
            }
        }

        // Feeding fish
        for (int j = 0; j < args.nfish; j++) {
            feedfish(&school[j], max_delta_f, INITIAL_WT);
        }

        // Calculate the barycenter as the weighed average of fish positions
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

        for (int j = 0; j < args.nfish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x + school[j].wt;
            sum_ywt += school[j].y + school[j].wt;
        }
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y); // numerical placeholder for barycenter
    }

    if (args.verbose) print_lake(school, args.gui_grid_size, lake_w, args.nfish);

    float delta_t =  (float)(clock() - t) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", delta_t);

    free(school);
}