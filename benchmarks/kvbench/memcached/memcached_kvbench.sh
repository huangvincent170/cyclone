
memcached=/usr/local/memcached/bin/memcached
memaslap=/home/pradeep/installations/libmemcached-1.0.18/clients/memaslap
result_dir=../../../utils-global/results/kvbench/memcached

#memcached params
memory=2024
port=11211
host=127.0.0.1

#memaslap params
running_time=1m
cfg_file=memaslap.conf


mkdir -p $result_dir/t1



$(memcached) -m $memory  nobody -p $port -l $host 1 & > memcachedserver.log &
$(memaslap) -s $host:$port -S 5s --time=$running_time --cfg_cmd=$cfg_file > $result_dir/t1/memslap_t1.log
pkill memcached
