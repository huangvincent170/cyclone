#!/bin/bash

# prepare network interface for DPDK

dpdk_home=/home/pfernando/dpdk
pciaddr=0000:18:00.0 
deviface=enp24s0f0

modprobe uio_pci_generic
${dpdk_home}/tools/dpdk-devbind.py --status

ip addr flush dev ${deviface}

${dpdk_home}/tools/dpdk-devbind.py --bind=uio_pci_generic  ${pciaddr}

#reserver huge pages
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge

echo 4096 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
${dpdk_home}/tools/dpdk-devbind.py --status
cat /proc/meminfo | grep Hugepages
