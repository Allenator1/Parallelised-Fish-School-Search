#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"

float lake_width;
int number_of_fish;
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
    number_of_fish = args.nfish;
    fitness_fn_type = args.fitness_fn;
    fish *school = (fish*)malloc(number_of_fish * sizeof(fish));

    unsigned int randState = SEED;
    for (int i = 0; i < number_of_fish; i++) {
        fish f;
        init_fish(&f, &randState);
        school[i] = f;
    }

    if (args.verbose) print_lake(school, args.gui_grid_size);

    clock_t t;
    t = clock();
    float step_ind = STEP_IND;
    float step_vol = STEP_VOL;
    float school_weight = INITIAL_WT * number_of_fish;
    bool school_weight_improved = false;

    for (int i = 0; i < args.nrounds; i++) {
        // Apply random individual movement
        float max_delta_f = 0;
        for (int j = 0; j < number_of_fish; j++) {
            swimfish(&school[j], &randState, step_ind);
            if (school[j].df > max_delta_f) {
                max_delta_f = school[j].df;
            }
        }

        // Apply feeding operator
        float prev_school_weight = school_weight;
        for (int j = 0; j < number_of_fish; j++) {
            if (!school[j].moved) continue;
            feedfish(&school[j], max_delta_f);
            school_weight += school[j].wt;
        }
        if (school_weight > prev_school_weight) school_weight_improved = true;

        // Calculate intinctive movement as weighted average of fish movement in school
        float sum_dxdf = 0;
        float sum_dydf = 0;
        float sum_df = 0;

        for (int j = 0; j < number_of_fish; j++) {
            if (!school[j].moved) continue;
            sum_dxdf += school[j].dx * school[j].df;
            sum_dydf += school[j].dy * school[j].df;
            sum_df += school[j].df;
        }
        float xI = sum_dxdf / sum_df;   
        float yI = sum_dydf / sum_df;

        // Calculate barycentre coordinates as weighted average of fish coordinates in school
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

        for (int j = 0; j < number_of_fish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x + school[j].wt;
            sum_ywt += school[j].y + school[j].wt;
        }
        float xB = sum_xwt / sum_wt;   
        float yB = sum_ywt / sum_wt;

        // Apply collective (instinctive and volitive) movement
        for (int j = 0; j < number_of_fish; j++) {
            collective_move(&school[j], &randState, xI, yI, xB, yB, school_weight_improved, step_vol);
        }

        step_ind -= step_ind / (args.nrounds + 1);
        step_vol -= step_vol / (args.nrounds + 1);
    }

    if (args.verbose) print_lake(school, args.gui_grid_size);

    float delta_t =  (float)(clock() - t) / CLOCKS_PER_SEC;
    printf("\nTime taken: %f seconds\n", delta_t);

    free(school);
}