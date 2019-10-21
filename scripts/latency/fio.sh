#!/bin/bash
FIO=fio/fio
block_dev=/home/pfernando/temp/fiotempfile

declare -a wl=("write" "randread" "read")
#declare -a wl=("randwrite")
declare -a depth=(1 2 4 6 8 10 12 14 16)
#declare -a depth=(2)

mkdir -p results/fio
for w in "${wl[@]}"
do
	mkdir -p results/fio/${w}
	for dep in "${depth[@]}"
	do
		$FIO --filename=$block_dev --name=randwrite --ioengine=libaio --iodepth=${dep} --rw=${w} --bs=4k --direct=1 --size=1G --numjobs=1 --runtime=60 > results/fio/${w}/${w}_${dep}.out
	done
done
