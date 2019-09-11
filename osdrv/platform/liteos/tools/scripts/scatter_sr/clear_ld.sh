#!/bin/bash

# ld dir
ld_dir=$1

# clear wow.ld
sed -i '/\.rodata/d' $ld_dir/wow.ld
sed -i '/\.constdata/d' $ld_dir/wow.ld
sed -i '/\.text/d' $ld_dir/wow.ld
sed -i '/\.data/d' $ld_dir/wow.ld
sed -i '/\.bss/d' $ld_dir/wow.ld
sed -i '/\COMMON/d' $ld_dir/wow.ld
sed -i '/\.int_stack/d' $ld_dir/wow.ld

# clear scatter.ld
sed -i '/\.rodata/d' $ld_dir/scatter.ld
sed -i '/\.constdata/d' $ld_dir/scatter.ld
sed -i '/\.text/d' $ld_dir/scatter.ld
sed -i '/\.data/d' $ld_dir/scatter.ld
