
memcached=/usr/local/memcached/bin/memcached
#memaslap=/home/pradeep/installations/libmemcached-1.0.18/clients/memaslap
memaslap=/home/pfernando/libmemcached-1.0.18/clients/memaslap
result_dir=../../../utils-global/results/kvbench/memcached

#memcached params
memory=2024
port=11211
host=127.0.0.1

declare -a thr=(1 2 4 8)


#memaslap params
running_time=1m
cfg_file=memaslap.conf

for nth in  "${thr[@]}"
do
mkdir -p $result_dir/t"$nth"


numactl --cpubind=1 --membind=1 ${memcached} -m $memory  nobody -p $port -l $host 1 & > memcachedserver.log &
${memaslap} -s $host:$port -S 5s --time=$running_time --threads=$nth --cfg_cmd=$cfg_file > $result_dir/t"$nth"/memslap.log
pkill -f memcached
sleep 5;

done
