#!/bin/bash

text_flag=`readelf -S $1 | grep "\.text" | awk '{print $1}'`
mem_flag=`readelf -S $1 | grep "\.ram_vectors" | awk '{print $1}'`

if [ $mem_flag = "[" ]; then
	mem_addr=`readelf -S $1 | grep "\.ram_vectors" | awk '{print $5}'`
else
	mem_addr=`readelf -S $1 | grep "\.ram_vectors" | awk '{print $4}'`
fi
if [ $text_flag = "[" ]; then
	text_addr=`readelf -S $1 | grep "\.text" | awk '{print $5}'`
else
	text_addr=`readelf -S $1 | grep "\.text" | awk '{print $4}'`
fi

let scatter_size=(16#$text_addr-16#$mem_addr)

let byte_align=(16#FFF000+16#800) #0xFFF000 + 0x800

let scatter=($scatter_size'&'$byte_align)

echo "###### ###### ##### ##### #####"
echo "###### scatter load size ######"
echo "###############################"
printf "           0x%x\n" ${scatter_size}
echo "###############################"
