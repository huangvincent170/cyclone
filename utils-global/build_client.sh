#!/bin/bash
pushd  /home/pfernando/cyclone/cyclone.git
unzip -q client_src.zip
cd core;make clean;make
cd ..
cd test;make clean;make client
popd
