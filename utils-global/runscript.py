#!/usr/bin/env python

import argparse
import sys
import os
import shutil

DBG=1

__gen_dir= 'gen_configs'
__deploy_dir = '/home/pfernando/cyclone'
__home = '/home/pfernando/deploy-cyclone/utils-global'
__rte_sdk = '/home/pfernando/dpdk'
__rte_nvmsdk = '/home/pfernando/nvm-dpdk'


#config - single replica, 3 replicas etc
__one = '1'
__two = '2'
__three = '3'

#workloads
__echo = 'echo'
__pmemkv = 'pmemkv'

#memory types
__dram = 'dram'
__nvram = 'nvram'
__empty = 'empty'

rl=[]
rl.append(__one)
rl.append(__two)
rl.append(__three)

wl=[]
wl.append(__echo)
wl.append(__pmemkv)
wl.append(__empty)

ml=[]
ml.append(__dram)
ml.append(__nvram)

parser = argparse.ArgumentParser(prog="runscript", description="script to run cyclone for numbers")
parser.add_argument('-c', dest='clean', action='store_true', default=False, help="clean env after a run")
parser.add_argument('-g', dest='gen', action='store_true', default=False, help="generate config")
parser.add_argument('-collect', dest='collect', action='store_true', default=False, help="collect output")
parser.add_argument('-db', dest='deploy_bins', action='store_true', default=False, help="deploy client/server binaries")
parser.add_argument('-dc', dest='deploy_configs', action='store_true', default=False, help="deploy configs")
parser.add_argument('-start', dest='start', action='store_true', default=False, help="run experiment")
parser.add_argument('-stop', dest='stop', action='store_true', default=False, help="stop experiment")
parser.add_argument('-w', dest='workload', default=__empty , help='workload name, eg: echo, pmemkv', choices=wl)
parser.add_argument('-m', dest='memtype', default=__empty , help='memory type', choices=ml)
parser.add_argument('-b', dest='bufsize', default=__empty , help='inflight buffer size')
parser.add_argument('-rep', dest='replicas', default=__empty , help='number of replicas', choices=rl)

try:
    args = parser.parse_args()

except:
    sys.exit(0)

def dbg(s):
    if DBG==1:
        print s

def msg(s):
    print '\n' + '>>>' +s + '\n'

def cd(dirt):

    dbg(dirt)


    if dirt == __home:
        os.chdir(__home);
    else:

        path = __home + '/' + dirt
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

#utility script for running blizzard tests

def clean():
    print 'cleaning deployed servers/clients'


def generate(args):
    w = args.workload
    m = args.memtype
    b = args.bufsize
    r = args.replicas
    #first we remove the old __gen_dir if any
    try:
        shutil.rmtree(__gen_dir)
    except OSError:
        pass
    if r == __empty:
        print "Error generating config, specify number of replicas"
        return 1;

    print 'generating workload : ' + w +' for memory tytpe: ' + m + ' replica nodes: ' + r
    cmd = 'python config_generator.py ../utils-arch-cluster/cluster-dpdk.ini.' + r + ' ../utils-arch-cluster/example.ini.' + r + ' '
    cmd += './' + w + '.py ' + b + ' '+ __gen_dir

    sh(cmd)


#build the cyclone binaries and copy them over to participating machines
def deploy_bin(args):
    w = args.workload
    m = args.memtype
    print 'building cyclone binaries'
    
    cd('../core')
    cmd = 'make clean'
    sh(cmd)
    cmd = 'make'
    if m == __nvram:
        cmd += ' CPPFLAGS=' + '\"-DPMEM_HUGE\"'
    sh(cmd)
    cd(__home)

    cd('../test')
    cmd = 'make clean'
    sh(cmd)

    cmd = 'make server'
    if m == __dram:
        cmd += ' RTE_SSDK=' + __rte_sdk
    elif m == __nvram:
        cmd += ' RTE_SSDK=' + __rte_nvmsdk
    sh(cmd)
    cd(__home)

    cd('../core')
    cmd = 'make clean'
    sh(cmd)
    cmd = 'make'
    sh(cmd)
    cd(__home)

    cd('../test')
    cmd = 'make client'
    sh(cmd)
    cd(__home)

    #now copy the binaries over
    cmd ='./copy_binaries.sh '
    cmd += __gen_dir + ' ' + __deploy_dir + ' ' + w
    msg(cmd)
    sh(cmd)

# this should include both config copy and binary copy
def deploy_configs(args):
    cmd = './deploy_configs.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    msg(cmd)
    sh(cmd)

def stop_cyclone(args):
    print 'stop experiment'
    cmd = './deploy_shutdown.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)

def start_cyclone(args):
    #server deploy
    cmd = './deploy_services.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)

    #client deploy
    cmd = './deploy_clients.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)


def gather_output(args):

    w = args.workload
    m = args.memtype
    b = args.bufsize
    r = args.replicas
    if r == __empty:
        print "Error gathering output, specify replicas"
        return 1
    outdir = 'results/' + w + '/rep' + r  + '/' + m + '/' + b
    print 'collect output data and copying in to' + outdir
    cmd = './gather_output.sh '
    cmd += __gen_dir + ' ' + __deploy_dir + ' ' + outdir
    sh(cmd)
    #version the ouput and move it to results dir


if __name__ == '__main__':

    c = args.clean
    g = args.gen
    strt = args.start
    stop = args.stop
    db = args.deploy_bins
    dc = args.deploy_configs
    clct = args.collect

    if db == True:
        deploy_bin(args)
    if g == True:
        generate(args)
    if dc == True:
        deploy_configs(args)
    if strt == True:
        start_cyclone(args)
    if stop == True:    
        stop_cyclone(args)
    if clct == True:
        gather_output(args)
