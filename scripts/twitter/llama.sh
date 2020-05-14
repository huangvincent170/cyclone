#!/bin/bash
declare -a wl=("llama_mem" "llama_persist")
declare -a bf=(1 2 3)

declare -a mt=("nvram")

declare -r rl=(1)

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

    ./twitter_bench.py -startsrv -m "$m" -w "$w"
    ./twitter_bench.py -startclnt -m "$m" -w "$w"
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
