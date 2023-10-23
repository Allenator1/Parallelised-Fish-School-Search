#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <mpi.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"

unsigned int randState;
#pragma omp threadprivate(randState)

float lake_width;
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
    fitness_fn_type = args.fitness_fn;

    omp_set_num_threads(args.nthreads);
    omp_set_schedule(args.schedule, args.chunk_size);   

    int rank;               // process rank 
    int num_processes;      // number of processes

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

#pragma omp parallel
{
    randState = SEED + rank + omp_get_thread_num();
}

    int fish_pproc = args.nfish / num_processes;
    int remainder = args.nfish % num_processes;
    if (rank < remainder) {
        fish_pproc++;
    }
    fish *school = (fish*)malloc(fish_pproc * sizeof(fish));
    double start_time = omp_get_wtime();

#pragma omp parallel for schedule(runtime)
    for (int i = 0; i < fish_pproc; i++) {
        fish f;
        init_fish(&f, &randState);
        school[i] = f;
    }

#pragma omp parallel
{
    for (int i = 0; i < args.nrounds; i++) {
        float max_df = 0;
        float sum_wt = 0;
        float sum_xwt = 0;      // sum of x * wt
        float sum_ywt = 0;      // sum of y * wt

#pragma omp for schedule(runtime) reduction(max:max_df)
        // Random swimming by fish
        for (int j = 0; j < fish_pproc; j++) {
            swimfish(&school[j], &randState, STEP_IND);
            max_df = (max_df, school[j].df);
        }

#pragma omp master
        MPI_Allreduce(MPI_IN_PLACE, &max_df, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
#pragma omp barrier

#pragma omp for schedule(runtime) nowait
        // Feeding 
        for (int j = 0; j < fish_pproc; j++) {
            feedfish(&school[j], max_df);
        }

#pragma omp for schedule(runtime) reduction(+: sum_wt, sum_xwt, sum_ywt)
        // Calculate the barycenter as the weighed average of fish positions
        for (int j = 0; j < fish_pproc; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x * school[j].wt;
            sum_ywt += school[j].y * school[j].wt;
        }
        float bary_sums[3] = {sum_xwt, sum_ywt, sum_wt};

#pragma omp master
        MPI_Allreduce(MPI_IN_PLACE, &bary_sums, 3, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
#pragma omp barrier

        float bari_x = bary_sums[0] / bary_sums[2];
        float bari_y = bary_sums[1] / bary_sums[2];
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y);
    }
}
    // Gather all fish to rank 0 and print the lake if verbose option is set
    if (args.verbose) {
        fish *mega_school = NULL;
        int nfloat_pproc = fish_pproc * sizeof(fish) / sizeof(float);
        int nfloat = args.nfish * sizeof(fish) / sizeof(float);

        if (rank == 0) {
            fish *mega_school = (fish*)malloc(args.nfish * sizeof(fish));
        }

        MPI_Gather(school, nfloat_pproc, MPI_FLOAT, 
                    mega_school, nfloat, MPI_FLOAT, 0, 
                    MPI_COMM_WORLD);

        if (rank == 0) { 
            print_lake(mega_school, args.gui_grid_size);
            free(mega_school);
        }
    }
    free(school);

    // Print the time taken to run the program
    if (rank == 0) {
        double delta_time = omp_get_wtime() - start_time;
        printf("%f\n", delta_time);
    }

    MPI_Finalize();
    return 0;
}