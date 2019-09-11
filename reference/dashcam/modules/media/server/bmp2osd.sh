#!/bin/sh

# path configure
CUR_DIR=`pwd`
REF_ROOT=${CUR_DIR}/../../../..
BMP2OSD_TOOL_NAME=bmp2osd
BMP2OSD_TOOL_PATH=${REF_ROOT}/out/tools/bin/${BMP2OSD_TOOL_NAME}/${BMP2OSD_TOOL_NAME}
BMP_SRC=${CUR_DIR}/logo.bmp

bmp2osd_dat()
{
	ffmpeg -i ${BMP_SRC} -s $1x$1 ${CUR_DIR}/logo_$1.bmp
	${BMP2OSD_TOOL_PATH} ${CUR_DIR}/logo_$1.bmp ${CUR_DIR}/logo_$1.bin
	xxd -c 16 -i `basename ${CUR_DIR}/logo_$1.bin` > ${CUR_DIR}/logo_$1.dat
	rm ${CUR_DIR}/logo_$1.bmp
	rm ${CUR_DIR}/logo_$1.bin
}

bmp2osd_dat 160;
bmp2osd_dat 86;
