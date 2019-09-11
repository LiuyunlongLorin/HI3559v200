
cd $LITEOSTOPDIR
echo "##########step1:build over!!! start to copy libs###########"
#copy the lib to the direct
cp out/lib/libblib.a fs/vfs/blib/
cp out/lib/libshell.a shell/
cp out/lib/libscatter.a kernel/extended/scatter
cp out/lib/libcortex-a7.a platform/arm/cortex-a7
cp out/lib/libarm926.a platform/arm/arm926
cp out/lib/libm2mcomm.a kernel/m2mcomm
cp out/lib/libdynload.a kernel/extended/dynload
cp out/lib/librunstop.a kernel/extended/runstop
cp out/lib/libcppsupport.a kernel/extended/cppsupport
cp out/lib/liblinuxadp.a  compat/linux
cp out/lib/libposix.a  compat/posix
cp out/lib/libc.a lib/libc
cp out/lib/libm.a lib/libm
cp out/lib/libz.a lib/zlib
cp out/lib/libfat.a fs/fat
cp out/lib/libjffs2.a fs/jffs2
cp out/lib/libnfs.a fs/nfs
cp out/lib/libramfs.a fs/ramfs
cp out/lib/libvfs.a fs/vfs
cp out/lib/libyaffs2.a  fs/yaffs2
cp out/lib/libiperf.a out/lib/libar6003.a  out/lib/libwpa.a  vendor/ar6k3_wifi
cp out/lib/liblwip.a  kernel/net/lwip
cp out/lib/libusb.a drivers/usb
echo "##########step2:copy over!!! delete the source###########"
#delete the c source
find shell -name "*.c" |xargs rm
find fs/vfs/blib -name "*.c" |xargs rm
find kernel/m2mcomm -name "*.cpp" |xargs rm
find kernel/extended/dynload -name "*.c" |xargs rm
find kernel/extended/runstop -name "*.c" |xargs rm
find kernel/extended/runstop -name "*.s" |xargs rm
find kernel/extended/cppsupport -name "*.c" |xargs rm
find compat/posix compat/linux -name "*.c" |xargs rm
find lib/libc lib/libm lib/zlib -name "*.c" |xargs rm
find platform/arm/cortex-a7 -name "*.c" |xargs rm
find platform/arm/cortex-a7 -name "*.s" |xargs rm
find fs/fat fs/jffs2 fs/nfs fs/ramfs fs/vfs fs/yaffs2   -name "*.c" |xargs rm
find kernel/net/lwip vendor/ar6k3_wifi drivers/usb -name "*.c" |xargs rm
find kernel/net/lwip vendor/ar6k3_wifi drivers/usb -name "*.cpp" |xargs rm

find platform/arm/arm926 -name "*.c" |xargs rm
find platform/arm/arm926 -name "*.s" |xargs rm
rm -rf kernel/extended/scatter/*.c
rm -rf platform/arm/cortex-m3

echo "##########step3:delete over!!! enable libmk###########"
##enable the libmakefile
mv fs/vfs/blib/libmk fs/vfs/blib/Makefile
mv shell/libmk shell/Makefile
mv compat/linux/libmk compat/linux/Makefile
mv compat/posix/libmk compat/posix/Makefile
mv lib/libc/libmk  lib/libc/Makefile
mv lib/libm/libmk  lib/libm/Makefile
mv lib/zlib/libmk  lib/zlib/Makefile
mv fs/fat/libmk fs/fat/Makefile
mv fs/jffs2/libmk fs/jffs2/Makefile
mv fs/nfs/libmk fs/nfs/Makefile
mv fs/ramfs/libmk fs/ramfs/Makefile
mv fs/vfs/libmk fs/vfs/Makefile
mv fs/yaffs2/libmk fs/yaffs2/Makefile
mv kernel/extended/scatter/libmk kernel/extended/scatter/Makefile
mv kernel/extended/cppsupport/libmk kernel/extended/cppsupport/Makefile
mv platform/arm/cortex-a7/libmk platform/arm/cortex-a7/Makefile
mv platform/arm/arm926/libmk platform/arm/arm926/Makefile
mv kernel/m2mcomm/libmk kernel/m2mcomm/Makefile
mv kernel/extended/dynload/libmk kernel/extended/dynload/Makefile
mv drivers/usb/libmk drivers/usb/Makefile
mv kernel/net/lwip/libmk kernel/net/lwip/Makefile
mv vendor/ar6k3_wifi/libmk vendor/ar6k3_wifi/Makefile
mv kernel/extended/runstop/libmk kernel/extended/runstop/Makefile
#mv libmk Makefile
echo "##########step4:enable libmk over!!! modify makefile and ld###########"
echo "##########step5:make lib version over###########"

cd -
