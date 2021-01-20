#!/bin/bash

# prepare network interface for DPDK

dpdk_home=/home/cyclone/dpdk-nvm
pciaddr=0000:18:00.0 
deviface=ens785f0
module_path=/home/cyclone/dpdk/x86_64-native-linuxapp-gcc/kmod/igb_uio.ko
bind_module=igb_uio

modprobe uio
insmod ${module_path}

${dpdk_home}/tools/dpdk-devbind.py --status

ip addr flush dev ${deviface}

${dpdk_home}/tools/dpdk-devbind.py --bind=${bind_module}  ${pciaddr}

#reserve huge pages
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge

echo 4096 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
${dpdk_home}/tools/dpdk-devbind.py --status
 cat /proc/meminfo | grep Hugepages
