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
    
    // Create a new MPI datatype for fish 
    MPI_Datatype mpi_fish_type;
    MPI_Type_contiguous(5, MPI_FLOAT, &mpi_fish_type);
    MPI_Type_commit(&mpi_fish_type);

    // Seed the random number generator uniquely for each process and thread
#pragma omp parallel
{
    randState = SEED + rank + omp_get_thread_num();
}   
    fish *all_fish = NULL;
    int *fish_counts = NULL;
    int *fish_offsets = NULL;

    int nfish_local = args.nfish / num_processes;
    int remainder = args.nfish % num_processes;
    if (rank < remainder) {
        nfish_local++;
    }
    
    fish *school = (fish*)malloc(nfish_local * sizeof(fish));
    double start_time = omp_get_wtime();

    if (rank == 0) {
        int curr_offset = 0;
        all_fish = (fish*)malloc(args.nfish * sizeof(fish));
        fish_counts = (int*)malloc(num_processes * sizeof(int));
        fish_offsets = (int*)malloc(num_processes * sizeof(int));

        for (int i = 0; i < num_processes; i++) {
            fish_offsets[i] = curr_offset;
            fish_counts[i] = args.nfish / num_processes;
            if (i < remainder) fish_counts[i]++;
            curr_offset += fish_counts[i];
        }

#pragma omp parallel for schedule(runtime)
        // Generate all fish at root process (rank = 0)
        for (int i = 0; i < args.nfish; i++) {
            fish f;
            init_fish(&f, &randState);
            all_fish[i] = f;
        } 
    }

    // Scatter fish to all processes
    MPI_Scatterv(all_fish, fish_counts, fish_offsets, mpi_fish_type, 
                 school, nfish_local, mpi_fish_type, 0, MPI_COMM_WORLD);    

    float max_df = 0; 
    float sum_wt = 0; 
    float sum_xwt = 0; 
    float sum_ywt = 0;

#pragma omp parallel shared(max_df, sum_wt, sum_xwt, sum_ywt)
{   
    for (int i = 0; i < args.nrounds; i++) {

#pragma omp for schedule(runtime) reduction(max:max_df)
        // Random swimming by fish
        for (int j = 0; j < nfish_local; j++) {
            swimfish(&school[j], &randState, STEP_IND);
            max_df = fmax(max_df, school[j].df);
        }

#pragma omp master
        MPI_Allreduce(MPI_IN_PLACE, &max_df, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

#pragma omp single
{
        sum_wt = 0;
        sum_xwt = 0;      // sum of x * wt
        sum_ywt = 0;      // sum of y * wt
}

#pragma omp for schedule(runtime) nowait
        // Feeding 
        for (int j = 0; j < nfish_local; j++) {
            feedfish(&school[j], max_df);
        }

#pragma omp for schedule(runtime) reduction(+: sum_wt, sum_xwt, sum_ywt)
        // Calculate the barycenter as the weighed average of fish positions
        for (int j = 0; j < nfish_local; j++) {
            sum_wt += school[j].wt;
            sum_xwt += school[j].x * school[j].wt;
            sum_ywt += school[j].y * school[j].wt;
        }
        float bary_sums[3] = {sum_xwt, sum_ywt, sum_wt};

#pragma omp master
        MPI_Allreduce(MPI_IN_PLACE, &bary_sums, 3, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

#pragma omp single
        max_df = 0;

        float bari_x = bary_sums[0] / bary_sums[2];
        float bari_y = bary_sums[1] / bary_sums[2];
        float bari = sqrt(bari_x * bari_x + bari_y * bari_y);
    }
}
    // Gather all fish to rank 0
    MPI_Gatherv(school, nfish_local, mpi_fish_type, 
                all_fish, fish_counts, fish_offsets,
                mpi_fish_type, 0, MPI_COMM_WORLD);

        if (rank == 0) { 
            if (args.verbose) print_lake(all_fish, args.gui_grid_size, args.nfish);
            printf("%f\n", omp_get_wtime() - start_time);

            free(all_fish);
            free(fish_counts);
            free(fish_offsets);
        }
    free(school);

    MPI_Finalize();
    return 0;
}