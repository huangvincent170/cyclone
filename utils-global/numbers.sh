#!/bin/bash
declare -a wl=("echo")
declare -a mt=("dram")
declare -a bf=(0 1 2)

for w in "${wl[@]}"
do


  for m in "${mt[@]}"
  do

	for b in "${bf[@]}"
	do
    ./runscript.py -c 
    ./runscript.py -g -m "$m" -w "$w" -b "$b"
    ./runscript.py -dc -m "$m" -w "$w"
    ./runscript.py -db -m "$m" -w "$w"

    ./runscript.py -start -m "$m" -w "$w"
	# wait for sometime
	sleep 30
    ./runscript.py -stop -m "$m" -w "$w"

	# gather output
    ./runscript.py -collect -m "$m" -w "$w" -b "$b"
	done
  done
done
exit
