#!/bin/bash
#SBATCH --job-name=mpi_matr          
#SBATCH --time=00:10:00              
#SBATCH --ntasks=8                   
#SBATCH --nodes=1                    
#SBATCH --partition=batch            

module load intel/mpi4

mpirun -r ssh ./main                       