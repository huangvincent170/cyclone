#!/usr/bin/env python
import argparse
import sys
import os
import shutil

DBG = 1

__home = os.getcwd()
__fbench_home = '/home/pfernando/filebench'  #binary

__empty = ''
__micro_rread = 'micro_rread'

__workload_l = []
__workload_l.append(__micro_rread)

parser = argparse.ArgumentParser(prog="runscript", description="script to run filebench")
parser.add_argument('-w', dest='workload', default=__empty, help='', choices=__workload_l)
parser.add_argument('-io', dest='iosize', default=__empty, help='')

try:
    args = parser.parse_args()

except:
    sys.exit(0)


def dbg(s):
    if DBG == 1:
        print s


def msg(s):
    print '\n' + '>>>' + s + '\n'


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


def tonum(str):
    ch = str.strip()[-1]
    num = long(str[:-1])
    if (ch == 'k'):
        return num * 1024
    elif (ch == 'm'):
        return num * 1024 ** 2
    elif (ch == 'g'):
        return num * 1024 ** 3
    else:
        return num


def fb(wl,dataobj):
    __t = wl + '.template'
    __out = wl + '.f'

    # generate workload file from template
    cd('wrklds')

    template = open(__t, "rt").read()

    with open(__out, "wt") as output:
        output.write(template % dataobj)

    cd(__home)
    cmd = __fbench_home +'/filebench'
    cmd += ' -f '  + 'wrklds/' + __out
    msg(cmd)
    sh(cmd)

    # delete the generated file
    os.remove('wrklds/' + __out)

    cd(__home)

if __name__ == '__main__':

    w = args.workload
    io = args.iosize

    if w == __micro_rread:
        __iosize = io
        msg("io size : " + io)
        dataobj = {"iosize": io}
        fb(__micro_rread,dataobj)
    else:
        sys.exit(0)
