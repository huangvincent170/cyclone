#!/bin/bash

# download pmemkv bench from https://github.com/pmem/pmemkv-tools.git
# download and install pmemkv from https://github.com/pmem/pmemkv.git
# download and install following software versions
# pmdk 1.7
# libpmeobj-cpp 1.8
# pmemkv 1.0
# pmemkv-tools (master)

declare -a rwmix=(0 50)
declare -a nthreads=(1 2 4 8)

db_file=/mnt/pmem1/pmemkv
#db_file=./dbpmem
#db_file=/mnt/pmem1/pmemkv

COMMON_EXEC="PMEM_IS_PMEM_FORCE=1 ./pmemkv-tools/pmemkv_bench
--db=/mnt/pmem1/pmemkv
--db_size_in_gb=2
--key_size=16
--value_size=16"

for rw in "${rwmix[@]}"
do
	rm /mnt/pmem1/pmemkv
	
	eval $COMMON_EXEC "--benchmarks=fillseq,readrandomwriterandom --readwritepercent=$rw"

	mkdir -p results/pmemkv/randomrw
	for th in "${nthreads[@]}"
	do
		eval $COMMON_EXEC "--benchmarks=readrandomwriterandom --readwritepercent=$rw --threads=$th" > results/pmemkv/randomrw/pmemkv_${rw}_${th}.out 
	done
done
