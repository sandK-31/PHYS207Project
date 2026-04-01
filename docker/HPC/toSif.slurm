#!/bin/bash
#SBATCH --job-name=tosif
#SBATCH --output=./sif.out
#SBATCH --error=./sif.err
#SBATCH --time=1:00:00
#SBATCH --mem=8G
#SBATCH --nodes=1
#SBATCH --cpus-per-task=4
#SBATCH --partition=standard
#SBATCH --account=ijov1

make
