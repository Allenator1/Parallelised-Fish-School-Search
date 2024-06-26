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

unsigned int randState;
#pragma omp threadprivate(randState)

float lake_width;
int number_of_fish;
int fitness_fn_type;

int main(int argc, char *argv[]) {
    struct Args args = {omp_get_max_threads(), NUM_FISH, NUM_ITERATIONS, 4, 10, false, 20, EUCLIDEAN};
    lake_width = EUCLIDEAN_DOMAIN_WIDTH;
    parse_args(argc, argv, &args);

    if (args.fitness_fn == SHUBERT) {
        lake_width = SHUBERT_DOMAIN_WIDTH;
    } else if (args.fitness_fn == RASTRIGIN) {
        lake_width = RASTRIGIN_DOMAIN_WIDTH;
    }
    number_of_fish = args.nfish;
    fitness_fn_type = args.fitness_fn;
    fish *school = (fish*)malloc(number_of_fish * sizeof(fish));

    omp_set_num_threads(args.nthreads);
    omp_set_schedule(args.schedule, args.chunk_size);    // sets the schedule and chunk_size for the parallel for regions

#pragma omp parallel
{
    randState = SEED + omp_get_thread_num();  // initialise random number generator for each thread
}

    // Initialise the fish school
    for (int i = 0; i < number_of_fish; i++) {
        fish f;
        init_fish(&f, &randState);
        school[i] = f;
    }
    if (args.verbose) print_lake(school, args.gui_grid_size);
    double start_time = omp_get_wtime();

    for (int i = 0; i < args.nrounds; i++) {
        float max_df = 0;
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

#pragma omp parallel
{
#pragma omp for schedule(runtime) reduction(max:max_df)
        // Random swimming by fish
        for (int j = 0; j < number_of_fish; j++) {
            swimfish(&school[j], &randState, STEP_IND);
            if (school[j].df > max_df) {
                max_df = school[j].df;
            }
        }

#pragma omp for schedule(runtime) nowait
        // Feeding 
        for (int j = 0; j < number_of_fish; j++) {
            feedfish(&school[j], max_df);
        }

#pragma omp for schedule(runtime) reduction(+: sum_wt, sum_xwt, sum_ywt)
        // Calculate the barycenter as the weighed average of fish positions
        for (int j = 0; j < number_of_fish; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x * school[j].wt;
            sum_ywt += school[j].y * school[j].wt;
        }
}
        float bari_x = sum_xwt / sum_wt;
        float bari_y = sum_ywt / sum_wt;
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y); // numerical placeholder for barycenter
    }

    if (args.verbose) print_lake(school, args.gui_grid_size);
    double delta_time = omp_get_wtime() - start_time;
    printf("%f\n", delta_time);
    free(school);
}