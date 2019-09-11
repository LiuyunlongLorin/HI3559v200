#!/bin/sh

# path configure
CUR_DIR=`pwd`
PIC_SRC_ROOT=${CUR_DIR}/res/pic
PIC_SRC_ROOT_DEST=${CUR_DIR}/res/picdest
BASE_WIDTH=$1
BASE_HEGIHT=$2
DEST_WIDTH=$3
DEST_HEGIHT=$4
fileresize()
{
	if [[ $1 =~ "gif" ]]
	then
	cp -rf ${PIC_SRC_ROOT}/$1 ${PIC_SRC_ROOT_DEST}/$1
	return
	fi
	ffmpeg -i ${PIC_SRC_ROOT}/$1 -vf scale=iw*$2:ih*$3 ${PIC_SRC_ROOT_DEST}/$1 -y
}

for file in ${PIC_SRC_ROOT}/*; do
eval $(awk 'BEGIN{printf("WIDTH_SCALE=%.2f",'${DEST_WIDTH}'/'${BASE_WIDTH}')}')
eval $(awk 'BEGIN{printf("HEIGHT_SCALE=%.2f",'${DEST_HEGIHT}'/'${BASE_HEGIHT}')}')
	fileresize $(basename $file) ${WIDTH_SCALE} ${HEIGHT_SCALE}
done
