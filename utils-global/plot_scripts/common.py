#!/usr/bin/env python

import os
import sys
import glob

SAMPLE_SIZE = 100


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

    #msg(cmd)
    try:
        os.system(cmd)
    except:
        print 'invalid cmd', cmd
        sys.exit(0)

def file_len(fname):
    with open(fname) as f:
        for i, l in enumerate(f):
            pass
    return i + 1

def get_tl_tuple(line):
    slines = line.split(' ')
    if(slines[4] == 'LOAD' and slines[8] == 'LATENCY'):
        return (float(slines[6]),float(slines[10]))
    return None


def parse_dir(dirpath):
    regexpath = dirpath + '/' + 'client_*.log'
    dirlist = glob.glob(regexpath)
    for f in dirlist:
        return parse_file(f)



def parse_file(fpath):
    tl_l = []
    nlines = file_len(fpath)
    start_line = nlines/2;

    with open(fpath) as f:
        for i in xrange(start_line):
            f.next()
        cnt=0
        for line in f:
            tuple = get_tl_tuple(line)
            if tuple is not None:
                tl_l.append(tuple)
            if cnt == SAMPLE_SIZE:
                break
            cnt += 1

    ave_load = 0
    ave_lat = 0
    for x in tl_l:
        ave_load += x[0]
        ave_lat += x[1]
    ave_load = ave_load/len(tl_l)
    ave_lat = ave_lat/len(tl_l)
    return (ave_load,ave_lat)