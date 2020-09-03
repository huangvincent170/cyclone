#!/bin/bash
if [ $# -ne 3 ]
    then echo "Usage $0 config_dir deploy_dir binary_prefix"
    exit
fi
output_dir=$1
deploy_dir=$2
#same as bench_subname
binary_prefix=$3
echo "config dir = $output_dir"
echo "deploy dir = $deploy_dir"


for i in ${output_dir}/* 
do
    if [ -d "$i" ] ; then
	node=$(basename $i)
	ip=`cat ${i}/ip_address`
	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.tcp/test/${binary_prefix}_server
	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.tcp/test/${binary_prefix}_async_client
	#clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/test/${binary_prefix}_async_client
	scp ../test/${binary_prefix}_server ${ip}:${deploy_dir}/cyclone.tcp/test/
	scp ../test/${binary_prefix}_async_client ${ip}:${deploy_dir}/cyclone.tcp/test/
    fi
done


