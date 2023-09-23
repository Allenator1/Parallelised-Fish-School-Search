#!/bin/sh

#SBATCH --account=courses0101
#SBATCH --partition=debug
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --time=00:30:00

module load gcc

cd ../src/
make all
cd ../test/
make driver

srun ./driver -f seq_pawsey.csv -p 1
srun ./driver -f parallel_pawsey.csv -p 2
srun ./driver -f parallel_schedules_pawsey.csv -p 2 -s