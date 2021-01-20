#!/bin/bash
declare -a wl=("hashmap")
declare -a bf=(1)
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
    ./kv_bench.py -db -m "$m" -w "$w" -commute
    #./kv_bench.py -db -m "$m" -w "$w"

        done
  done
done
done
exit
