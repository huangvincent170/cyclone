#!/usr/bin/env python
import sys
import argparse
sys.path.append('../common')
from commonbench import *

#workloads
adjvector = 'adjvector'
llama_mem = 'llama_mem'
llama_persist = 'llama_persist'

wl=[]
wl.append(adjvector)
wl.append(llama_mem)
wl.append(llama_persist)

wl.append(empty)


parser = argparse.ArgumentParser(prog="runscript", description="script to run cyclone for numbers")
parser.add_argument('-c', dest='clean', action='store_true', default=False, help="clean env after a run")
parser.add_argument('-g', dest='gen', action='store_true', default=False, help="generate config")
parser.add_argument('-collect', dest='collect', action='store_true', default=False, help="collect output")
parser.add_argument('-db', dest='deploy_bins', action='store_true', default=False, help="deploy client/server binaries")
parser.add_argument('-dc', dest='deploy_configs', action='store_true', default=False, help="deploy configs")
parser.add_argument('-startsrv', dest='startsrv', action='store_true', default=False, help="run experiment")
parser.add_argument('-startclnt', dest='startclnt', action='store_true', default=False, help="run experiment")
parser.add_argument('-stop', dest='stop', action='store_true', default=False, help="stop experiment")
parser.add_argument('-w', dest='workload', default=empty , help='workload name, eg: echo, pmemkv', choices=wl)
parser.add_argument('-m', dest='memtype', default=empty , help='memory type', choices=ml)
parser.add_argument('-b', dest='bufsize', default=empty , help='inflight buffer size')
parser.add_argument('-rep', dest='replicas', default=empty , help='number of replicas', choices=rl)
parser.add_argument('-commute', dest='is_commute', action='store_true', default=False , help='concurrent execution')

try:
    args = parser.parse_args()

except:
    dbg('Error parsing input')
    sys.exit(0)

class Twitter(Common):
    #map some workload names to binary name
    def wl2binary(self,wload):
        switcher= {
            'adjvector_ncc' : 'adjvector',
            'llama_mem' :'llama',
            'llama_persist' :'llama'
        }
        return switcher.get(wload,wload) # second arg is the default return

    def bench(self):
        return 'twitter';

    def get_bench_dir(self):
        return 'ERROR'

    def get_server_cxx(self,wload):
        if wload == llama_mem:
            return '\"-DLL_MEMORY_ONLY\"'
        elif wload == llama_persist:
            return '\"-DLL_PERSISTENCE\"'
        return '' #else

if __name__ == '__main__':

    c = args.clean
    g = args.gen
    strt_serv = args.startsrv
    strt_clnt = args.startclnt
    stop = args.stop
    db = args.deploy_bins
    dc = args.deploy_configs
    clct = args.collect

    tw = Twitter();


    if args.clean is True:
        tw.clean(args)
    if db == True:
        tw.deploy_bin(args)
    if g == True:
        tw.generate(args)
    if dc == True:
        tw.deploy_configs(args)
    if strt_serv == True:
        tw.start_cyclone_server(args)
    if strt_clnt == True:
        tw.start_cyclone_client(args)
    if stop == True:
        tw.stop_cyclone(args)
    if clct == True:
        tw.gather_output(args)
