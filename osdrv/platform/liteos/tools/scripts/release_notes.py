#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import re
import sys

release_notes = {
    'Bugfix': {},
    'Feature': {},
}

modules = [
    'compat',
    'build',
    'doc',
    'drivers',
    'fs',
    'kernel',
    'lib',
    'net',
    'platform',
    'sample',
    'shell',
    'tools',
    '3rdParty',
]

repos = [
    '/compat/cmsis',
    '/compat/linux',
    '/compat/posix',
    '/doc',
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
    '/tools',
    ]

root_path = ''

def work(root_path, since):
    for repo in repos:
        is_merge, is_private, is_commit, is_description, is_first = False, False, False, False, True
        cur_description, cur_module, cur_bug_type = '', '', ''
        cmd = 'cd ' + root_path + repo + ' && git log --since=' + '"' + since + '"'

        for line in os.popen(cmd):
            line = line.strip()

            if line.startswith('commit'):
                if not is_merge and not is_private and not is_first:
                    print cur_description + ' == ' + cur_module + '===' + cur_bug_type
                    if cur_description == '':
                        print "Modify content is illegal"
                        continue
                    if cur_module == '':
                        print "Module is illegal"
                        continue
                    if cur_bug_type == '':
                        print "Bug Type is illegal"
                        continue
                    release_notes[cur_bug_type][cur_module].append(cur_description)
                is_merge, is_private, is_commit, is_description, is_first = False, False, True, False, False
                cur_description, cur_module, cur_bug_type = '', '', ''
                continue

            if line.startswith('Merge:') and is_commit:
                is_merge = True

            if line.startswith('Author:') and is_commit:
                is_merge = False

            is_commit = False

            if is_merge or is_private:
                continue

            if line.startswith('Modify content:'):
                cur_description = line[len('Modify content:') : ].strip()
                is_description = True
                continue

            if line.startswith('Module name:'):
                cur_module = line[len('Module name:') : ].strip()
                is_description = True
                continue
            #    for module in modules:
            #        if line[len('Model name:') : ].strip() == module:
            #            cur_module = module
            #            is_description = False
            #            break

            #if is_description:
            #    cur_description = (cur_description + ' ' + line).strip()
            #    continue

            if line == '[Public]':
                is_private = False
            elif line == '[Private]':
                is_private = True

            if line.startswith('Bug Type:'):
                cur_bug_type = line[len('Bug Type:') : ].strip()

def init_log():
    for module in modules:
        global release_notes
        release_notes['Bugfix'][module] = []
        release_notes['Feature'][module] = []

def out_put():
    rn_file = open('ReleaseNotes.txt', 'w+')

    rn_file.write('New Features\nThe following are some new features with this release.\n')

    for module, descriptions in release_notes['Feature'].items():
        if not descriptions:
            continue

        rn_file.write('\n    * ' + module + ':\n')
        for description in descriptions:
            rn_file.write('      - ' + description + '\n')

    rn_file.write('\nBug Fixs\nThe following are some bug fixs with this release.\n')
    for module, descriptions in release_notes['Bugfix'].items():
        if not descriptions:
            continue

        rn_file.write('\n    * ' + module + ':\n')
        for description in descriptions:
            rn_file.write('      - ' + description + '\n')

    rn_file.close()

def main():
    if len(sys.argv) != 3:
        print 'Usage: sys.argv[0] root_path starttime'
        return

    init_log()

    root_path, since = sys.argv[1], sys.argv[2]
    realpath = os.path.realpath(root_path)
    work(realpath, since)

    out_put()

if __name__ == '__main__':
    main()
