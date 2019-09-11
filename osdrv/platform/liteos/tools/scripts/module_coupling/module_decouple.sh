#!/bin/bash
set -x

LOS_DRI="LOSCFG_DRIVERS"

MAKE()
{
    make -j
    if [ -e $PWD/out/$cur_platform/vs_server.bin ]
    then
        echo "vs_server.bin is exist"
        make clean
    else
        make -j
        if [ -e $PWD/out/$cur_platform/vs_server.bin ]
        then
            echo "vs_server.bin is exist"
            make clean
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
                make clean
            else
                echo "vs_server.bin is not exist"
                exit 1
            fi
        fi
        cd $PWD/tools
        git checkout scripts/
        cd -
        make clean
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

    if [ $cur_platform == "hi3518ev200" ]
    then
        MAKEWOW
    fi

    if [ $cur_platform == "hi3519/cortex-a7" ]
    then
        MAKE
    fi

    if [ $cur_platform == "hi3519/cortex-a17" ]
    then
        MAKE
    fi

    if [ $cur_platform == "hi3911" ]
    then
        MAKE
    fi

    if [ $cur_platform == "him5v100" ]
    then
        MAKE
    fi

    if [ $cur_platform == "hi3731" ]
    then
        MAKE
    fi
}

# this is for 16a and 18ev200, open testsuit and then open debug
testsuit_decouple()
{
    sed -i "s/LOSCFG_PLATFORM_OSAPPINIT=y/LOSCFG_TEST=y/g" .config
    MAKE
}
#this is for all, open osappinit and then open debug
osappinit_decouple()
{
    # restore .config
    echo "LOSCFG_COMPILE_DEBUG=y" >> .config
    MAKE_PLATFORM
    sed -i "s/LOSCFG_COMPILE_DEBUG=y/LOSCFG_COMPILE_DEBUG=n/g" .config
}

#this is for all
debug_decouple()
{
    ## open some debug modules
    if [ $cur_platform != "hi3911" ] && [ $cur_platform != "him5v100" ]
    then
        echo "LOSCFG_DEBUG_VERSION=y" >> .config
        echo "LOSCFG_NET_LWIP_SACK_TFTP=y" >> .config
        echo "LOSCFG_TOOLS_IPERF=y" >> .config
        echo "LOSCFG_NET_TELNET=y" >> .config
        echo "LOSCFG_SHELL=y" >> .config
        echo "LOSCFG_PLATFORM_DVFS=y" >> .config
        echo "LOSCFG_VENDOR=y" >> .config
        echo "LOSCFG_FS_PROC=y" >> .config
        echo "LOSCFG_FS_RAMFS=y" >> .config
        echo "LOSCFG_FS_NFS=y" >> .config
        echo "LOSCFG_NET_WPA=y" >> .config
        sed -i "s/"$LOS_DRI"_WIFI_BCM=y/"$LOS_DRI"_WIFI_QRD=y/g" .config
        MAKE_PLATFORM
        ## close the debug modules  ##
        sed -i "s/LOSCFG_VENDOR=y/LOSCFG_VENDOR=n/g" .config
        sed -i "s/LOSCFG_PLATFORM_DVFS=y/LOSCFG_PLATFORM_DVFS=n/g" .config
        sed -i "s/LOSCFG_SHELL=y/LOSCFG_SHELL=n/g" .config
        sed -i "s/LOSCFG_NET_TELNET=y/LOSCFG_NET_TELNET=n/g" .config
        sed -i "s/LOSCFG_TOOLS_IPERF=y/LOSCFG_TOOLS_IPERF=n/g" .config
        sed -i "s/LOSCFG_NET_LWIP_SACK_TFTP=y/LOSCFG_NET_LWIP_SACK_TFTP=n/g" .config
    else
        echo "LOSCFG_DEBUG_VERSION=y" >> .config
        echo "LOSCFG_FS_PROC=y" >> .config
        echo "LOSCFG_FS_RAMFS=y" >> .config
        MAKE_PLATFORM
    fi
}

fs_decouple()
{
    if [ $cur_platform == "hi3731" ]
    then
        echo "do not need doing anything"
    else
        sed -i "s/LOSCFG_FS_JFFS=y/LOSCFG_FS_JFFS=n/g" .config
    fi
    sed -i "s/LOSCFG_FS_PROC=y/LOSCFG_FS_PROC=n/g" .config
    sed -i "s/LOSCFG_FS_RAMFS=y/LOSCFG_FS_RAMFS=n/g" .config
    sed -i "s/LOSCFG_FS_NFS=y/LOSCFG_FS_NFS=n/g" .config
    sed -i "s/LOSCFG_FS_FAT_CHINESE=y/LOSCFG_FS_CHINESE=n/g" .config
    sed -i "s/LOSCFG_FS_FAT=y/LOSCFG_FS_FAT=n/g" .config
    MAKE_PLATFORM
#   sed -i "s/LOSCFG_FS_YAFFS=y/LOSCFG_FS_YAFFS=n/g" .config
#   MAKE_PLATFORM
#   sed -i "s/LOSCFG_FS_CACHE=y/LOSCFG_FS_CACHE=n/g" .config
#   MAKE_PLATFORM
}

