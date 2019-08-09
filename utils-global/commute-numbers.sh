#!/bin/bash
declare -a bf=()
#declare -a wl=("echo")
declare -a wl=("echo")
#declare -a bf=(1 2 4 6 8 10 12 16 20)
declare -a bf=(1)

#declare -a mt=("dram" "nvram")
declare -a mt=("dram")

declare -r rl=(3)

for replicas in "${rl[@]}"
do
for w in "${wl[@]}"
do
  for m in "${mt[@]}"
  do
	for b in "${bf[@]}"
	do
    ./kv_bench.py -c 
    ./kv_bench.py -g -rep "$replicas" -m "$m" -w "$w" -b "$b"
    ./kv_bench.py -dc -m "$m" -w "$w"
    ./kv_bench.py -db -m "$m" -w "$w" -commute

    ./kv_bench.py -start -m "$m" -w "$w"
	# wait for sometime
	sleep 60
    ./kv_bench.py -stop -m "$m" -w "$w"

	# gather output
    ./kv_bench.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b"
	done
  done
done
done
exit
