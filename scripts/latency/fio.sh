#!/bin/bash
FIO=fio/fio
block_dev=/home/pfernando/temp/fiotempfile

#declare -a wl=("write" "randread" "read")
declare -a wl=("randread")
#declare -a depth=(1 2 4 6 8 10 12 14 16)
declare -a depth=(1 4 8 12 16 20 24 28 32 36 40 44)
#declare -a depth=(4)

mkdir -p results/fio
for w in "${wl[@]}"
do
	mkdir -p results/fio/${w}
	for dep in "${depth[@]}"
	do
		$FIO --filename=$block_dev --name=${w} --ioengine=libaio --iodepth=${dep} --rw=${w} --bs=4k --direct=1 --size=8G --numjobs=1 --runtime=60 > results/fio/${w}/${w}_${dep}.out
	done
done
