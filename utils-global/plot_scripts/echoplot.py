#!/usr/bin/env python

import numpy as np
import argparse
import sys
from common import *


from matplotlib.backends.backend_pdf import PdfPages
import matplotlib.pyplot as plt

DBG = 1
color = ['#d73027', '#fc8d59']

result_home = '../results'

__dram = 'dram'
__nvram = 'nvram'

__echo = 'echo'

wrkld_l = []
wrkld_l.append(__echo)


#test strategies
tstone_l = []
tstone_l.append(__dram)
tstone_l.append(__nvram)


__one = '1'
__two = '2'
__three = '3'
__empty = ''
rl=[]
rl.append(__one)
rl.append(__two)
rl.append(__three)
rl.append(__empty)

parser = argparse.ArgumentParser(prog="runscript", description="plot script")
parser.add_argument('-r', dest='replicas', default=__empty, choices=rl)
try:
    args = parser.parse_args()

except:
    sys.exit(0)



lgnd_l = []

def plot(ax, tlist):

    line1 = ax.plot(tuple([x[0]/1000 for x in tlist[0]]), tuple([x[1] for x in tlist[0]]),linewidth=1, marker = 's')
    line2 = ax.plot(tuple([x[0]/1000 for x in tlist[1]]), tuple([x[1] for x in tlist[1]]),linewidth=1, marker = 's')

    ax.set_xlabel('throughput (1000 x tx/sec)',fontsize='6')
    ax.tick_params(axis='x',which='both',top='off',labelsize='6')

    ax.set_ylabel('latency(micro-sec)', fontsize='6')
    ax.tick_params(axis='y',which='both',right='off',labelsize='6')

    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)

    lgnd_l.append(line1)
    lgnd_l.append(line2)

if __name__ == '__main__':

  r = args.replicas
  if r == __empty:
      sys.exit(0)

  bufsize = [0, 1, 2, 3, 4, 5]
  t_l = []
  for idx1,item1 in enumerate(wrkld_l):
        for idx2,item2 in enumerate(tstone_l):
            temp_l = []
            for bsize in bufsize:
                filepath = result_home + '/' + item1 + '/rep' + r + '/' + \
                        item2 + '/' + str(bsize);
                temp_l.append(parse_dir(filepath))
            t_l.append(temp_l)

  print t_l
  pp = PdfPages('echoplot.pdf')
  fig, (ax1) = plt.subplots(nrows=1, ncols=1,figsize=(2.5,1.5))
  plot(ax1, t_l)

  plt.legend((lgnd_l[0][0],lgnd_l[1][0]),
             ('dram','nvram'),
             fontsize='6',ncol=1,loc = 'best',frameon=False)

  plt.tight_layout()
  pp.savefig(figure=fig)
  pp.close()








