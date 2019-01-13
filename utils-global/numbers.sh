#!/bin/bash
declare -a bf=()
#declare -a wl=("echo" "pmemkv")
declare -a wl=("echo" "pmemkv" "volatile_pmemkv" "pmemkv_ncc" "volatile_pmemkv_ncc")
declare -a bf_echo=(1 2 4 6 8 10 12 14 16 18 20 22 24 26 28 32 34 36)
declare -a bf_pmem=(1 2 3 4 5 6 7 8 9 10 12 14 16 18 20 22 24)
#declare -a bf_echo=(1 2)
#declare -a bf_pmem=(1)

declare -a mt=("dram" "nvram")
#declare -a mt=("dram")

#declare -r rl=(1 2 3)
declare -r rl=(3)

for replicas in "${rl[@]}"
do
for w in "${wl[@]}"
do
  if [ $w = "echo" ]; then
	bf=( "${bf_echo[@]}" )
  else
	bf=( "${bf_pmem[@]}" )
  fi
  for m in "${mt[@]}"
  do
	for b in "${bf[@]}"
	do
    ./runscript.py -c 
    ./runscript.py -g -rep "$replicas" -m "$m" -w "$w" -b "$b"
    ./runscript.py -dc -m "$m" -w "$w"
    ./runscript.py -db -m "$m" -w "$w"

    ./runscript.py -start -m "$m" -w "$w"
	# wait for sometime
	sleep 160
    ./runscript.py -stop -m "$m" -w "$w"

	# gather output
    ./runscript.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b"
	done
  done
done
done
exit
