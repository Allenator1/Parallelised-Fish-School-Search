#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"

float lake_width;
int fitness_fn_type;

int main(int argc, char *argv[]) {
    struct Args args = {.nfish=NUM_FISH, .nrounds=NUM_ITERATIONS, 
        .verbose=false, .gui_grid_size=20, .fitness_fn=EUCLIDEAN};
    parse_args(argc, argv, &args);

    lake_width = EUCLIDEAN_DOMAIN_WIDTH;
    if (args.fitness_fn == SHUBERT) {
        lake_width = SHUBERT_DOMAIN_WIDTH;
    } else if (args.fitness_fn == RASTRIGIN) {
        lake_width = RASTRIGIN_DOMAIN_WIDTH;
    }
    fitness_fn_type = args.fitness_fn;
    fish *school = (fish*)malloc(args.nfish * sizeof(fish));

    unsigned int randState = SEED;
    for (int i = 0; i < args.nfish; i++) {
        fish f;
        init_fish(&f, &randState);
        school[i] = f;
    }

    clock_t t;
    t = clock();

    for (int i = 0; i < args.nrounds; i++) {
        // Random swimming by fish
        float max_df = 0;
        for (int j = 0; j < args.nfish; j++) {
            swimfish(&school[j], &randState, STEP_IND);
            if (school[j].df > max_df) {
                max_df = school[j].df;
            }
        }

        // Feeding fish
        for (int j = 0; j < args.nfish; j++) {
            feedfish(&school[j], max_df);
        }

        // Calculate the barycenter as the weighed average of fish positions
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

        for (int j = 0; j < args.nfish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x * school[j].wt;
            sum_ywt += school[j].y * school[j].wt;
        }
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y); // numerical placeholder for barycenter
    }

    if (args.verbose) print_lake(school, args.gui_grid_size, args.nfish);

    float delta_t =  (float)(clock() - t) / CLOCKS_PER_SEC;
    printf("%f\n", delta_t);

    free(school);
}