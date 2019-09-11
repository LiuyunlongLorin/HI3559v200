#!/bin/bash
#build the source to get lib
#set -x
MAKE()
{
    make -j
    if [ -e $PWD/out/$cur_platform/vs_server.bin ]
    then
        echo "vs_server.bin is exist"
    else
        make -j
        if [ -e $PWD/out/$cur_platform/vs_server.bin ]
        then
            echo "vs_server.bin is exist"
        else
            echo "make error"
            exit 1
        fi
    fi
}
MAKEWOW()
{
    make wow
    make -j
    if [ -e $PWD/out/$cur_platform/lib/obj/wow.O ]
    then
        echo "wow.O is exist"
        if [ -e $PWD/out/$cur_platform/vs_server.bin ]
        then
            echo "vs_server.bin is exist"
        else
            make -j
            if [ -e $PWD/out/$cur_platform/vs_server.bin ]
            then
                echo "vs_server.bin is exist"
            else
                echo "vs_server.bin is not exist"
                exit 1
            fi
        fi
        cd $PWD/tools
        git checkout scripts/
        cd -
    else
        echo "make wow error"
        exit 1
    fi
}
MAKE_PLATFORM()
{
    if [ $cur_platform == "hi3516a" ]
    then
        MAKEWOW
    fi

    if [ $cur_platform == "hi3516cv300" ]
    then
        MAKEWOW
    fi

    if [ $cur_platform == "hi3518ev200" ]
    then
        MAKEWOW
    fi

    if [ $cur_platform == "hi3559/cortex-a7" ]
    then
        MAKEWOW
    fi

    if [ $cur_platform == "hi3559/cortex-a17" ]
    then
        MAKEWOW
    fi

    if [ $cur_platform == "hi3911" ]
    then
        MAKE
    fi

    if [ $cur_platform == "hi3731" ]
    then
        MAKE
    fi
}

LITEOSTOPDIR=`pwd`
cd ${LITEOSTOPDIR}

src_array=(
kernel/extended/scatter
kernel/extended/dynload
kernel/extended/runstop
kernel/extended/cppsupport
kernel/extended/cpup
lib/libc/src/sysdeps/a7
lib/libc/src/sysdeps/common
net/wpa_supplicant
fs/vfs/bcache
fs/proc
fs/nfs
fs/ramfs
fs/yaffs2
fs/jffs2
shell
drivers/wifi/wifi_adapt
drivers/wifi/bcm_wifi
drivers/wifi/ar6k3_wifi
drivers/usb
platform/cpu/arm/arm-a/cortex-a7
platform/cpu/arm/arm-a/cortex-a17
platform/cpu/arm/arm-a/arm926
)

lib_array=(
libscatter.a
libdynload.a
librunstop.a
libcppsupport.a
libcpup.a
libcsysdeps.a
libcsysdeps.a
libwpa.a
libbcache.a
libproc.a
libnfs.a
libramfs.a
libyaffs2.a
libjffs2.a
libshell.a
libwifi_adapt.a
libwwd.a
libar6003.a
libusb_base.a
libcortex-a7.a
libcortex-a17.a
libarm926.a
)


