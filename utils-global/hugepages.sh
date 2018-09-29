#!/bin/bash
umount /mnt/huge
mkdir -p /mnt/huge
mount -t hugetlbfs nodev /mnt/huge
echo 2048 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages

./usertools/dpdk-devbind.py --status
sudo modprobe uio_pci_generic
ip addr flush dev eth0
sudo ./dpdk-devbind.py --bind=uio_pci_generic 
