#!/bin/bash
#declare -a wl=("hashmap")
declare -a wl=("adjvector")
#declare -a bf=(1 2 3 4 5 6 7 8 9 10 12 14)
declare -a bf=(1)

declare -a mt=("nvram")

declare -r rl=(3)

for replicas in "${rl[@]}"
do
for w in "${wl[@]}"
do
  for m in "${mt[@]}"
  do
	for b in "${bf[@]}"
	do
    ./twitter_bench.py -c 
    ./twitter_bench.py -g -rep "$replicas" -m "$m" -w "$w" -b "$b"
    ./twitter_bench.py -dc -m "$m" -w "$w"
    ./twitter_bench.py -db -m "$m" -w "$w"

    ./twitter_bench.py -start -m "$m" -w "$w"
	# wait for sometime
	sleep 60
    ./twitter_bench.py -stop -m "$m" -w "$w"

	# gather output
    ./twitter_bench.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b"
	done
  done
done
done
exit
