#!/bin/bash
declare -a wl=("adjvector")
declare -a bf=(2 4 10 12 14 15 16) 

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
    ./twitter_bench.py -db -m "$m" -w "$w" -commute
    
    #precautionary cleanup
    ./twitter_bench.py -stop -m "$m" -w "$w"
    ./twitter_bench.py -startsrv -m "$m" -w "$w"
    ./twitter_bench.py -startclnt -m "$m" -w "$w"
    # wait for sometime
    sleep 60
    ./twitter_bench.py -stop -m "$m" -w "$w"

    # gather output
    ./twitter_bench.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b" -commute
    done
  done
done
done
exit
