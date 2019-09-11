#!/bin/bash

# cross_compile
cross_compile=$1

wow_switch=$2

scatter_switch=$3

# scatter_sr dir
scatter_sr_dir=$4

# ld dir
scatter_wow_ld_dir=$5

# temp dir where *.Os are made
tmp_dir=$6

# dir which contains *.Os, must consistent with -L in Makefile
objs_dir=$7

ar=${cross_compile}ar

lib_list=

if [ "$wow_switch"x = "y"x -a "$scatter_switch"x = "n"x ]; then
    lib_list=lib_list.wow
elif [ "$wow_switch"x = "n"x -a "$scatter_switch"x = "y"x ]; then
    lib_list=lib_list.scatter
else
    cat $scatter_sr_dir/lib_list.wow > tmp_lib_list
    cat $scatter_sr_dir/lib_list.scatter >> tmp_lib_list
    cat tmp_lib_list | sort | uniq > lib_list.collection
    rm tmp_lib_list
    mv lib_list.collection $scatter_sr_dir
    lib_list=lib_list.collection
fi

# make objs_dir, tmp_dir
rm -rf $objs_dir && mkdir $objs_dir && rm -rf $tmp_dir && mkdir -p $tmp_dir

# cp libs and some objs to tmp_dir
while read LINE; do
    lib=`echo $LINE | awk -F " " '{print $1}'`
    if [ -f $lib ]; then
        cp $lib $tmp_dir
        readelf -sW $lib >> $tmp_dir/libs_symbols.list
    fi
done < $scatter_sr_dir/$lib_list

# break libs up
cd $tmp_dir
for libname in *.a; do
    if [ -f $libname ]; then
        dir=${libname%\.a}
        mkdir $dir; cd $dir;
        $ar x $tmp_dir/$libname

        if [ "libhi3516asdk" = "$dir" ]; then
            rm "pwm_write.o"
        fi

        if [ "libpolarssl" = "$dir" ]; then
            rm "timing.o"
        fi

        if [ "libgcc" = "$dir" ]; then
            rm "linux-atomic-64bit.o"
        fi

        if [ "libproduct_funcs" = "$dir" ]; then
            rm "fb_dsp.o"
        fi

        cd $tmp_dir
    fi
done

# clear *.ld
$scatter_sr_dir/clear_ld.sh $scatter_wow_ld_dir

if [ "$wow_switch"x = "y"x ]; then
    # make wow.O
    $scatter_sr_dir/wow_tool.sh $cross_compile $tmp_dir $scatter_sr_dir $objs_dir
    # modify wow.ld
    $scatter_sr_dir/wow_ld.sh $scatter_wow_ld_dir $root_dir
fi

if [ "$scatter_switch"x = "y"x ]; then
    # make scatter *.O
    $scatter_sr_dir/scatter_tool.sh $cross_compile $tmp_dir $scatter_sr_dir $objs_dir
    # modify scatter.ld
    $scatter_sr_dir/scatter_ld.sh $scatter_wow_ld_dir $root_dir
fi
cd ..
rm -rf $tmp_dir
