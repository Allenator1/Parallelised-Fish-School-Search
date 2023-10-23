#include<stdio.h>
#include<omp.h>
#include<mpi.h>


int main(int argc, char* argv[])
{
	int process_id, number_of_processes;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&process_id);
	MPI_Comm_size(MPI_COMM_WORLD,&number_of_processes);
	#pragma omp parallel
	printf("Hello world, I am process %d among %d processes and thread_id %d among %d threads\n",process_id,number_of_processes,omp_get_thread_num(),omp_get_num_threads());
	MPI_Finalize();
}
