#!/bin/bash

./echo.sh                  #commute disabled, batching enabled
./echo_nobatch.sh          #commute disabled, batching disabled
./echo_copy.sh             #commute disabled, batching enabled, extra copy
./echo_copy_nobatch.sh     #commute disabled, batching disabled, extra copy
./echo_commute_overhead.sh #commute enabled, batching enabled
