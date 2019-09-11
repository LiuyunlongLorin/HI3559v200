#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import shutil

includes = [
    os.path.realpath('.') + '/compat/linux/include',
]

incs = []

def incs_handler(path):
    for inc in includes:
        if path.startswith(inc):
            left = path[len(inc) + 1 : ]
            incs.append(left)

def find_handler(path):
    cnt = 0
    for line in open(path):
        cnt = cnt + 1
        for inc in incs:
            if '"' + inc + '"' in line or '<' + inc + '>' in line:
                print path[len(os.path.realpath('.')) + 1 : ], '\t +' + str(cnt)
                return

def work(path, handler, suffix):
    if not os.path.isdir(path) and cur_path.endswith(suffix):
        handler(cur_path)
        return

    for file_name in os.listdir(path):
        cur_path = os.path.join(path, file_name)
        if os.path.isdir(cur_path):
            work(cur_path, handler, suffix)
        elif cur_path.endswith(suffix):
            handler(cur_path)

def main():
    work(os.path.realpath('.'), incs_handler, '.h')
    work(os.path.realpath('.'), find_handler, '.c')

if __name__ == '__main__':
    main()
