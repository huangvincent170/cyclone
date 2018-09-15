#!/bin/bash
if [ $# -ne 2 ]
    then echo "Usage $0 config_dir deploy_dir"
    exit
fi
output_dir=$1
deploy_dir=$2
mkdir -p results
for i in ${output_dir}/* 
do
    if [ -d "$i" ] ; then
	node=$(basename $i)
	ip=`cat ${i}/ip_address`
	if [[ -f "$i/launch_clients" ]] && [[ ! -f "$i/launch_servers" ]] ; then
		echo "copy back logs"
		scp ${ip}:${deploy_dir}/${node}/client_log0  results/client_${ip}.log
	fi
    fi
done


