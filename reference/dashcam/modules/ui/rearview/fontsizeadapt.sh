#!/bin/sh

# path configure
CUR_DIR=`pwd`
FONT_ROOT_DEST=${CUR_DIR}/xmldest
BASE_WIDTH=$1
BASE_HEGIHT=$2
DEST_WIDTH=$3
DEST_HEGIHT=$4
eval $(awk 'BEGIN{printf("SIZE_SCALE=%.2f",'${DEST_WIDTH}'*'${DEST_HEGIHT}'/'${BASE_WIDTH}'/'${BASE_HEGIHT}')}')

for x in $(grep 'size' ${FONT_ROOT_DEST}/font.xml); do
NUM=$(echo $x | tr -cd "[0-9]")
ORINGIN=$NUM
eval $(awk 'BEGIN{printf("NEWSIZE=%d",'${NUM}'*'${SIZE_SCALE}')}')
sed -i "s/size=\"$ORINGIN\"/size=\"$NEWSIZE\"/g"  ${FONT_ROOT_DEST}/font.xml
done
