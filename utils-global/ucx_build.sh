#!/bin/bash -e

cd ../core
# make clean
make
cd ../test
# make clean
make
cd ../utils-global
rm -rf ~/cyclone/
mkdir -p ~/cyclone/cyclone.tcp/test/
./numbers.sh
