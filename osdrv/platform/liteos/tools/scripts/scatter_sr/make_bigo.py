#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import re
import sys

image_symbols = []

def work(libs_symbols_path):
    cur_lib, cur_obj = '', ''
    last_lib, last_obj = '', ''
    pattern = re.compile(r'File: (\S+)\((\S+)\)')
    for line in open(libs_symbols_path):
        s = line.strip().split()
        match = pattern.match(line)
        if match:
            cur_lib, cur_obj = match.groups()
            continue
        elif len(s) != 8 or s[0] == 'Num:':
            continue

        BIND, NDX, NAME = s[4], s[6], s[7]
        if (BIND == 'GLOBAL' or BIND == 'WEAK') and NDX != 'UND' and NAME in image_symbols:
            if cur_lib != last_lib or cur_obj != last_obj:
                print os.path.split(cur_lib)[-1], cur_obj
                last_lib, last_obj = cur_lib, cur_obj

def main():
    if len(sys.argv) < 3:
        print 'Usage:', sys.argv[0], 'image_symbols_list_path libs_symbols_list_path'
        return

    # load image symbols list
    abspath = os.path.abspath(sys.argv[1])
    for line in open(abspath):
        image_symbols.append(line.strip())

    abspath = os.path.abspath(sys.argv[2])
    work(abspath)

if __name__ == '__main__':
    main()
