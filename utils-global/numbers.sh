#!/bin/bash

export CYCLONE_PASS="danielpassword"

rm -rf test
# python config_generator.py ../utils-arch-cluster/jedi-machines.ini.2 ../utils-arch-cluster/jedi-example.ini.2 ./echo.py test
python config_generator.py ../utils-arch-cluster/jedi-machines.ini.1 ../utils-arch-cluster/jedi-example.ini.1 ./echo.py test
#python config_generator.py ../utils-arch-cluster/cluster-dpdk.ini.1 ../utils-arch-cluster/example.ini.1 ./echo.py test
#python config_generator.py ../utils-arch-cluster/aep-machines.ini.2 ../utils-arch-cluster/aep-example.ini.2 ./echo.py test

./deploy_configs.sh test /home/dzahka3/cyclone
./copy_binaries.sh test /home/dzahka3/cyclone echo
#./deploy_services.sh test /home/pfernando/cyclone

#./deploy_clients.sh test /home/pfernando/cyclone
