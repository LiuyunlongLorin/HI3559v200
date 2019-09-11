#!/usr/bin/python

import sys
import os
import re
import operator

module_list = {
        'kernel': 'liblitekernel.a',
        'dynload': 'libdynload.a',
        'scatter': 'libscatter.a',
        'runstop': 'librunstop.a',
        'm2mcomm': 'libm2mcomm.a',
        'wpa': 'libwpa.a',
        'lwip': 'liblwip.a',
        'usb': 'libusb.a',
        'posix': 'libposix.a',
        'cmsis': 'libcmsis.a',
        'linux': 'liblinuxadp.a',
        'shell': 'libshell.a',
        'vfs': 'libvfs.a',
        'fat': 'libfat.a',
        'jffs': 'libjffs2.a',
        'yaffs': 'libyaffs2.a',
        'nfs': 'libnfs.a',
        'libc': 'libc.a',
        'libm': 'libm.a',
        }

sections = ['.text', '.data', '.bss', '.rodata']

def work(platform):
    print '\n', platform, '\n', '===================='
    for k, v in module_list.items():
        total_size = 0
        section_size = [0 for _ in xrange(len(sections))]
        for line in os.popen('readelf -S ' + reduce(os.path.join, ['.', 'out', platform, 'lib', v])):
            if ']' not in line or '.rel.' in line or '[Nr]' in line:
                continue
            name, size = line[line.find(']') + 1 : ].split()[0], line[line.find(']') + 1 : ].split()[4]
            for i, sp in enumerate(map(re.compile, sections)):
                if sp.match(name):
                    section_size[i] = section_size[i] + eval('0x' + size)
                    total_size = total_size + eval('0x' + size)
        print k + ':'
        print '%-8s %6.1fk\t'%('total:', round(total_size / 1024.0, 1))
        for i, s in enumerate(sections):
            print '%-8s %6.1fk\t'%(s + ':', round(section_size[i] / 1024.0, 1)),
        print '\n'

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'Usage:', sys.argv[0], 'platform0 [, platform1, ...]'
    else:
        for platform in sys.argv[1 : ]:
            work(platform)
