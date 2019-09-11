#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import sys
repos = [
'/build',
'/compat/cmsis',
'/compat/linux',
'/compat/posix',
'/drivers/gpio',
'/drivers/hidmac',
'/drivers/hieth-sf',
'/drivers/higmac',
'/drivers/i2c',
'/drivers/mem',
'/drivers/mmc',
'/drivers/mtd/common',
'/drivers/mtd/nand',
'/drivers/mtd/spi_nor',
'/drivers/random',
'/drivers/rtc',
'/drivers/spi',
'/drivers/uart',
'/drivers/usb',
'/drivers/video',
'/drivers/wifi',
'/drivers/wtdg',
'/fs/fat',
'/fs/include',
'/fs/jffs2',
'/fs/nfs',
'/fs/proc',
'/fs/ramfs',
'/fs/vfs',
'/fs/yaffs2',
'/kernel',
'/lib',
'/net',
'/platform/bsp',
'/platform/cpu',
'/sample',
'/shell',
'/test',
'/tools',
]
root_path = ''
def nonblank_lines(f):
    for l in f:
        line = l.rstrip()
        if line:
            yield line

def work(realpath, since):
    total_add_loc, total_del_loc = 0,0
    for repo in repos:
        add_loc, del_loc = 0,0
        cmd = 'cd ' + realpath + repo + ' && git log --since=' + '"' + since + '"' + ' --pretty=tformat: --numstat'
        for line in nonblank_lines(os.popen(cmd)):
            line = line.strip().split()
            if line[0].isdigit():
                add_loc += int(line[0])
            if line[1].isdigit():
                del_loc += int(line[1])
        if add_loc>0 and del_loc>0:
            print("+%-20d -%-20d  %s" %(add_loc, del_loc, repo))
            total_add_loc += add_loc
            total_del_loc += del_loc
    print("+%-20d -%-20d  total" %(total_add_loc, total_del_loc))

def main():
    if len(sys.argv) != 3:
        print 'Usage: %s rootpath starttime' %sys.argv[0]
        print 'Please excute under folder Huawei_LiteOS'
        print 'eg: %s . 2017-06-06' %sys.argv[0]
        return

    root_path, since = sys.argv[1], sys.argv[2]
    realpath = os.path.realpath(root_path)

    work(realpath, since)

if __name__ == '__main__':
    main()


