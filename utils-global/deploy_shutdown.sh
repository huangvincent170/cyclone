#!/bin/bash
if [ $# -ne 2 ]
    then echo "Usage $0 config_dir deploy_dir"
    exit
fi
source ./common.sh
passwd=$(get_passwd)
if [ $passwd = "default_pass" ] ; then
   echo "Please set the cyclone passwd, using $passwd .."
fi
output_dir=$1
deploy_dir=$2
for i in ${output_dir}/*
do
    if [ -d "$i" ] ; then
	node=$(basename $i)
	ip=`cat ${i}/ip_address`
	echo "Executing shutdown on node $node"
	clush -w ${ip} ${deploy_dir}/${node}/killall.sh
	clush -w ${ip} "echo ${passwd} | sudo -S rm -f /mnt/huge/*"

    fi
done




