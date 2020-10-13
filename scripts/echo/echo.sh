#!/bin/bash
declare -a wl=("echo")
#declare -a bf=(1 2 4 6 8 10 12 16 20)
declare -a bf=(1)

declare -a mt=("nvram")
# declare -a mt=("dram")

#declare -r rl=(1 2 3)
declare -r rl=(1)

for replicas in "${rl[@]}"
do
for w in "${wl[@]}"
do
  for m in "${mt[@]}"
  do  
    for b in "${bf[@]}"
    do  
    ./echo_bench.py -c  
    ./echo_bench.py -g -rep "$replicas" -m "$m" -w "$w" -b "$b"
    ./echo_bench.py -dc -m "$m" -w "$w"
    ./echo_bench.py -db -m "$m" -w "$w"

   ./echo_bench.py -startsrv -m "$m" -w "$w"
   ./echo_bench.py -startclnt -m "$m" -w "$w"
    # wait for sometime
   sleep 60
   ./echo_bench.py -stop -m "$m" -w "$w"

    # gather output
   ./echo_bench.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b"
    done
  done
done
done
exit
