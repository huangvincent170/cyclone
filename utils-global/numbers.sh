#!/bin/bash

rm -rf test
#python config_generator.py ../utils-arch-cluster/aep-machines.ini.1 ../utils-arch-cluster/aep-example.ini.1 ./echo.py test
#python config_generator.py ../utils-arch-cluster/cluster-dpdk.ini.1 ../utils-arch-cluster/example.ini.1 ./echo.py test
python config_generator.py ../utils-arch-cluster/aep-machines.ini.2 ../utils-arch-cluster/aep-example.ini.2 ./echo.py test

./deploy_configs.sh test /home/pfernando/cyclone
./copy_binaries.sh test /home/pfernando/cyclone echo
#./deploy_services.sh test /home/pfernando/cyclone

#./deploy_clients.sh test /home/pfernando/cyclone
