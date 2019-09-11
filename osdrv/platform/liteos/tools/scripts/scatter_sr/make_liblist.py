#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import shutil

symbol_libs = {}
current_search_dir = ''

def do_work(full_path, file_name):
    for line in os.popen('readelf -sW ' + full_path):
        s = line.strip().split()
        if len(s) != 8 or s[0] == 'Num:':
            continue
        BIND, NDX, NAME = s[4], s[6], s[7]
        if (BIND == 'GLOBAL' or BIND == 'WEAK') and NDX != 'UND' and NAME in symbol_libs:
            symbol_libs.setdefault(NAME, []).append(full_path)
            return

def work(path):
    if not os.path.isdir(path):
        do_work(path, os.path.split(path)[-1])
        return

    for file_name in os.listdir(path):
        cur_path = os.path.join(path, file_name)
        if os.path.isdir(cur_path):
            work(cur_path)
        elif file_name.endswith('.a'):
            do_work(cur_path, file_name)

def main():
    if len(sys.argv) < 3:
        print 'Usage:', sys.argv[0], 'symbol_list_path search_path'
        return

    # load symbol list
    for line in open(sys.argv[1]):
        symbol_libs[line.strip()] = []

    for path in sys.argv[2 : ]:
        global current_search_dir
        abspath = os.path.abspath(path)
        current_search_dir = abspath
        work(abspath)

    for symbol, libs in sorted(symbol_libs.iteritems(), key=lambda d: len(d[1])):
        if not libs:
            continue
        if len(libs) == 1:
            print libs[0]
            continue

        print '++++++++++++++++++++++++++++++++++++++'
        print symbol + ' defined in ' + str(len(libs)) + ' libs:'
        for lib in libs:
            print lib

if __name__ == '__main__':
    main()
