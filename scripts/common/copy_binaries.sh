#!/bin/bash
if [ $# -ne 4 ]
    then echo "Usage $0 config_dir deploy_dir bench_name binary_prefix"
    exit
fi
output_dir=$1
deploy_dir=$2
bench_name=$3
#same as bench_subname
binary_prefix=$4
echo "config dir = $output_dir"
echo "deploy dir = $deploy_dir"


for i in ${output_dir}/* 
do
    if [ -d "$i" ] ; then
	node=$(basename $i)
	ip=`cat ${i}/ip_address`
	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/benchmarks/${bench_name}/${binary_prefix}_server
#	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/test/${binary_prefix}_client
#	clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/test/${binary_prefix}_async_client
		if [ -f "$i/launch_servers" ] ; then
			scp benchmarks/${bench_name}/${binary_prefix}/${binary_prefix}_server ${ip}:${deploy_dir}/cyclone.git/benchmarks/${bench_name}/${binary_prefix}
		fi
    fi
done


