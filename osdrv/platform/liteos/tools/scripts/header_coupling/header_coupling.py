#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys
import os
import shutil

includes = [
    './kernel/extended/dynload/include',
    './compat/cmsis',
    './kernel/include',
    './kernel/extended/scatter',
    './kernel/extended/cppsupport',
    './kernel/extended/runstop',
    './drivers/rtc/include',
    './drivers/wtdg/include',
    './drivers/lcd/include',
    './drivers/cellwise/include',
    './drivers/gpio/include',
    './drivers/i2c/include',
    './drivers/usb',
    './drivers/uart/include',
    './drivers/spi/include',
    './drivers/mmc/include',
    './drivers/mtd/common/include',
    './drivers/mtd/nand/include',
    './drivers/mtd/spi_nor/include',
    './drivers/hidmac/include',
    './drivers/random/include',
    './lib/libc/include',
    './lib/libc/src/stdio',
    './lib/libc/src/time',
    './lib/zlib/include',
    './compat/posix/include',
    './compat/linux/include',
    './compat/posix/src',
    './lib/libm/include',
    './fs/vfs/blib',
    './fs/include',
    './net/lwip_sack/include',
    './net/lwip_sack/include/ipv4',
    './net/mac',
    './fs/proc/include',
    './shell/include',
    './lib/libc/libwc/include',
]

dic = {}

def handle(path):
    for inc in includes:
        if path.startswith(inc):
            left = path[len(inc) + 1 : ]
            dic.setdefault(left, []).append(inc)

def work(path):
    if not os.path.isdir(path):
        if cur_path.endswith('.h'):
            handle(cur_path)
        return

    for file_name in os.listdir(path):
        cur_path = os.path.join(path, file_name)
        if os.path.isdir(cur_path):
            work(cur_path)
        elif cur_path.endswith('.h'):
            handle(cur_path)

def main():
    if len(sys.argv) < 2:
        print 'Usage:', sys.argv[0], 'search_path1[, search_path2, ...]'
        return

    for path in sys.argv[1 : ]:
        realpath = os.path.realpath(path)
        work(realpath)

    for k, v in dic.items():
        if len(v) >= 2:
            print k, v

if __name__ == '__main__':
    main()
