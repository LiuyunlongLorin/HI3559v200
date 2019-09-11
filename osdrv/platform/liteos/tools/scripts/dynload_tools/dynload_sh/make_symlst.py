#!/usr/bin/python
# -*- coding: UTF-8 -*-

'''
此脚本生成指定路径下的库所依赖的外部符号(未定义符号)列表
'''

import sys
import os
import shutil
import operator

edit_suffixs = ['.o', 'so', 'so.53', 'so.51', 'so.54', 'so.56']
namesExternModule = []
namesUserModule = []

def do_work(full_path):
    for line in os.popen('readelf -sW ' + full_path):
        s = line.strip().split()
        if len(s) != 8 or s[0] == 'Num:':
            continue
        BIND, NDX, NAME = s[4], s[6], s[7]
        NAME = NAME.split('@')[0]
        if (BIND == 'GLOBAL' or BIND == 'WEAK') and (NDX != 'UND') and (NAME not in namesUserModule):
            namesUserModule.append(NAME)
            if NAME in namesExternModule:
                namesExternModule.remove(NAME)
        if (BIND == 'GLOBAL' or BIND == 'WEAK') and (NDX == 'UND') and (NAME not in namesExternModule) and (NAME not in namesUserModule):
            namesExternModule.append(NAME)

def work(path):
    if not os.path.isdir(path):
        do_work(path)
        return

    for file_name in os.listdir(path):
        cur_path = os.path.join(path, file_name)
        if os.path.isdir(cur_path):
            work(cur_path)
        elif reduce(operator.ior, map(lambda x: True if file_name.endswith(x) else False, edit_suffixs)):
            do_work(cur_path)

if __name__ == '__main__':
    if len(sys.argv) == 1:
        work(os.path.realpath('.'))
    else:
        for path in sys.argv[1 : ]:
            realpath = os.path.realpath(path)
            work(realpath)

    names = 0
    namesExternModule.sort()
    for names in namesExternModule:
        print names
