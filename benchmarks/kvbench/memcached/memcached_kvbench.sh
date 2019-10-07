
memcached=/usr/local/memcached/bin/memcached
#memaslap=/home/pradeep/installations/libmemcached-1.0.18/clients/memaslap
memaslap=/home/pfernando/libmemcached-1.0.18/clients/memaslap
result_dir=../../../utils-global/results/kvbench/memcached

#memcached params
memory=10240
port=11211
threads=7
host=localhost

declare -a thr=(1 2 4 6 8 8 8 8 8)
declare -a conc=(1 2 4 6 8 16 32 48 64)
index=0

#memaslap params
running_time=1m
cfg_file=memaslap.conf

for nth in  "${thr[@]}"
do
mkdir -p $result_dir/t"$nth"

echo "threads:" $nth "concurrency:" ${conc[$index]}
numactl --cpubind=1 --membind=1 ${memcached} -m $memory -t $threads  nobody -p $port -l $host 1 & > memcachedserver.log &
numactl --cpubind=0 --membind=0 ${memaslap} -s $host:$port -S 5s --time=$running_time --threads=$nth --concurrency=${conc[$index]} --binary --cfg_cmd=$cfg_file > $result_dir/t"$nth"/memslap"${conc[$index]}".log
pkill -f memcached
sleep 5;
((index++))
done
