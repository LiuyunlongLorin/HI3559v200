#!/bin/sh

./mkyaffs2image610 rootfs rootfs_mn34220_30fps_2k_4bit.yaffs2 1 2
./mkfs.jffs2  -s 0x1000 -e 0x10000 -p 0x100000 -d rootfs/ -o rootfs_64k.jffs2
