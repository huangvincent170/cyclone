
running_time=1m
cfg_file=memaslap.conf

/home/pradeep/installations/libmemcached-1.0.18/clients/memaslap -s 127.0.0.1:11211 -S 5s --time=$running_time --cfg_cmd=$cfg_file
