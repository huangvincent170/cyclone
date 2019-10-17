#!/bin/bash

declare -a wl=("micro_rread")
declare -a iosize=("1k" "2k")

mkdir -p results/filebench
for w in "${wl[@]}"
do
	for i in "${iosize[@]}"
	do
		./filebench.py -io 64 -w micro_rread > results/filebench/${w}_${i}.out

	done
done
