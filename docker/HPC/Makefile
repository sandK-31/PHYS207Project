CC=g++
SHELL:=/bin/bash

all:
	source /sw/lmod/lmod/init/bash && \
	module load singularity && \
	singularity pull --arch amd64 library://sandk-33/ansg/ansg-base-env:latest
