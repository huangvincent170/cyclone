#!/bin/bash
if [ $# -ne 2 ]
    then echo "Usage $0 config_dir deploy_dir"
    exit
fi
output_dir=$1
deploy_dir=$2
echo "config dir = $output_dir"
echo "deploy dir = $deploy_dir"


for i in ${output_dir}/* 
do
    if [ -d "$i" ] ; then
	node=$(basename $i)
	ip=`cat ${i}/ip_address`
	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/test/echo_server
	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/test/echo_client
	if [ -f "$i/launch_servers" ] ; then
	    scp ../test/echo_server ${ip}:${deploy_dir}/cyclone.git/test/
	else
	    scp ../test/echo_client ${ip}:${deploy_dir}/cyclone.git/test/
	fi
    fi
done


