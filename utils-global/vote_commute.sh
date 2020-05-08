#!/bin/bash
#declare -a wl=("hashmap")
declare -a wl=("pqueue")
declare -a bf=(1 2 4 6 8 10 12 14 16 18 20)

declare -a mt=("nvram")

#declare -r rl=(1)
declare -r rl=(3)

for replicas in "${rl[@]}"
do
for w in "${wl[@]}"
do
  for m in "${mt[@]}"
  do
	for b in "${bf[@]}"
	do
    ./vote_bench.py -c 
    ./vote_bench.py -g -rep "$replicas" -m "$m" -w "$w" -b "$b"
    ./vote_bench.py -dc -m "$m" -w "$w"
    ./vote_bench.py -db -m "$m" -w "$w" -commute

    ./vote_bench.py -startsrv -m "$m" -w "$w"
    ./vote_bench.py -startclnt -m "$m" -w "$w"
	# wait for sometime
	sleep 60
    ./vote_bench.py -stop -m "$m" -w "$w"

	# gather output
    ./vote_bench.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b" -commute
	done
  done
done
done
exit