if [ $# == 1 ]
then
support_platform=(
$1
)
else
support_platform=(
hi3559/cortex-a7
hi3516a
hi3516cv300
hi3518ev200
hi3559/cortex-a17
hi3731
)
fi

support_cpu=(
arm926
cortex-a7
cortex-a17
)
######## if no input parameter exit shell script #########
if [ $# != 0 ] && [ $# != 1 ]
then
    echo "######## please do not input parameters or only input one parameter such as hi3516a ########"
    exit
fi

#. env_setup.sh hi3516a
echo "##########step1:build over!!! start to copy libs###########"
for cur_platform in ${support_platform[@]}
do
    echo "platform is : ${cur_platform}"

    chmod 666 ./.config
    chmod 777 ./tools/build/config/debug -R

    if [ ${cur_platform} == "hi3516a" ] || [ ${cur_platform} == "hi3559/cortex-a7" ]
    then
        cur_cpu="cortex-a7"
    elif [ ${cur_platform} == "hi3518ev200" ] || [ ${cur_platform} == "hi3731" ] || [ ${cur_platform} == "hi3911" ] || [ ${cur_platform} == "hi3516cv300" ]
    then
        cur_cpu="arm926"
    elif [ ${cur_platform} == "hi3559/cortex-a17" ]
    then
        cur_cpu="cortex-a17"
    fi

    if [ ${cur_platform} == "hi3559/cortex-a7" ]
    then
        config_file="hi3559_cortex-a7.config"
    elif [ ${cur_platform} == "hi3559/cortex-a17" ]
    then
        config_file="hi3559_cortex-a17.config"
    else
        config_file="${cur_platform}.config"
    fi
    cp $LITEOSTOPDIR/tools/build/config/debug/$config_file ./.config

    make clean
    MAKE_PLATFORM
    sed -i "s/LOSCFG_DRIVERS_WIFI_QRD=y/LOSCFG_DRIVERS_WIFI_BCM=y/g" .config
    MAKE_PLATFORM
    ########### creat dir for platforms ##########
    for dir in ${src_array[@]}
    do
        if [ ${dir} == "platform/cpu/arm/arm-a/cortex-a7" ]
        then
            if [ ${cur_platform} != "hi3516a" ] && [ ${cur_platform} != "hi3559/cortex-a7" ]
            then
                continue
            fi
        fi
        if [ ${dir} == "platform/cpu/arm/arm-a/arm926" ]
        then
            if [ ${cur_platform} != "hi3518ev200" ] && [ ${cur_platform} != "hi3731" ] && [ ${cur_platform} != "hi3911" ] && [ ${cur_platform} != "hi3516cv300" ]
            then
                continue
            fi
        fi
        if [ ${dir} == "platform/cpu/arm/arm-a/cortex-a17" ]
        then
            if [ ${cur_platform} != "hi3559/cortex-a17" ]
            then
                continue
            fi
        fi
        if [ ${dir} == "lib/libc/src/sysdeps/a7" ]
        then
            if [ ${cur_platform} != "hi3516a" ] && [ ${cur_platform} != "hi3559/cortex-a7" ] && [ ${cur_platform} != "hi3559/cortex-a17" ]
            then
                continue
            fi
        fi
        if [ ${dir} == "lib/libc/src/sysdeps/common" ]
        then
            if [ ${cur_platform} != "hi3518ev200" ] && [ ${cur_platform} != "hi3731" ] && [ ${cur_platform} != "hi3911" ] && [ ${cur_platform} != "hi3516cv300" ]
            then
                continue
            fi
        fi
        mkdir -p ${dir}/$cur_cpu
    done

    ####### copy the lib to the direct ########
    for index in `seq 0 21`
    do
    if [ ${lib_array[index]} = "libcsysdeps.a" ]
    then
        if [ ${src_array[index]} == "lib/libc/src/sysdeps/a7" ]
        then
            if [ ${cur_platform} != "hi3516a" ] && [ ${cur_platform} != "hi3559/cortex-a7" ] && [ ${cur_platform} != "hi3559/cortex-a17" ]
            then
                continue
            fi

            if [ -e ${src_array[index]}/cortex-a7 ] && [ -e ${src_array[index]}/cortex-a17 ]
            then
                mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile
            fi
        fi
        if [ ${src_array[index]} = "lib/libc/src/sysdeps/common" ]
        then
            if [ ${cur_platform} != "hi3518ev200" ] && [ ${cur_platform} != "hi3731" ] && [ ${cur_platform} != "hi3911" ] && [ ${cur_platform} != "hi3516cv300" ]
            then
                continue
            fi
            if [ -e ${src_array[index]}/arm926 ]
            then
                mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile
            fi
        fi
    fi
    if [ ${lib_array[index]} = "libcortex-a7.a" ]
    then
        if [ ${cur_platform} != "hi3516a" ] && [ ${cur_platform} != "hi3559/cortex-a7" ]
        then
            continue
        else
            mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile
        fi
    fi
    if [ ${lib_array[index]} = "libcortex-a17.a" ]
    then
        if [ ${cur_platform} != "hi3559/cortex-a17" ]
        then
            continue
        else
            mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile
        fi
    fi
    if [ ${lib_array[index]} == "libarm926.a" ]
    then
        if [ ${cur_platform} != "hi3518ev200" ] && [ ${cur_platform} != "hi3731" ] && [ ${cur_platform} != "hi3911" ] && [ ${cur_platform} != "hi3516cv300" ]
        then
            continue
        else
            mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile
        fi
    fi
    if [ $# == 1 ] && [ -e out/${cur_platform}/lib/${lib_array[index]} ]
    then
        cp  out/${cur_platform}/lib/${lib_array[index]} ${src_array[index]}/$cur_cpu/
        mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile

    elif [ -e out/${cur_platform}/lib/${lib_array[index]} ]
    then
        cp out/${cur_platform}/lib/${lib_array[index]} ${src_array[index]}/$cur_cpu/
        if [ -e ${src_array[index]}/libmk ] && [ -e ${src_array[index]}/${support_cpu[0]} ] && [ -e ${src_array[index]}/${support_cpu[1]} ] && [ -e ${src_array[index]}/${support_cpu[2]} ]
        then
            mv -f ${src_array[index]}/libmk ${src_array[index]}/Makefile
        fi
    fi

    done
    make clean
done

echo "##########step2:copy over!!! delete the source###########"
for dir in ${src_array[@]}
do
    ##usb文件单独删除
    if [ ${dir} == "drivers/usb" ]
    then
        continue
    fi
    find ${dir} -name "*.[csS]" | xargs rm
done
##delete some file of usb##
while read LINE;do
    file=`echo $LINE | awk -F " " '{print $1}'`
    rm -rf drivers/usb/$file
done < tools/scripts/usb_del_list.txt
rm -rf platform/cpu/arm/arm-m/cortex-m3
rm -rf test/
rm -rf vendor/
vendor="vendor\/Kconfig"
Test="test\/Kconfig"
sed -i '/'"$vendor"'/d' tools/menuconfig/extra/Configs/Config.in
sed -i '/'"$Test"'/d' tools/menuconfig/extra/Configs/Config.in

cd $LITEOSTOPDIR
find  ./ -name ".git" |xargs rm -rf
#find  ./ -name "Kconfig" | xargs rm -rf
#rm mklibversion.sh
cd -
