#!/usr/bin/env python

import argparse
import sys
import os
import shutil
from const import *

DBG=1

__gen_dir= 'gen_configs'
__deploy_dir = '/home/pfernando/cyclone'


#workloads
__echo = 'echo'
__pmemkv = 'pmemkv'


wl=[]
wl.append(__echo)
wl.append(__pmemkv)



parser = argparse.ArgumentParser(prog="runscript", description="script to run cyclone for numbers")
parser.add_argument('-g', dest='gen', action='store_true', default=False, help="generate config")
parser.add_argument('-b', dest='build', action='store_true', default=False, help="build cyclone client/server binaries")
parser.add_argument('-d', dest='deploy', action='store_true', default=False, help="deploy configs")
parser.add_argument('-c', dest='build', action='store_true', default=False, help="clean env after a run")
parser.add_argument('-run', dest='build', action='store_true', default=False, help="run experiment")
parser.add_argument('-stop', dest='build', action='store_true', default=False, help="stop experiment")
parser.add_argument('-w', dest='workload', default=__empty , help='workload name, eg: echo, pmemkv', choices=wl)

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

    #first we remove the old __gen_dir if any
    try:
        shutil.rmtree(__gen_dir)
    except OSError:
        pass

    print 'generating workload : ' + w +'for memory tytpe: ' + m
    cmd = 'python config_generator.py ../utils-arch-cluster/cluster-dpdk.ini ../utils-arch-cluster/example.ini '
    cmd += './' + w + '.py ' + __gen_dir

    sh(cmd)


#build the cyclone binaries and copy them over to participating machines
def deploy_bin():
    print 'building cyclone binaries'

# this should include both config copy and binary copy
def deploy_configs():
    cmd = './deploy_configs.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    msg(cmd)
    sh(cmd)


def stop_exp():
    print 'stop experiment'
    cmd = './deploy_shutdown.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)

def run_exp():
    #server deploy
    cmd = './deploy_services.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)

    #client deploy
    cmd = './deploy_clients.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)


def gather_output():
    print 'collect output data'
    cmd = './gather_output.sh '
    cmd += __gen_dir + ' ' + __deploy_dir
    sh(cmd)
    #version the ouput and move it to results dir


