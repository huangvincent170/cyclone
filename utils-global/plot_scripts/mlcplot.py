#!/usr/bin/env python

import numpy as np
import argparse
import sys
from common import *


from matplotlib.backends.backend_pdf import PdfPages
import matplotlib.pyplot as plt

DBG = 1

__cdram = '#d73027'
__cnvm =  '#fc8d59'


def parse_rw_lat(fpath, filter):
    tmpfile = 'dfile.tmp'
    cmd = 'sed -n \'' + filter + '\' ' + fpath + '| cut -f 3 >' + tmpfile
    sh(cmd)
    with open(tmpfile) as f:
        l = f.read().splitlines()
        l = [float(x) for x in l]
    os.remove(tmpfile)
    return l

def parse_lat_through(fpath, filter):
    tmpfile = 'dfile.tmp'
    cmd = 'sed -n \'' + filter + '\' ' + fpath + '| cut -f 2,3 >' + tmpfile
    sh(cmd)

    with open(tmpfile) as f:
        l = f.read().splitlines()
        tl_l = [x.split() for x in l]
    os.remove(tmpfile)

    return tl_l



def plot_one(ax, dram_l, nvm_l):
    line1 = ax.plot(tuple([float(x[1])/1000 for x in dram_l[0]]), tuple([x[0] for x in dram_l[0]]),linewidth=1, marker = 's',ms=3)
    line2 = ax.plot(tuple([float(x[1])/1000 for x in dram_l[1]]), tuple([x[0] for x in dram_l[1]]),linewidth=1, marker = 'o',ms=3)

    line3 = ax.plot(tuple([float(x[1])/1000 for x in nvm_l[0]]), tuple([x[0] for x in nvm_l[0]]),linewidth=1, marker = 's',ms=3)
    line4 = ax.plot(tuple([float(x[1])/1000 for x in nvm_l[1]]), tuple([x[0] for x in nvm_l[1]]),linewidth=1, marker = 'o',ms=3)

    ax.set_xlabel('$throughput \\times 10^3$',fontsize='6')
    ax.tick_params(axis='x',which='both',top='off',labelsize='6')
    ax.set_xlim(-4,)

    ax.set_ylabel('latency(nano-sec)', fontsize='6')
    ax.tick_params(axis='y',which='both',right='off',labelsize='6')

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)

    ax.legend((line1[0],line2[0],line3[0],line4[0]),
               ('dram-seq','dram-random','nvm-seq','nvm-random'),
               fontsize='5',ncol=2,loc = 'best',frameon=False)

def plot_two(ax, dram_l, nvm_l):

    xt_l = []
    xl_l = ['100% read', '100% write(NT)', '2Reads,\n 1 Write(NT)']

    print dram_l
    print nvm_l
    start = 1.0
    width = 0.2

    ig_gap = 4
    g_gap = 2.5

    ind = []
    cnt = start
    for x in range(0,6):
        ind.append(cnt)
        if x%2:
            xt_l.append(cnt - ((width*g_gap)/2) + .18)
            cnt += ig_gap*width
        else:
            cnt += g_gap*width
    print ind
    print xt_l
 #   ind = [start + 4*x*width for x in range(0,6)]
    rects1 = ax.bar(ind, tuple([x/1000 for x in dram_l]),width, linewidth=0.1, color = __cdram)
    patterns = ('','////','','////','','////')
    for bar, pattern in zip(rects1,patterns):
        bar.set_hatch(pattern)

#    start = start + width
    ind = [(x + width) for x in ind]
    rects2 = ax.bar(ind, tuple([x/1000 for x in nvm_l]),width, linewidth=0.1, color=__cnvm )
    patterns = ('','////','','////','','////')
    for bar, pattern in zip(rects2,patterns):
        bar.set_hatch(pattern)

    #ax.set_xlabel('throughput',fontsize='6')
    ax.tick_params(axis='x',which='both',top='off',labelsize='6')
    ax.set_xticks(xt_l)
    ax.set_xticklabels(xl_l)
    ax.set_xlim(.8,5)

    ax.set_ylabel('$throughput \\times 10^3 $', fontsize='6')
    ax.tick_params(axis='y',which='both',right='off',labelsize='6')

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)

    ax.legend((rects1[0],rects1[1],rects2[0],rects2[1]),
               ('dram-seq','dram-random','nvm-seq','nvm-random'),
               fontsize='5',ncol=2,loc = 'best',frameon=False)


if __name__ == '__main__':

    pp = PdfPages('mlc.pdf')
    fig, ax = plt.subplots(nrows=2, ncols=1,figsize=(3.5,2.5))

    #parse the files for latency/throughput

    dram_l = []
    dram_l.append(parse_lat_through('mlc_data/DRAM_aep03.out', '31,50p;51q'))
    dram_l.append(parse_lat_through('mlc_data/DRAM_aep03.out', '53,72p;73q'))

    nvm_l = []
    nvm_l.append(parse_lat_through('mlc_data/PMEM_aep003.out','55,74p;75q'))
    nvm_l.append(parse_lat_through('mlc_data/PMEM_aep003.out','77,96p;97q'))
    plot_one(ax[0], dram_l,nvm_l)


    dram_l = parse_rw_lat('mlc_data/DRAM_aep03.out','18,23p;24q')
    nvm_l = parse_rw_lat('mlc_data/PMEM_aep003.out','42,47p;48q')
    plot_two(ax[1], dram_l,nvm_l)

    plt.tight_layout()
    pp.savefig(figure=fig)
    pp.close()








