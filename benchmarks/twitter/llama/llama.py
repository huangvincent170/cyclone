def launch_cmds_startup():
    print("Configuring for llama graph application")


def launch_cmds_server_gen(f, q, r, m, quorums, replicas, clients, ports):
    passwd = 'dummy'
    if os.environ.has_key('CYCLONE_PASS'):
        passwd=os.environ.get('CYCLONE_PASS')
    cmd='echo ' + passwd + ' | sudo -S '
    cmd= cmd + 'rm -rf /mnt/pmem0/llama_db\n'
    cmd= cmd + 'echo ' + passwd + ' | sudo -S '
    cmd= cmd + 'mkdir -p /mnt/pmem0/llama_db\n'
    f.write(cmd)
    cmd=''
    cmd=cmd + 'echo ' + passwd + ' | sudo -S '
    cmd=cmd + ' PMEM_IS_PMEM_FORCE=1 '
    cmd=cmd + ' LD_LIBRARY_PATH=/usr/lib:/usr/local/lib '
    cmd=cmd + '/home/pfernando/cyclone/cyclone.git/benchmarks/twitter/llama/llama_server '
    cmd=cmd + str(r) + ' '
    cmd=cmd + str(m) + ' '
    cmd=cmd + str(clients) + ' '
    cmd=cmd + 'config_cluster.ini config_quorum.ini ' +str(ports) + ' &> server_log &\n'
    f.write(cmd)

def launch_cmds_preload_gen(f, m, c, quorums, replicas, clients, machines, ports):
    cmd=''


def launch_cmds_client_gen(f, m, c, quorums, replicas, clients, machines, ports,bufsize):
    passwd = 'dummy'
    if m >= replicas:
        client_machines=machines-replicas
        if client_machines > clients:
            client_machines = clients
        clients_per_machine=clients/client_machines
        c_start = clients_per_machine*(m - replicas)
        c_stop  = c_start + clients_per_machine
        if m == replicas + client_machines - 1:
            c_stop = clients
        if c == 0 and m < replicas + client_machines:
            cmd=''
            if os.environ.has_key('CYCLONE_PASS'):
                passwd=os.environ.get('CYCLONE_PASS')    
            cmd=cmd + ' echo ' + passwd + ' | sudo -S '
            cmd=cmd + ' LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:/usr/local/lib64 '    
            cmd=cmd + '/home/pfernando/cyclone/cyclone.git/benchmarks/twitter/llama/llama_async_client '
            cmd=cmd + str(c_start) + ' '
            cmd=cmd + str(c_stop) + ' '
            cmd=cmd + str(m) + ' '
            cmd=cmd + str(replicas) + ' '
            cmd=cmd + str(clients) + ' '
            cmd=cmd + str(quorums) + ' '
            cmd=cmd + 'config_cluster.ini config_quorum ' + str(ports) + ' ' + str(bufsize)  +' &> client_log' + str(0) + '&\n'
            f.write(cmd)
        
def killall_cmds_gen(f):
    passwd = 'dummy'
    if os.environ.has_key('CYCLONE_PASS'):
        passwd=os.environ.get('CYCLONE_PASS')
    f.write('echo ' + passwd + ' | sudo -S pkill -f llama_server\n')
    f.write('echo ' + passwd + ' | sudo -S pkill -f llama_async\n')
