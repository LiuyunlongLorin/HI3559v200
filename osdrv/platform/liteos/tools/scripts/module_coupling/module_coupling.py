#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import shutil
import operator

suffix = ['.a', '.o', '.so', '.O']

handled = []

module_list = {}

# ============= data structures =============
class vertex:
    name = ''
    path = ''
    in_vertexs = [] # 本模块被哪些其他模块所依赖
    out_vertexs = [] # 本模块依赖哪些其他模块
    undefined_symbol = None # 本模块中有哪些未定义符号
    defined_symbol = None # 本模块中有哪些定义符号

    def __init__(self, name, path):
        self.name = name
        self.path = path
        self.undefined_symbol = []
        self.defined_symbol = []


class edge:
    master = None # 被依赖的模块
    slave = None # 依赖的模块
    bywhat = '' # 依赖的桥梁符号

    def __init__(self, master, slave, bywhat):
        self.master = master
        self.slave = slave
        self.bywhat = bywhat


class graph:
    vertex_cnt = 0 # 顶点个数
    vertexs = [] # 所有的模块
    edges = [] # 所有的模块之间的依赖关系
    matrix = None # 邻接矩阵

    def __init__(self, vertex_cnt):
        self.vertex_cnt = vertex_cnt
        self.matrix = [[False for _ in xrange(vertex_cnt)] for _ in xrange(vertex_cnt)]

G = None

# ============= functions =============
# 获取 search_path 下的所有模块名及其路径
def get_module_list(path):
    global module_list
    if not os.path.isdir(path):
        module_list[os.path.basename(path)] = path
        return

    for file_name in os.listdir(path):
        cur_path = os.path.join(path, file_name)
        if os.path.isdir(cur_path):
            get_module_list(cur_path)
        elif reduce(operator.ior, map(lambda x: True if file_name.endswith(x) else False, suffix)):
            module_list[file_name] = cur_path


# 获取一个模块中的符号
def get_symbol(v):
    for line in os.popen('readelf -sW ' + v.path):
        s = line.strip().split()
        if len(s) != 8 or s[0] == 'Num:':
            continue

        BIND, NDX, NAME = s[4], s[6], s[7]
        if BIND != 'GLOBAL' and BIND != 'WEAK':
            continue
        if NDX != 'UND':
            v.defined_symbol.append(NAME)
        if NDX == 'UND':
            v.undefined_symbol.append(NAME)

    return v

# 获取模块间的依赖关系
def get_dependency(v1, i1, v2, i2):
    if (i1, i2) in handled or (i2, i1) in handled:
        return

    for ds1 in v1.defined_symbol:
        if ds1 in v2.undefined_symbol: # 模块 1 中有模块 2 未定义的符号 => 模块 2 依赖模块 1
            v1.in_vertexs.append(v2)
            v2.out_vertexs.append(v1)
            G.edges.append(edge(v1, v2, ds1))
            G.matrix[i2][i1] = True # v2 到 v1 有条边，谁在前谁是出发点
            break

    for ds2 in v2.defined_symbol:
        if ds2 in v1.undefined_symbol: # 模块 2 中有模块 1 未定义的符号 => 模块 1 依赖模块 2
            v2.in_vertexs.append(v1)
            v1.out_vertexs.append(v2)
            G.edges.append(edge(v2, v1, ds2))
            G.matrix[i1][i2] = True # v1 到 v2 有条边，谁在前谁是出发点
            break

    handled.append((i1, i2))

# 构建模块之间的网络
def construct_network():
    for i1, v1 in enumerate(G.vertexs):
        for i2, v2 in enumerate(G.vertexs):
            if i1 != i2:
                get_dependency(v1, i1, v2, i2)


# 初始化依赖图
def build_graph():
    global module_list, G
    G = graph(len(module_list))
    for k, v in module_list.items():
        G.vertexs.append(get_symbol(vertex(k, v)))
    del module_list


def show_all_modules():
    for v in G.vertexs:
        print v.name

def show_dependency(name):
    for e in G.edges:
        if (name and (e.slave.name == name or e.master.name == name)) or not name:
            print e.slave.name, '->', e.master.name, ' ', e.bywhat

def draw_graph(name):
    outfile = open('modules.dot', 'w')
    outfile.write('digraph G {\n')

    for e in G.edges:
        if (name and (e.slave.name == name or e.master.name == name)) or not name:
            outfile.write('"' + e.slave.name + '" -> "' + e.master.name + '"' + '[label="' + e.bywhat + '"]' + ';\n')

    outfile.write('}\n')
    outfile.close()

    os.popen("dot -Tjpg modules.dot -o modules.jpg")
    os.popen("modules.jpg")


def main():
    if len(sys.argv) < 2:
        print 'Usage:', sys.argv[0], 'search_path0 [, search_path1, ...]'
        return

    print 'Analysing ...'

    for path in sys.argv[1 : ]:
        realpath = os.path.realpath(path)
        get_module_list(realpath)

    build_graph()
    construct_network()

    while True:
        print "\n********************************************"
        print '(0) show all dependency'
        print '(1) show all modules'
        print '(2) show certain dependency'
        print '(3) draw all dependency'
        print '(4) draw dependency of certain module'
        print '(q) Quit'
        print "============================================\n"
        r = raw_input('Input choice[deault 0] ')
        if r == '0' or r == '':
            show_dependency(None)
        elif r == '1':
            show_all_modules()
        elif r == '2':
            name = raw_input('Input a name: ')
            show_dependency(name)
        elif r == '3':
            draw_graph(None)
        elif r == '4':
            name = raw_input('Input a name: ')
            draw_graph(name)
        elif r == 'q':
            break

if __name__ == '__main__':
    main()
