def launch_cmds_startup():
    print("Configuring for echo application")

def launch_cmds_server_gen(f, q, r, m, quorums, replicas, clients, ports):
    cmd=''
    passwd='' 
    if os.environ.has_key('RBT_SLEEP_USEC'):
        cmd=cmd + 'RBT_SLEEP_USEC=' + os.environ.get('RBT_SLEEP_USEC') + ' ' 
    if os.environ.has_key('CYCLONE_PASS'):
        passwd=os.environ.get('CYCLONE_PASS')
    cmd=cmd + 'sudo -S '
    cmd=cmd + ' PMEM_IS_PMEM_FORCE=1 '
    cmd=cmd + ' LD_LIBRARY_PATH=/usr/local/lib64:/usr/lib:/usr/local/lib '
    cmd=cmd + '/home/dzahka3/cyclone/cyclone.tcp/test/echo_server '
    cmd=cmd + str(r) + ' ' 
    cmd=cmd + str(m) + ' ' 
    cmd=cmd + str(clients) + ' ' 
    #cmd=cmd + 'config_cluster.ini config_quorum.ini ' +str(ports) + ' &> server_log &\n'
    cmd=cmd + 'config_cluster.ini config_quorum.ini ' +str(ports)
    f.write(cmd)


def launch_cmds_preload_gen(f, m, c, quorums, replicas, clients, machines, ports):
    cmd=''


def launch_cmds_client_gen(f, m, c, quorums, replicas, clients, machines, ports):
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
            if os.environ.has_key('PAYLOAD'):
                cmd=cmd + 'PAYLOAD=' + os.environ.get('PAYLOAD') + ' '
            if os.environ.has_key('CC_TX'):
                cmd=cmd + 'CC_TX=' + os.environ.get('CC_TX') + ' '    
            if os.environ.has_key('QUORUMS_ACTIVE'):
                cmd=cmd + 'QUORUMS_ACTIVE=' + os.environ.get('QUORUMS_ACTIVE') + ' '    
            if os.environ.has_key('CYCLONE_PASS'):
                passwd=os.environ.get('CYCLONE_PASS')
            cmd=cmd + ' sudo -S '
            cmd=cmd + ' LD_LIBRARY_PATH=/usr/local/lib64:/usr/lib:/usr/local/lib '
            cmd=cmd + '/home/dzahka3/cyclone/cyclone.tcp/test/echo_async_client '
            cmd=cmd + str(c_start) + ' '
            cmd=cmd + str(c_stop) + ' '
            cmd=cmd + str(m) + ' '
            cmd=cmd + str(replicas) + ' '
            cmd=cmd + str(clients) + ' '
            cmd=cmd + str(quorums) + ' '
            #cmd=cmd + 'config_cluster.ini config_quorum ' + str(ports) + ' &> client_log' + str(0) + '&\n'
            cmd=cmd + 'config_cluster.ini config_quorum 3864 ' + str(ports)
            f.write(cmd)
       
def killall_cmds_gen(f):
    passwd=''
    if os.environ.has_key('CYCLONE_PASS'):
        passwd=os.environ.get('CYCLONE_PASS')
    f.write('echo ' + passwd + ' | sudo -S pkill echo_server\n')
    f.write('echo ' + passwd + ' | sudo -S pkill echo_client\n')
    f.write('echo ' + passwd + ' | sudo -S pkill echo_async\n')

