#!/usr/bin/env python

import sys
import os
import shutil

DBG=1


gen_dir= 'gen_configs'
deploy_dir = '/home/pfernando/cyclone'
home = '/home/pfernando/deploy-cyclone/utils-global'
rte_sdk = '/home/pfernando/dpdk'
rte_nvmsdk = '/home/pfernando/nvm-dpdk'
#pmemkv lib with crash-consistent logging turned-off
ncc_pmem = '/home/pfernando/pmdknlog/src/nondebug'





#config - single replica, 3 replicas etc
one = '1'
two = '2'
three = '3'


#memory types
dram = 'dram'
nvram = 'nvram'
empty = 'empty'

rl=[]
rl.append(one)
rl.append(two)
rl.append(three)


ml=[]
ml.append(dram)
ml.append(nvram)




def dbg(s):
    if DBG==1:
        print s

def msg(s):
    print '\n' + '>>>' +s + '\n'


def cd(dirt):

    dbg(dirt)


    if dirt == home:
        os.chdir(home);
    else:

        path = home + '/' + dirt
        dbg(path)

        try:
            os.chdir(path)
        except:
            print 'invalid directory ', path
            sys.exit(0)

def sh(cmd):

    msg(cmd)
    try:
        os.system(cmd)
    except:
        print 'invalid cmd', cmd
        sys.exit(0)

def stat(f):

    dbg(os.getcwd() + '/' + f)
    try:
        os.stat(f)
    except:
        dbg('invalid file' + os.getcwd() + '/' + f)
        return 1

    return 0

class Common:

    def wl2binary(self,arg):
        raise NotImplementedError("Please Implement this method")

    def bench(self):
        raise NotImplementedError("Please Implement this method")
 
    def get_bench_dir(self):
        raise NotImplementedError("Please Implement this method")

    def get_server_cxx(self,wload):
        raise NotImplementedError("Please Implement this method")

    def clean(self,args):
        print 'cleaning deployed servers/clients'
       
    def generate(self,args):
        w = args.workload
        m = args.memtype
        b = args.bufsize
        r = args.replicas
        #first we remove the old gen_dir if any
        try:
            shutil.rmtree(gen_dir)
        except OSError:
            pass
        if r == empty:
            print "Error generating config, specify number of replicas"
            return 1;

        print 'generating workload : ' + w +' for memory tytpe: ' + m + ' replica nodes: ' + r
        cmd = 'python config_generator.py ../utils-arch-cluster/cluster-dpdk.ini.' + r + ' ../utils-arch-cluster/example.ini.' + r + ' '
        cmd += '../benchmarks/' + self.bench() + '/' + self.wl2binary(w) + '/'+ self.wl2binary(w) + '.py ' + b + ' '+ gen_dir

        sh(cmd)


    #build the cyclone binaries and copy them over to participating machines
    def deploy_server_bin(self,args):
        w = args.workload
        m = args.memtype
        print 'building cyclone binaries'

        cd('../core')
        cmd = 'make clean'
        sh(cmd)
        cmd = 'make'
        if m == nvram:
            cmd += ' CPPFLAGS=' + '\"-DPMEM_HUGE\"'
        sh(cmd)
        cd(home)
        
        bench_dir = '../benchmarks/' + self.bench() + '/' + self.wl2binary(w) 
        cd(bench_dir)
        cmd = 'make clean'
        sh(cmd)

        cmd = 'make server'
        if m == dram:
            cmd += ' RTE_SSDK=' + rte_sdk
        elif m == nvram:
            cmd += ' RTE_SSDK=' + rte_nvmsdk
        cmd +=  ' ' + self.get_server_cxx(w)
        
        sh(cmd)
        cd(home)

        #now copy the binaries over
        cmd ='./copy_binaries.sh '
        cmd += gen_dir + ' ' + deploy_dir + ' ' + self.bench() + ' ' + self.wl2binary(w)
        msg(cmd)
        sh(cmd)
        cd(home)

    # our client machines are different from servers. we are
    # shipping source
    def deploy_client_bin(self,args):
        w = args.workload
        tmpdir = 'tmpdir'
        #compress core and test directories
        try:
            shutil.rmtree(tmpdir)
        except OSError:
            pass
        if not os.path.exists(tmpdir):
            os.makedirs(tmpdir)

        cd('..')
        cmd = 'zip -rq '+ home + '/tmpdir/client_src.zip '
        cmd = cmd + 'core ' + 'benchmarks/' + self.bench() + '/' + self.wl2binary(w)
        sh(cmd)
        cd(home)

        #ship compressed files
        cmd ='./copy_client_src.sh '
        cmd += gen_dir + ' ' + self.bench() + ' ' + self.wl2binary(w) + ' ' + deploy_dir
        sh(cmd)
        cd(home)

    def deploy_bin(self,args):
        self.deploy_server_bin(args)
        self.deploy_client_bin(args)

    # this should include both config copy and binary copy
    def deploy_configs(self,args):
        w = args.workload
        cmd = './deploy_configs.sh '
        cmd += gen_dir + ' ' + deploy_dir + ' ' + self.bench() + ' ' + self.wl2binary(w)
        msg(cmd)
        sh(cmd)

    def stop_cyclone(self,args):
        print 'stop experiment'
        cmd = './deploy_shutdown.sh '
        cmd += gen_dir + ' ' + deploy_dir
        sh(cmd)

    def start_cyclone(self,args):
        #server deploy
        cmd = './deploy_services.sh '
        cmd += gen_dir + ' ' + deploy_dir
        sh(cmd)

        #client deploy
        cmd = './deploy_clients.sh '
        cmd += gen_dir + ' ' + deploy_dir
        sh(cmd)


    def gather_output(self,args):

        w = args.workload
        m = args.memtype
        b = args.bufsize
        r = args.replicas
        if r == empty:
            print "Error gathering output, specify replicas"
            return 1
        outdir = 'results/'+ self.bench() + '/' + w + '/rep' + r  + '/' + m + '/' + b
        print 'collect output data and copying in to' + outdir
        cmd = './gather_output.sh '
        cmd += gen_dir + ' ' + deploy_dir + ' ' + outdir
        sh(cmd)
        #version the ouput and move it to results dir



