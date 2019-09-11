#!/bin/bash

function rand(){
    min=$1
    max=$(($2-$min+1))
    num=$(cat /proc/sys/kernel/random/uuid | cksum | awk -F ' ' '{print $1}')
    echo $(($num%$max+$min))
}

function file_size(){
	stat -c %s $1 | tr -d '\n'
}

if [ $# -eq 0 ];
then
	echo "no file name gived"
	exit
fi

for i in {1..500}
do
	filesize=$(file_size $1)
    rnd=$(rand 0 $filesize)
    num=`printf "%04d\n" $i`
    echo "split "$rnd" bit to rep_"$num".mp4"
    head -c $rnd $1 > test_repair/rep_$num.mp4
    echo "success"
done
exit 0