net_decouple()
{
    sed -i "s/LOSCFG_NET_LWIP_SACK=y/LOSCFG_NET_LWIP_SACK=n/g" .config
    sed -i "s/LOSCFG_NET_WPA=y/LOSCFG_NET_WPA=n/g" .config
    ## these drivers depend on lwip
    sed -i "s/"$LOS_DRI"_HIGMAC=y/"$LOS_DRI"_HIGMAC=n/g" .config
    sed -i "s/"$LOS_DRI"_HIETH_SF=y/"$LOS_DRI"_HIETH_SF=n/g" .config
    sed -i "s/"$LOS_DRI"_USB=y/"$LOS_DRI"_USB=n/g" .config
    sed -i "s/"$LOS_DRI"_WIFI=y/"$LOS_DRI"_WIFI=n/g" .config
    sed -i "s/"$LOS_DRI"_WIFI_QRD=y/"$LOS_DRI"_WIFI_QRD=n/g" .config
    MAKE_PLATFORM
}
driver_decouple()
{
    ## close driver ##
    sed -i "s/"$LOS_DRI"_CELLWISE=y/"$LOS_DRI"_CELLWISE=n/g" .config
    sed -i "s/"$LOS_DRI"_GPIO=y/"$LOS_DRI"_GPIO=n/g" .config
    sed -i "s/"$LOS_DRI"_NETDEV=y/"$LOS_DRI"_NETDEV=n/g" .config
#   sed -i "s/"$LOS_DRI"_HIGMAC=y/"$LOS_DRI"_HIGMAC=n/g" .config
#   sed -i "s/"$LOS_DRI"_HIETH_SF=y/"$LOS_DRI"_HIETH_SF=n/g" .config
    sed -i "s/"$LOS_DRI"_I2C=y/"$LOS_DRI"_I2C=n/g" .config
    sed -i "s/"$LOS_DRI"_LCD=y/"$LOS_DRI"_LCD=n/g" .config
    sed -i "s/"$LOS_DRI"_MEM=y/"$LOS_DRI"_MEM=n/g" .config
    sed -i "s/"$LOS_DRI"_MMC=y/"$LOS_DRI"_MMC=n/g" .config
    sed -i "s/"$LOS_DRI"_RANDOM=y/"$LOS_DRI"_RANDOM=n/g" .config
    sed -i "s/"$LOS_DRI"_RTC=y/"$LOS_DRI"_RTC=n/g" .config
    sed -i "s/"$LOS_DRI"_SPI=y/"$LOS_DRI"_SPI=n/g" .config
#   sed -i "s/"$LOS_DRI"_USB=y/"$LOS_DRI"_USB=n/g" .config
    sed -i "s/"$LOS_DRI"_VIDEO=y/"$LOS_DRI"_VIDEO=n/g" .config
#   sed -i "s/"$LOS_DRI"_WIFI=y/"$LOS_DRI"_WIFI=n/g" .config
#   sed -i "s/"$LOS_DRI"_WIFI_QRD=y/"$LOS_DRI"_WIFI_QRD=n/g" .config
    MAKE_PLATFORM
}
common_decouple()
{
    osappinit_decouple
    debug_decouple
    fs_decouple
    net_decouple
    driver_decouple
}
support_platform=(
hi3516a
hi3518ev200
hi3519/cortex-a7
hi3519/cortex-a17
hi3911
him5v100
hi3731
)
config_file=(
hi3516a
hi3518ev200
hi3519_cortex-a7
hi3519_cortex-a17
hi3911
him5v100
hi3731
)
if [ $# != 0 ]
then
     echo "######### please do not input parameter #######"
     exit
fi

for cur_platform in ${support_platform[@]}
do
    echo "********** the current platform is $cur_platform **********"

    if [ $cur_platform == "hi3516a" ]
    then
        cp -rf ./tools/build/config/${config_file[0]}.config .config
        common_decouple
    fi

    if [ $cur_platform == "hi3518ev200" ]
    then
        cp -rf ./tools/build/config/${config_file[1]}.config .config
        common_decouple
    fi

    if [ $cur_platform == "hi3519/cortex-a7" ]
    then
        cp -rf ./tools/build/config/${config_file[2]}.config .config
        common_decouple
    fi

    if [ $cur_platform == "hi3519/cortex-a17" ]
    then
        cp -rf ./tools/build/config/${config_file[3]}.config .config
        common_decouple
    fi

    if [ $cur_platform == "hi3911" ]
    then
        cp -rf ./tools/build/config/${config_file[4]}.config .config
        common_decouple
    fi

    if [ $cur_platform == "him5v100" ]
    then
        cp -rf ./tools/build/config/${config_file[5]}.config .config
        common_decouple
    fi

    if [ $cur_platform == "hi3731" ]
    then
        cp -rf ./tools/build/config/${config_file[6]}.config .config
        common_decouple
    fi
done
