#!/usr/bin/env bash
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
        if [ ! -f "$i/launch_servers" ] ; then
			clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/test
			clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/core
			clush -w ${ip} rm -rf ${deploy_dir}/cyclone.git/client_src.zip
            scp tmpdir/client_src.zip ${ip}:${deploy_dir}/cyclone.git/
			clush -w ${ip} ${deploy_dir}/${node}/build_client.sh
        fi
    fi
done


