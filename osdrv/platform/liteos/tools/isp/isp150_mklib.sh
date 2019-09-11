#!/bin/bash
#build the source to get lib
echo "##########step1: start to make ###########"
TOOLSDIR=`pwd`
echo "TOOLSDIR="$TOOLSDIR
cd ../../
pwd
TOPDIR=`pwd`
echo "TOPDIR="$TOPDIR
cd isp/v150_chicago/build/
BUILDDIR=`pwd`
echo "BUILDDIR="$BUILDDIR
cd $BUILDDIR
pwd
make clean;make MMU=1 -j

echo "##########step2: start to cp libs ###########"
cd $TOPDIR
LITEOSLIBSDIR=isp/v150_chicago/build/libs/liteos
LITEOSOUTDIR=isp/v150_chicago/build/out
mkdir $LITEOSLIBSDIR
cp $LITEOSOUTDIR/libas.a $LITEOSLIBSDIR/
cp $LITEOSOUTDIR/libs/firmware/libcortex-a7.a $LITEOSLIBSDIR/
cp $LITEOSOUTDIR/libs/firmware//libshell.a $LITEOSLIBSDIR/
cp $LITEOSOUTDIR/libs/firmware/libperf.a $LITEOSLIBSDIR/
cp $LITEOSOUTDIR/libs/firmware/libisp_rdr_exc.a $LITEOSLIBSDIR/

echo "##########step3: start to rm src ###########"

#拷贝内核源码
FWHWDIR=isp/v150_chicago/firmware/system/HuaweiLite_OS
FWLITEOSKERBASE=$FWHWDIR/kernel/base
rm $FWLITEOSKERBASE/core -rf
cp kernel/base/core $FWLITEOSKERBASE/ -rf
rm $FWLITEOSKERBASE/include -rf
cp kernel/base/include $FWLITEOSKERBASE/ -rf
rm $FWLITEOSKERBASE/ipc -rf
cp kernel/base/ipc $FWLITEOSKERBASE/ -rf
rm $FWLITEOSKERBASE/mem -rf
cp kernel/base/mem $FWLITEOSKERBASE/ -rf
rm $FWLITEOSKERBASE/misc -rf
cp kernel/base/misc $FWLITEOSKERBASE/ -rf
rm $FWLITEOSKERBASE/om -rf
cp kernel/base/om $FWLITEOSKERBASE/ -rf
rm $FWLITEOSKERBASE/cpup -rf
cp kernel/extended/cpup/los_cpup.c $FWLITEOSKERBASE/core/
cp kernel/extended/cpup/los_cpup.inc $FWLITEOSKERBASE/core/
cp kernel/extended/include/los_cpup.ph $FWLITEOSKERBASE/include/

rm isp/v150_chicago/include/system/HuaweiLite_OS -rf
cp kernel/include isp/v150_chicago/include/system/ -rf
mv isp/v150_chicago/include/system/include isp/v150_chicago/include/system/HuaweiLite_OS

#拷贝isp150、a7源码
rm $FWHWDIR/platform/bsp/isp150 -rf
cp platform/bsp/isp150 $FWHWDIR/platform/bsp/ -rf
rm $FWHWDIR/platform/cpu/arm/cortex-a7 -rf
cp platform/cpu/arm/cortex-a7 $FWHWDIR/platform/cpu/arm/ -rf

#删除a7、perf、shell、test源码
find $FWHWDIR/platform/cpu/arm/cortex-a7 -name "*.[cs]" |xargs rm
find $FWHWDIR/perf -name "*.[cs]" |xargs rm
rm $FWHWDIR/perf/module.mk
rm $FWHWDIR/test -rf
rm $FWHWDIR/shell/src -rf
rm $FWHWDIR/shell/module.mk

rm $FWHWDIR/platform/bsp/isp150/cache.s

rm $FWHWDIR/extended -rf

#调整文件路径
INCLUDESYSHWDIR=isp/v150_chicago/include/system/HuaweiLite_OS
mkdir $INCLUDESYSHWDIR/asm
mv $FWHWDIR/platform/bsp/isp150/include/asm/hal_platform_ints.h $INCLUDESYSHWDIR/asm/
mv $FWHWDIR/platform/bsp/isp150/include/asm/dma.h $INCLUDESYSHWDIR/asm/
mv $FWHWDIR/platform/bsp/isp150/include/isp_printf.h $INCLUDESYSHWDIR/
mv $FWHWDIR/platform/bsp/isp150/config/los_config.h $INCLUDESYSHWDIR/
mv $FWHWDIR/platform/bsp/isp150/config/los_builddef.h $INCLUDESYSHWDIR/
mv $FWHWDIR/platform/cpu/arm/cortex-a7/los_hwi.h $INCLUDESYSHWDIR/
mv $FWHWDIR/platform/cpu/arm/cortex-a7/los_hw.h $INCLUDESYSHWDIR/

#编译修改，不编a7,cpup
#!/bin/sh
file_rm=$FWHWDIR/platform/module.mk
sed -i '11d' $file_rm
cpup_rm=$FWLITEOSKERBASE/module.mk
sed -i '17d' $cpup_rm

#删除标定数据源码
rm isp/v150_chicago/build/separator/source -rf

echo "##########step4: enable libmk ###########"
cd $TOOLSDIR
mv isp150_libmk ../../isp/v150_chicago/build/build/main.mk
cd $BUILDDIR

echo "########## isp150_mklib.sh end ###########"

