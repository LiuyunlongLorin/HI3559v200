#!/bin/bash

so_dir=$1
cur_root=`pwd`
project_dir=${cur_root}/../../..

$cur_root/dynload_sh/make_symlst.py $so_dir > $cur_root/dynload_sh/symbol.list

$cur_root/dynload_sh/make_symcfile.sh $cur_root/dynload_sh/symbol.list $cur_root/dynload_sh/los_dynload_gsymbol.c
cp $cur_root/dynload_sh/los_dynload_gsymbol.c $project_dir/kernel/extended/dynload/src

$cur_root/dynload_sh/make_ldflags.sh
cp $cur_root/dynload_sh/dynload_ld.mk $project_dir/build/mk

rm $cur_root/dynload_sh/los_dynload_gsymbol.c
rm $cur_root/dynload_sh/symbol.list
rm $cur_root/dynload_sh/dynload_ld.mk
