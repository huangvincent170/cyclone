#!/usr/bin/env python
import argparse
from commonbench import *

#workloads
adjvector = 'adjvector'

wl=[]
wl.append(adjvector)

wl.append(empty)


parser = argparse.ArgumentParser(prog="runscript", description="script to run cyclone for numbers")
parser.add_argument('-c', dest='clean', action='store_true', default=False, help="clean env after a run")
parser.add_argument('-g', dest='gen', action='store_true', default=False, help="generate config")
parser.add_argument('-collect', dest='collect', action='store_true', default=False, help="collect output")
parser.add_argument('-db', dest='deploy_bins', action='store_true', default=False, help="deploy client/server binaries")
parser.add_argument('-dc', dest='deploy_configs', action='store_true', default=False, help="deploy configs")
parser.add_argument('-start', dest='start', action='store_true', default=False, help="run experiment")
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

class Vote(Common):
    #map some workload names to binary name
    def wl2binary(self,arg):
        switcher= {
            'adjvector_ncc' : 'adjvector',
        }
        return switcher.get(arg,arg)

    def bench(self):
        return 'twitter';

    def get_bench_dir(self):
        return 'ERROR'

    def get_server_cxx(self,wload):
        return '' #else

if __name__ == '__main__':

    c = args.clean
    g = args.gen
    strt = args.start
    stop = args.stop
    db = args.deploy_bins
    dc = args.deploy_configs
    clct = args.collect

    vt = Vote();


    if args.clean is True:
        vt.clean(args)
    if db == True:
        vt.deploy_bin(args)
    if g == True:
        vt.generate(args)
    if dc == True:
        vt.deploy_configs(args)
    if strt == True:
        vt.start_cyclone(args)
    if stop == True:
        vt.stop_cyclone(args)
    if clct == True:
        vt.gather_output(args)
