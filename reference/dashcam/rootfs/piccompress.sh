#!/bin/sh

# path configure
PIC_SRC_ROOT=$1

for file in ${PIC_SRC_ROOT}/*.png; do
	pngquant -f --ext .png --quality 50-50 $file
done
