#!/bin/sh

#SBATCH --account=courses0101
#SBATCH --partition=debug
#SBATCH --ntasks=${1}
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=32
#SBATCH --time=00:30:00

module load gcc

gcc -fopenmp -lm driver.c -o driver

srun ./driver -f parallel_pawsey.csv -p 2 -n ${1}
srun ./driver -f parallel_schedules_pawsey.csv -p 2 -n ${1} -s