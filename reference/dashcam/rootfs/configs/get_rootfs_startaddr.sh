#!/bin/sh

if [ $# -ne 1 ];then
	echo "usage $0 burn.xml-path"
	echo "usage $0 configs/hi3559v200/config_ext4_ext4_emmc.xml"
	exit
fi

echo `cat ${1} |grep rootfs |awk '{print $6}' |awk -F\" '{print $2}' |awk -FM '{print $1}'`
