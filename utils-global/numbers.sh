#!/bin/bash
declare -a wl=("echo")
#declare -a mt=("dram")
#declare -a bf=(0 1 2 3 4)
#declare -a bf=(4 5 6)
#declare -a mt=("nvram" "dram")
declare -a mt=("nvram")
declare -a bf=(0)
declare -r rl=(1)
for replicas in "${rl[@]}"
do
for w in "${wl[@]}"
do
  for m in "${mt[@]}"
  do
	for b in "${bf[@]}"
	do
    ./runscript.py -c 
    ./runscript.py -g -rep "$replicas" -m "$m" -w "$w" -b "$b"
    ./runscript.py -dc -m "$m" -w "$w"
    ./runscript.py -db -m "$m" -w "$w"

    #./runscript.py -start -m "$m" -w "$w"
	# wait for sometime
	#sleep 120
    #./runscript.py -stop -m "$m" -w "$w"

	# gather output
    #./runscript.py -collect -rep "$replicas" -m "$m" -w "$w" -b "$b"
	done
  done
done
done
exit
