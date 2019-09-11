#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import shutil

def do_work(full_path):
    indexs = []
    for r_i, line in enumerate(open(full_path)):
        for c_i, ch in enumerate(line):
            if ord(ch) & 0x80 != 0 and len(indexs) <= 10:
                indexs.append(r_i)
                break
    if indexs:
        print full_path
        print indexs

def work(path):
    if not os.path.isdir(path):
        do_work(path)
        return

    for file_name in os.listdir(path):
        cur_path = os.path.join(path, file_name)
        if os.path.isdir(cur_path):
            work(cur_path)
        elif file_name.endswith('.c'):
            do_work(cur_path)

def main():
    if len(sys.argv) < 2:
        print 'Usage:', sys.argv[0], 'search_path'
        return

    for path in sys.argv[1 : ]:
        realpath = os.path.realpath(path)
        work(realpath)

if __name__ == '__main__':
    main()
