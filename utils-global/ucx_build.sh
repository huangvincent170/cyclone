#!/bin/bash -e

export LD_LIBRARY_PATH="~/ucx-1.10.0/install/lib"
cd ../core
make
cd ../utils-global
rm -rf ~/cyclone/
mkdir -p ~/cyclone/cyclone.tcp/test/
./numbers.sh
