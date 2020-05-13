#!/bin/bash

bench_res=allresults
mkdir  ${bench_res}

./echo.sh
mv results/echo ${bench_res}/non-commute

./echo_commute.sh
mv results/echo ${bench_res}/commute

./echo_nobatch.sh
mv results/echo ${bench_res}/no-batching
