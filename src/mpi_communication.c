#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>

#include "../include/util.h"
#include "../include/fish.h"
#include "../include/constants.h"

unsigned int randState = SEED;
float lake_width;
int fitness_fn_type;

int main(int argc, char *argv[]) {
    lake_width = EUCLIDEAN_DOMAIN_WIDTH;
    fitness_fn_type = EUCLIDEAN;

    int rank;               // process rank 
    int num_processes;      // number of processes

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    
    // Create a new MPI datatype for fish 
    MPI_Datatype mpi_fish_type;
    MPI_Type_contiguous(5, MPI_FLOAT, &mpi_fish_type);
    MPI_Type_commit(&mpi_fish_type);

	double start_time = omp_get_wtime();

    int nfish_local = NUM_FISH / num_processes;
    int remainder = NUM_FISH % num_processes;
    if (rank < remainder) {
        nfish_local++;
    }

    fish *school = (fish*)malloc(nfish_local * sizeof(fish));

	fish *all_fish = NULL;
    int *fish_counts = NULL;
    int *fish_offsets = NULL;
	FILE *fp1, *fp2, *fopen();

    if (rank == 0) {
        int curr_offset = 0;
        all_fish = (fish*)malloc(NUM_FISH * sizeof(fish));
        fish_counts = (int*)malloc(num_processes * sizeof(int));
        fish_offsets = (int*)malloc(num_processes * sizeof(int));

        for (int i = 0; i < num_processes; i++) {
            fish_offsets[i] = curr_offset;
            fish_counts[i] = NUM_FISH / num_processes;
            if (i < remainder) fish_counts[i]++;
            curr_offset += fish_counts[i];
        }

        for (int i = 0; i < NUM_FISH; i++) {
            fish f;
            init_fish(&f, &randState);
            all_fish[i] = f;
        } 

		fp1 = fopen("before_communication.txt","w+");
		for (int i = 0; i < NUM_FISH; i++) {
			fprintf(fp1, "%f %f %f %f %f\n", all_fish[i].x, all_fish[i].y, all_fish[i].wt, all_fish[i].fitness, all_fish[i].df);
		}
    }

	// Scatter fish to all processes
    MPI_Scatterv(all_fish, fish_counts, fish_offsets, mpi_fish_type, 
                 school, nfish_local, mpi_fish_type, 0, MPI_COMM_WORLD);    

    // Gather all fish to rank 0
    MPI_Gatherv(school, nfish_local, mpi_fish_type, 
                all_fish, fish_counts, fish_offsets,
                mpi_fish_type, 0, MPI_COMM_WORLD);

        if (rank == 0) { 
			fp2 = fopen("after_communication.txt","w+");
			for (int i = 0; i < NUM_FISH; i++) {
				fprintf(fp2, "%f %f %f %f %f\n", all_fish[i].x, all_fish[i].y, all_fish[i].wt, all_fish[i].fitness, all_fish[i].df);
			}
            printf("%f\n", omp_get_wtime() - start_time);
            
            free(all_fish);
            free(fish_counts);
            free(fish_offsets);
        }
    free(school);

    MPI_Finalize();
    return 0;
}