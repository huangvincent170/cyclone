#!/bin/bash
if [ $# -ne 3 ]
    then echo "Usage $0 config_dir deploy_dir out_dir"
    exit
fi
output_dir=$1
deploy_dir=$2
out_dir=$3
mkdir -p ${out_dir}
for i in ${output_dir}/* 
do
    if [ -d "$i" ] ; then
	node=$(basename $i)
	ip=`cat ${i}/ip_address`
	if [[ -f "$i/launch_clients" ]] && [[ ! -f "$i/launch_servers" ]] ; then
		echo "copy back logs"
		scp ${ip}:${deploy_dir}/${node}/client_log0  ${out_dir}/client_${ip}.log
	fi
#	if [ "${node}" = "cyclone_0" ]; then
#		echo "copy back server 0 logs"
#		scp ${ip}:${deploy_dir}/${node}/server_log  ${out_dir}/server_${ip}.log
#	fi
	#copy all server output
	if [[ -f "$i/launch_clients" ]] && [[ -f "$i/launch_servers" ]] ; then
        echo "copy back server logs"
        scp ${ip}:${deploy_dir}/${node}/server_log  ${out_dir}/server_${ip}.log
    fi
    fi
done


