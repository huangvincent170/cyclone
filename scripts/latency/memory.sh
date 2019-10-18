#!/bin/bash

mkdir -p results/mlc
#download mlc: https://software.intel.com/en-us/articles/intelr-memory-latency-checker.
# checkout aep sweep scripts from pradeepfn/mlc git repo. (private for license reasons)

#PMEM
#sudo ./mlc/aep_perf_sweep.sh -m ./Linux/mlc -p /mnt/pmem1 > results/mlc/PMEM.out

#DRAM: 
sudo ./mlc/dram_perf_sweep.sh -m ./Linux/mlc > results/mlc/DRAM.out
