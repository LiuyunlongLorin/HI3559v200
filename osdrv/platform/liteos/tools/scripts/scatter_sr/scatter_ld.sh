#!/bin/bash

ld_dir=$1

# add include
#sed -i '/scatter.ld/d' $root_dir/liteos.ld
#sed -i "/__fast_start/a\INCLUDE tools\/scripts\/ld\/scatter.ld" $root_dir/liteos.ld
sed -i '/\.rodata/d' $ld_dir/scatter.ld
sed -i '/\.text/d' $ld_dir/scatter.ld
sed -i '/\.data/d' $ld_dir/scatter.ld

# fast_rodata
sed -i '/.fast_rodata ALIGN(0x1000) : /a\scatter.O(.rodata*); scatter.O(.constdata*);' $ld_dir/scatter.ld

# scatter_text
sed -i '/.fast_text ALIGN(0x4) : /a\scatter.O(.text*); scatter.O(ch07_3);' $ld_dir/scatter.ld

# fast_data
sed -i '/.fast_data ALIGN(0x4) : /a\ . = ALIGN(0x4); KEEP(scatter.O(SORT(.liteos.table.*.wow.*))); KEEP(scatter.O(SORT(.liteos.table.*.scatter.*))); . = ALIGN(0x4); scatter.O(.data*);' $ld_dir/scatter.ld
