#!/bin/bash

declare -a wl=("micro_rread")
declare -a iosize=("64" "256" "1k" "4k" "16k" "64k"  "256k" "1m")

mkdir -p results/filebench
for w in "${wl[@]}"
do
	for i in "${iosize[@]}"
	do
		./filebench.py -io ${i} -w micro_rread > results/filebench/${w}_${i}.out

	done
done
