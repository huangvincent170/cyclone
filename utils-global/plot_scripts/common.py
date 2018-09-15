#!/usr/bin/env python

SAMPLE_SIZE = 100

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

if __name__ == '__main__':

    parse_file('../results/client_10.212.43.11.log')