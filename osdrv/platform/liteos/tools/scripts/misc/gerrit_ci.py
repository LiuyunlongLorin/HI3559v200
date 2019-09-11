#!/usr/bin/python
# -*- coding: UTF-8 -*-

import json

import os
import re
import sys

gerrit_port = 29418
gerrit_host = '10.186.67.216'

image_symbols = []

def work(gerrit_user, change_id):
    cmd = 'ssh -p ' + str(gerrit_port) + ' ' + gerrit_user + '@' + gerrit_host + ' gerrit query --format=JSON --current-patch-set --files ' + change_id

    for line in os.popen(cmd):
        data = json.loads(line)
        if 'status' in data and data['status'] == 'NEW':
            cmd = 'cd ' + data['project'] + ' && git fetch ssh://' + gerrit_user + '@' + gerrit_host + ':' + str(gerrit_port) + '/' + data['project'] + ' ' + data['currentPatchSet']['ref'] + ' && git cherry-pick FETCH_HEAD'
            os.popen(cmd)

def main():
    if len(sys.argv) != 3:
        print 'Usage:', sys.argv[0], 'gerrit_user change_id'
        return

    work(sys.argv[1], sys.argv[2])

if __name__ == '__main__':
    main()
