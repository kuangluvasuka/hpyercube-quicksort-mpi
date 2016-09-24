#!/bin/bash
source /usr/usc/openmpi/1.8.8/setup.sh
make
mpirun -np 8 quicksort

